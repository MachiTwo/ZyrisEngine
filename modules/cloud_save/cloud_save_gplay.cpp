/**************************************************************************/
/*  cloud_save_gplay.cpp                                                  */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

// ─────────────────────────────────────────────────────────────────────────────
// Google Play Games Services — Snapshots (Saved Games) Integration
//
// This file documents the full integration pattern for the GPGS C++ SDK.
// The SDK provides gpg::SnapshotManager for reading and writing saved games.
//
// Key objects:
//   gpg::GameServices*           — Root entry point, created once.
//   gpg::SnapshotManager         — Provides Open, Commit, Read, Delete.
//   gpg::SnapshotMetadata        — Lightweight metadata (no blob).
//   gpg::SnapshotMetadataChange  — Builder for updating metadata on commit.
//   gpg::SnapshotData            — Holds the raw byte blob.
//
// Conflict handling:
//   When Open returns a conflict, we compare modification times.
//   If remote is newer, we resolve by keeping the remote blob.
//   The conflict_callback (if set by the developer) may override this.
//
// Compile guard: GOOGLE_PLAY_GAMES_ENABLED
// ─────────────────────────────────────────────────────────────────────────────

#include "cloud_save_gplay.h"

#include "core/error/error_macros.h"
#include "core/io/marshalls.h"

#ifdef GOOGLE_PLAY_GAMES_ENABLED
// #include <gpg/gpg.h>
// The GPGS SDK types below are forward-declared for documentation purposes.
// A real integration must include the actual SDK header above.
#endif

// ── Helpers ───────────────────────────────────────────────────────────────────

String CloudSaveGPlay::_make_snapshot_name(const String &p_slot_name) {
	// Snapshot names in GPGS must match: ^[a-zA-Z0-9-_.~]+$  (max 100 chars)
	String name = p_slot_name.replace("/", "_").replace("\\", "_");
	if (name.length() > 100) {
		name = name.substr(0, 100);
	}
	return name;
}

PackedByteArray CloudSaveGPlay::_serialize_snapshot(const Ref<Snapshot> &p_snapshot) const {
	ERR_FAIL_COND_V(p_snapshot.is_null(), PackedByteArray());
	Variant v = p_snapshot;
	int len = 0;
	Error err = encode_variant(v, nullptr, len, false);
	ERR_FAIL_COND_V(err != OK, PackedByteArray());
	PackedByteArray bytes;
	bytes.resize(len);
	encode_variant(v, bytes.ptrw(), len, false);
	return bytes;
}

Ref<Snapshot> CloudSaveGPlay::_deserialize_snapshot(const PackedByteArray &p_bytes) const {
	ERR_FAIL_COND_V(p_bytes.is_empty(), Ref<Snapshot>());
	Variant v;
	int bytes_read = 0;
	Error err = decode_variant(v, p_bytes.ptr(), p_bytes.size(), &bytes_read, false);
	ERR_FAIL_COND_V(err != OK, Ref<Snapshot>());
	return Ref<Snapshot>(v);
}

// ── Lifecycle ─────────────────────────────────────────────────────────────────

CloudSaveProvider::CloudResult CloudSaveGPlay::initialize() {
#ifdef GOOGLE_PLAY_GAMES_ENABLED
	if (_client_id.is_empty()) {
		ERR_PRINT("GooglePlayCloudProvider: client_id is empty. Set it in Project Settings > Application > Persistence > Cloud Save > Google Play.");
		return CLOUD_ERR_INVALID_CONFIG;
	}

	// GPGS initialization pattern (gpg-cpp-sdk v2):
	//
	//   gpg::AndroidInitialization::JNI_OnLoad(jvm);  // from JNI_OnLoad
	//
	//   auto platform_config = gpg::AndroidPlatformConfiguration()
	//       .SetActivity(android_activity);
	//
	//   auto config = gpg::GameServices::Builder()
	//       .SetOnAuthActionStarted([](gpg::AuthOperation op) { ... })
	//       .SetOnAuthActionFinished([](gpg::AuthOperation op, gpg::AuthStatus status) {
	//           if (status == gpg::AuthStatus::VALID) {
	//               // Authentication succeeded — mark _initialized = true
	//           }
	//       })
	//       .Create(platform_config);
	//
	//   // Store config in a member variable.
	//   // The auth callbacks fire asynchronously; initialize() may need to wait.
	//
	// For simplicity, this stub returns CLOUD_ERR_NOT_INITIALIZED until the
	// SDK is linked and the pattern above is implemented.

	WARN_PRINT("GooglePlayCloudProvider: GPGS SDK not linked. Google Play Cloud Save is inactive.");
	return CLOUD_ERR_NOT_INITIALIZED;
#else
	WARN_PRINT("GooglePlayCloudProvider: Engine was not compiled with GOOGLE_PLAY_GAMES_ENABLED.");
	return CLOUD_ERR_NOT_INITIALIZED;
#endif
}

void CloudSaveGPlay::shutdown() {
	_initialized = false;
}

bool CloudSaveGPlay::is_ready() const {
	return _initialized;
}

// ── Core Operations ───────────────────────────────────────────────────────────

CloudSaveProvider::CloudResult CloudSaveGPlay::upload(const String &p_slot_name, const Ref<Snapshot> &p_snapshot) {
#ifdef GOOGLE_PLAY_GAMES_ENABLED
	if (!_initialized) {
		return CLOUD_ERR_NOT_INITIALIZED;
	}

	PackedByteArray bytes = _serialize_snapshot(p_snapshot);
	ERR_FAIL_COND_V(bytes.is_empty(), CLOUD_ERR_UNKNOWN);

	String snap_name = _make_snapshot_name(p_slot_name);

	// GPGS Commit pattern:
	//
	//   game_services->Snapshots().Open(
	//       snap_name.utf8().get_data(),
	//       gpg::SnapshotConflictPolicy::LONGEST_PLAYTIME,
	//       [this, bytes, snap_name](gpg::SnapshotManager::OpenResponse const &response) {
	//           if (response.status == gpg::SnapshotManager::OpenStatus::VALID) {
	//               gpg::SnapshotData data = response.data.Read();
	//               data.SetData(bytes.ptr(), bytes.size());
	//
	//               auto metadata_change = gpg::SnapshotMetadataChange::Builder()
	//                   .SetDescription("Save slot: " + snap_name)
	//                   .Create();
	//
	//               response.data.Open(snap_name, gpg::SnapshotConflictPolicy::MOST_RECENTLY_MODIFIED);
	//               game_services->Snapshots().Commit(response.data, metadata_change, data,
	//                   [](gpg::SnapshotManager::CommitResponse const &commit_resp) {
	//                       if (commit_resp.status != gpg::SnapshotManager::CommitStatus::VALID) {
	//                           WARN_PRINT("GPGS Commit failed.");
	//                       }
	//                   });
	//           }
	//       });
	//
	// Note: GPGS callbacks are asynchronous by nature.  In a blocking worker
	// thread context, use Semaphore to wait for completion.

	(void)snap_name;
	(void)bytes;
	return CLOUD_ERR_NOT_INITIALIZED; // Replace with actual call.
#else
	(void)p_slot_name;
	(void)p_snapshot;
	return CLOUD_ERR_NOT_INITIALIZED;
#endif
}

CloudSaveProvider::CloudResult CloudSaveGPlay::download(const String &p_slot_name, Ref<Snapshot> &r_snapshot) {
#ifdef GOOGLE_PLAY_GAMES_ENABLED
	if (!_initialized) {
		return CLOUD_ERR_NOT_INITIALIZED;
	}

	String snap_name = _make_snapshot_name(p_slot_name);

	// GPGS Read pattern:
	//
	//   game_services->Snapshots().Open(
	//       snap_name.utf8().get_data(),
	//       gpg::SnapshotConflictPolicy::MOST_RECENTLY_MODIFIED,
	//       [this, &r_snapshot](gpg::SnapshotManager::OpenResponse const &response) {
	//           if (response.status == gpg::SnapshotManager::OpenStatus::VALID) {
	//               gpg::SnapshotData data = response.data.Read();
	//               PackedByteArray bytes;
	//               bytes.resize(data.size());
	//               memcpy(bytes.ptrw(), data.data(), data.size());
	//               r_snapshot = _deserialize_snapshot(bytes);
	//           } else if (response.status == gpg::SnapshotManager::OpenStatus::CONFLICT) {
	//               // Invoke conflict resolver — keep server version by default.
	//           }
	//           semaphore.signal();
	//       });
	//   semaphore.wait();

	(void)snap_name;
	(void)r_snapshot;
	return CLOUD_ERR_NOT_INITIALIZED;
#else
	(void)p_slot_name;
	r_snapshot = Ref<Snapshot>();
	return CLOUD_ERR_NOT_INITIALIZED;
#endif
}

CloudSaveProvider::CloudResult CloudSaveGPlay::query_slot(const String &p_slot_name, RemoteSlotInfo &r_info) {
#ifdef GOOGLE_PLAY_GAMES_ENABLED
	if (!_initialized) {
		return CLOUD_ERR_NOT_INITIALIZED;
	}

	// Use Snapshots().FetchAll() or .ShowSelectUIOperation() then filter by name.
	// game_services->Snapshots().FetchAll([&](gpg::SnapshotManager::FetchAllResponse const &r) {
	//     for (auto const &md : r.data) {
	//         if (String(md.FileName().c_str()) == snap_name) {
	//             r_info.timestamp = (uint64_t)md.LastModifiedTime().count() / 1000;
	//             r_info.size_bytes = md.PlayedTime().count(); // placeholder
	//         }
	//     }
	// });

	(void)p_slot_name;
	(void)r_info;
	return CLOUD_ERR_NOT_INITIALIZED;
#else
	(void)p_slot_name;
	(void)r_info;
	return CLOUD_ERR_NOT_INITIALIZED;
#endif
}

CloudSaveProvider::CloudResult CloudSaveGPlay::delete_slot(const String &p_slot_name) {
#ifdef GOOGLE_PLAY_GAMES_ENABLED
	if (!_initialized) {
		return CLOUD_ERR_NOT_INITIALIZED;
	}

	// game_services->Snapshots().Delete(snapshot_metadata);
	(void)p_slot_name;
	return CLOUD_ERR_NOT_INITIALIZED;
#else
	(void)p_slot_name;
	return CLOUD_ERR_NOT_INITIALIZED;
#endif
}

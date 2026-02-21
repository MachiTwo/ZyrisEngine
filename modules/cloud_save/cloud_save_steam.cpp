/**************************************************************************/
/*  cloud_save_steam.cpp                                                  */
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
// Implementation Note:
//
// The Steamworks SDK types (ISteamRemoteStorage, SteamRemoteStorage(),
// EResult, k_EResultOK, etc.) are guarded behind STEAMWORKS_ENABLED to allow
// the engine to compile cleanly on platforms and build configurations where the
// SDK is not linked.  When STEAMWORKS_ENABLED is not defined the provider
// reports CLOUD_ERR_NOT_INITIALIZED from every call so the SaveServer falls
// back gracefully to local disk.
//
// To enable:
//   Add "STEAMWORKS_ENABLED=1" to your platform's build options and ensure the
//   Steamworks SDK headers + libraries are on the include/link path.
// ─────────────────────────────────────────────────────────────────────────────

#include "cloud_save_steam.h"

#include "core/error/error_macros.h"
#include "core/io/marshalls.h"
#include "core/os/os.h"
#include "scene/resources/snapshot.h"

#ifdef STEAMWORKS_ENABLED
// The game project is responsible for including and linking the Steamworks SDK.
// GodotSteam users: the singleton is available via Engine::get_singleton().
// Direct SDK users: include the Steamworks header below.
//   #include <steam/steam_api.h>
//
// For compile-time safety, we forward-declare only what we touch here.
// A full integration MUST include steam_api.h instead:

// Forward declarations (replace with proper #include when integrating)
struct ISteamRemoteStorage;
extern ISteamRemoteStorage *SteamRemoteStorage();

// Minimal EResult subset
enum EResult {
	k_EResultOK = 1,
	k_EResultInsufficientPrivilege = 24,
	k_EResultDiskFull = 20,
	k_EResultBusy = 10,
};
#endif // STEAMWORKS_ENABLED

// ── Helpers ──────────────────────────────────────────────────────────────────

String CloudSaveSteam::_make_remote_filename(const String &p_slot_name) {
	// Replace path separators to produce a flat filename valid across OS.
	// Steam Remote Storage treats filenames as opaque strings, but disallows '/'.
	String name = p_slot_name.replace("/", "_").replace("\\", "_");
	return name + ".zsav";
}

PackedByteArray CloudSaveSteam::_serialize_snapshot(const Ref<Snapshot> &p_snapshot) const {
	ERR_FAIL_COND_V(p_snapshot.is_null(), PackedByteArray());

	// Encode the Snapshot resource into a variant byte stream.
	// encode_variant returns the number of bytes written.
	Variant v = p_snapshot;
	int len = 0;
	Error err = encode_variant(v, nullptr, len, false);
	ERR_FAIL_COND_V(err != OK, PackedByteArray());

	PackedByteArray bytes;
	bytes.resize(len);
	encode_variant(v, bytes.ptrw(), len, false);
	return bytes;
}

Ref<Snapshot> CloudSaveSteam::_deserialize_snapshot(const PackedByteArray &p_bytes) const {
	ERR_FAIL_COND_V(p_bytes.is_empty(), Ref<Snapshot>());

	Variant v;
	int bytes_read = 0;
	Error err = decode_variant(v, p_bytes.ptr(), p_bytes.size(), &bytes_read, false);
	ERR_FAIL_COND_V(err != OK, Ref<Snapshot>());

	return Ref<Snapshot>(v);
}

CloudSaveProvider::CloudResult CloudSaveSteam::_map_steam_error(int p_result) const {
#ifdef STEAMWORKS_ENABLED
	switch ((EResult)p_result) {
		case k_EResultOK:
			return CLOUD_OK;
		case k_EResultDiskFull:
			return CLOUD_ERR_QUOTA_EXCEEDED;
		case k_EResultInsufficientPrivilege:
			return CLOUD_ERR_NOT_AUTHENTICATED;
		case k_EResultBusy:
			return CLOUD_ERR_UNAVAILABLE;
		default:
			return CLOUD_ERR_UNKNOWN;
	}
#else
	(void)p_result;
	return CLOUD_ERR_NOT_INITIALIZED;
#endif
}

// ── Lifecycle ─────────────────────────────────────────────────────────────────

CloudSaveProvider::CloudResult CloudSaveSteam::initialize() {
#ifdef STEAMWORKS_ENABLED
	ISteamRemoteStorage *remote = SteamRemoteStorage();
	if (!remote) {
		ERR_PRINT("SteamCloudProvider: SteamRemoteStorage() returned null. "
				  "Ensure SteamAPI_Init() was called before SaveServer starts.");
		return CLOUD_ERR_NOT_INITIALIZED;
	}

	// Verify the user has Steam Cloud enabled in their Steam client settings.
	if (!remote->IsCloudEnabledForApp()) {
		WARN_PRINT("SteamCloudProvider: Steam Cloud is disabled for this app in the Steam settings.");
		return CLOUD_ERR_NOT_INITIALIZED;
	}

	// Log quota information for debugging.
	int64_t total_bytes = 0;
	int64_t available_bytes = 0;
	remote->GetQuota(&total_bytes, &available_bytes);
	int64_t used_bytes = total_bytes - available_bytes;
	print_verbose(vformat("SteamCloudProvider: Quota — total: %d KB, used: %d KB, available: %d KB",
			total_bytes / 1024, used_bytes / 1024, available_bytes / 1024));

	if (available_bytes <= 0) {
		WARN_PRINT("SteamCloudProvider: Steam Cloud quota exhausted.");
		return CLOUD_ERR_QUOTA_EXCEEDED;
	}

	_initialized = true;
	print_verbose("SteamCloudProvider: Initialized successfully.");
	return CLOUD_OK;
#else
	WARN_PRINT("SteamCloudProvider: Engine was not compiled with STEAMWORKS_ENABLED. "
			   "Steam Cloud save is unavailable.");
	return CLOUD_ERR_NOT_INITIALIZED;
#endif
}

void CloudSaveSteam::shutdown() {
	_initialized = false;
	// Steam SDK cleanup is the game's responsibility (SteamAPI_Shutdown()).
}

bool CloudSaveSteam::is_ready() const {
	return _initialized;
}

// ── Core Operations ───────────────────────────────────────────────────────────

CloudSaveProvider::CloudResult CloudSaveSteam::upload(const String &p_slot_name, const Ref<Snapshot> &p_snapshot) {
#ifdef STEAMWORKS_ENABLED
	if (!_initialized) {
		return CLOUD_ERR_NOT_INITIALIZED;
	}

	ISteamRemoteStorage *remote = SteamRemoteStorage();
	ERR_FAIL_NULL_V(remote, CLOUD_ERR_NOT_INITIALIZED);

	// Serialize the snapshot to a raw byte stream.
	PackedByteArray bytes = _serialize_snapshot(p_snapshot);
	ERR_FAIL_COND_V(bytes.is_empty(), CLOUD_ERR_UNKNOWN);

	// Check available quota before uploading to provide a clear error.
	int64_t total_bytes = 0;
	int64_t available_bytes = 0;
	remote->GetQuota(&total_bytes, &available_bytes);
	if ((int64_t)bytes.size() > available_bytes) {
		WARN_PRINT(vformat("SteamCloudProvider: Upload of '%s' aborted — not enough quota (%d KB needed, %d KB available).",
				p_slot_name, bytes.size() / 1024, available_bytes / 1024));
		return CLOUD_ERR_QUOTA_EXCEEDED;
	}

	String filename = _make_remote_filename(p_slot_name);
	CharString filename_cs = filename.utf8();

	// ISteamRemoteStorage::FileWrite is synchronous and thread-safe.
	bool ok = remote->FileWrite(filename_cs.get_data(), bytes.ptr(), bytes.size());
	if (!ok) {
		WARN_PRINT(vformat("SteamCloudProvider: FileWrite failed for slot '%s'.", p_slot_name));
		return CLOUD_ERR_UNKNOWN;
	}

	print_verbose(vformat("SteamCloudProvider: Uploaded slot '%s' (%d bytes).", p_slot_name, bytes.size()));
	return CLOUD_OK;
#else
	(void)p_slot_name;
	(void)p_snapshot;
	return CLOUD_ERR_NOT_INITIALIZED;
#endif
}

CloudSaveProvider::CloudResult CloudSaveSteam::download(const String &p_slot_name, Ref<Snapshot> &r_snapshot) {
#ifdef STEAMWORKS_ENABLED
	if (!_initialized) {
		return CLOUD_ERR_NOT_INITIALIZED;
	}

	ISteamRemoteStorage *remote = SteamRemoteStorage();
	ERR_FAIL_NULL_V(remote, CLOUD_ERR_NOT_INITIALIZED);

	String filename = _make_remote_filename(p_slot_name);
	CharString filename_cs = filename.utf8();

	// Check if the file exists remotely before attempting to read.
	if (!remote->FileExists(filename_cs.get_data())) {
		return CLOUD_ERR_NOT_FOUND;
	}

	int32_t file_size = remote->GetFileSize(filename_cs.get_data());
	if (file_size <= 0) {
		WARN_PRINT(vformat("SteamCloudProvider: Remote file '%s' exists but has 0 bytes.", p_slot_name));
		return CLOUD_ERR_NOT_FOUND;
	}

	PackedByteArray bytes;
	bytes.resize(file_size);

	// ISteamRemoteStorage::FileRead is synchronous and thread-safe.
	int32_t read = remote->FileRead(filename_cs.get_data(), bytes.ptrw(), file_size);
	if (read != file_size) {
		WARN_PRINT(vformat("SteamCloudProvider: FileRead for '%s' returned %d bytes, expected %d.", p_slot_name, read, file_size));
		return CLOUD_ERR_NETWORK;
	}

	r_snapshot = _deserialize_snapshot(bytes);
	if (r_snapshot.is_null()) {
		WARN_PRINT(vformat("SteamCloudProvider: Failed to deserialize snapshot for slot '%s'.", p_slot_name));
		return CLOUD_ERR_UNKNOWN;
	}

	print_verbose(vformat("SteamCloudProvider: Downloaded slot '%s' (%d bytes).", p_slot_name, file_size));
	return CLOUD_OK;
#else
	(void)p_slot_name;
	r_snapshot = Ref<Snapshot>();
	return CLOUD_ERR_NOT_INITIALIZED;
#endif
}

CloudSaveProvider::CloudResult CloudSaveSteam::query_slot(const String &p_slot_name, RemoteSlotInfo &r_info) {
#ifdef STEAMWORKS_ENABLED
	if (!_initialized) {
		return CLOUD_ERR_NOT_INITIALIZED;
	}

	ISteamRemoteStorage *remote = SteamRemoteStorage();
	ERR_FAIL_NULL_V(remote, CLOUD_ERR_NOT_INITIALIZED);

	String filename = _make_remote_filename(p_slot_name);
	CharString filename_cs = filename.utf8();

	if (!remote->FileExists(filename_cs.get_data())) {
		return CLOUD_ERR_NOT_FOUND;
	}

	r_info.slot_name = p_slot_name;
	r_info.size_bytes = (uint64_t)remote->GetFileSize(filename_cs.get_data());
	r_info.timestamp = (uint64_t)remote->GetFileTimestamp(filename_cs.get_data());
	// Steam does not expose a checksum; leave empty — SaveServer compares timestamps.
	r_info.checksum = "";

	return CLOUD_OK;
#else
	(void)p_slot_name;
	(void)r_info;
	return CLOUD_ERR_NOT_INITIALIZED;
#endif
}

CloudSaveProvider::CloudResult CloudSaveSteam::delete_slot(const String &p_slot_name) {
#ifdef STEAMWORKS_ENABLED
	if (!_initialized) {
		return CLOUD_ERR_NOT_INITIALIZED;
	}

	ISteamRemoteStorage *remote = SteamRemoteStorage();
	ERR_FAIL_NULL_V(remote, CLOUD_ERR_NOT_INITIALIZED);

	String filename = _make_remote_filename(p_slot_name);
	CharString filename_cs = filename.utf8();

	if (remote->FileExists(filename_cs.get_data())) {
		remote->FileDelete(filename_cs.get_data());
	}
	return CLOUD_OK;
#else
	(void)p_slot_name;
	return CLOUD_ERR_NOT_INITIALIZED;
#endif
}

/**************************************************************************/
/*  cloud_save_psn.cpp                                                    */
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
// PSN Cloud Save — Pass-Through Implementation
//
// PlayStation does not expose an explicit "upload to PSN" C++ API to
// developers under NDA.  Instead the title writes save data to a mount point
// provided by the Save Data Management API and the system transparently
// syncs with PS Plus cloud storage.
//
// What we implement here:
//   - initialize(): Mount the save data directory using the PS SDK.
//   - upload():     Write the snapshot to the mount point.
//                   OS handles the actual cloud upload.
//   - download():   Return CLOUD_ERR_NOT_FOUND so SaveServer reads from local
//                   disk (already synced by the OS on boot/resume).
//   - query_slot(): Read file stats from the mount point.
//   - delete_slot():Remove the file from the mount point.
//
// IMPORTANT: Replace every marked TODO block with the actual licensed SDK
// function calls.  The function names and types are intentionally omitted
// from this open-source repository.
// ─────────────────────────────────────────────────────────────────────────────

#include "cloud_save_psn.h"

#include "core/error/error_macros.h"
#include "core/io/file_access.h"
#include "core/io/marshalls.h"

// ── Helpers ───────────────────────────────────────────────────────────────────

String CloudSavePSN::_make_file_path(const String &p_slot_name) const {
	String name = p_slot_name.replace("/", "_");
	return _save_mount_path.path_join(name + ".zsav");
}

PackedByteArray CloudSavePSN::_serialize_snapshot(const Ref<Snapshot> &p_snapshot) const {
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

Ref<Snapshot> CloudSavePSN::_deserialize_snapshot(const PackedByteArray &p_bytes) const {
	ERR_FAIL_COND_V(p_bytes.is_empty(), Ref<Snapshot>());
	Variant v;
	int bytes_read = 0;
	Error err = decode_variant(v, p_bytes.ptr(), p_bytes.size(), &bytes_read, false);
	ERR_FAIL_COND_V(err != OK, Ref<Snapshot>());
	return Ref<Snapshot>(v);
}

// ── Lifecycle ─────────────────────────────────────────────────────────────────

CloudSaveProvider::CloudResult CloudSavePSN::initialize() {
#if defined(PLATFORM_PS4) || defined(PLATFORM_PS5)
	// TODO (NDA): Call sceSaveDataMount2() or equivalent PS5 API to mount
	// the save data directory.  Store the returned mount point path in
	// _save_mount_path and set _initialized = true on success.
	//
	// Canonical pattern (redacted):
	//   SceSaveDataMountPoint mount = {};
	//   SceSaveDataMount2 params = {};
	//   params.dirName.data[...] = ...;
	//   params.mountMode = SCE_SAVE_DATA_MOUNT_MODE_RDWR | SCE_SAVE_DATA_MOUNT_MODE_CREATE2;
	//   int ret = sceSaveDataMount2(&params, &mount);
	//   if (ret >= 0) {
	//       _save_mount_path = String(mount.data);
	//       _initialized = true;
	//       return CLOUD_OK;
	//   }
	WARN_PRINT("PSNCloudProvider: PS SDK not linked. PlayStation cloud save is inactive.");
	return CLOUD_ERR_NOT_INITIALIZED;
#else
	WARN_PRINT("PSNCloudProvider: Not running on a PlayStation platform.");
	return CLOUD_ERR_NOT_INITIALIZED;
#endif
}

void CloudSavePSN::shutdown() {
#if defined(PLATFORM_PS4) || defined(PLATFORM_PS5)
	if (_initialized) {
		// TODO (NDA): Call sceSaveDataUmount() to release the mount point.
		_initialized = false;
		_save_mount_path = "";
	}
#endif
}

bool CloudSavePSN::is_ready() const {
	return _initialized && !_save_mount_path.is_empty();
}

// ── Core Operations ───────────────────────────────────────────────────────────

CloudSaveProvider::CloudResult CloudSavePSN::upload(const String &p_slot_name, const Ref<Snapshot> &p_snapshot) {
	if (!is_ready()) {
		return CLOUD_ERR_NOT_INITIALIZED;
	}

	PackedByteArray bytes = _serialize_snapshot(p_snapshot);
	ERR_FAIL_COND_V(bytes.is_empty(), CLOUD_ERR_UNKNOWN);

	String path = _make_file_path(p_slot_name);
	String tmp_path = path + ".tmp";

	Ref<FileAccess> f = FileAccess::open(tmp_path, FileAccess::WRITE);
	ERR_FAIL_COND_V_MSG(f.is_null(), CLOUD_ERR_NETWORK, "PSNCloudProvider: Cannot write to save data mount.");

	f->store_buffer(bytes.ptr(), bytes.size());
	f->close();

	// Atomic replace via filesystem rename.
	Ref<DirAccess> da = DirAccess::create(DirAccess::ACCESS_FILESYSTEM);
	Error rename_err = da->rename(tmp_path, path);
	ERR_FAIL_COND_V(rename_err != OK, CLOUD_ERR_NETWORK);

	// TODO (NDA): After writing, call sceSaveDataSyncSaveDataMemory() or
	// equivalent to flush the memory-cached data to disk within the mount.
	// The OS will then handle PS Plus cloud upload transparently.

	print_verbose(vformat("PSNCloudProvider: Wrote slot '%s' to save data mount (%d bytes).", p_slot_name, bytes.size()));
	return CLOUD_OK;
}

CloudSaveProvider::CloudResult CloudSavePSN::download(const String &p_slot_name, Ref<Snapshot> &r_snapshot) {
	// The OS synchronizes PSN cloud saves locally on title launch.
	// We instruct SaveServer to read from local disk instead.
	(void)p_slot_name;
	r_snapshot = Ref<Snapshot>();
	return CLOUD_ERR_NOT_FOUND; // Trigger local fallback in SaveServer.
}

CloudSaveProvider::CloudResult CloudSavePSN::query_slot(const String &p_slot_name, RemoteSlotInfo &r_info) {
	if (!is_ready()) {
		return CLOUD_ERR_NOT_INITIALIZED;
	}

	String path = _make_file_path(p_slot_name);
	if (!FileAccess::exists(path)) {
		return CLOUD_ERR_NOT_FOUND;
	}

	r_info.slot_name = p_slot_name;
	r_info.timestamp = (uint64_t)FileAccess::get_modified_time(path);
	r_info.size_bytes = FileAccess::get_file_as_bytes(path).size();
	r_info.checksum = "";
	return CLOUD_OK;
}

CloudSaveProvider::CloudResult CloudSavePSN::delete_slot(const String &p_slot_name) {
	if (!is_ready()) {
		return CLOUD_ERR_NOT_INITIALIZED;
	}

	String path = _make_file_path(p_slot_name);
	if (FileAccess::exists(path)) {
		DirAccess::remove_absolute(path);
	}
	return CLOUD_OK;
}

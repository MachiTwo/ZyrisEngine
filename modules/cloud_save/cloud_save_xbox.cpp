/**************************************************************************/
/*  cloud_save_xbox.cpp                                                   */
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
// XGameSaveFiles Integration
//
// XGameSaveFiles exposes a regular filesystem folder that the GDK transparently
// syncs with the cloud.  We:
//   1. Call XGameSaveFilesGetFolderWithUIAsync() once in initialize() to obtain
//      the path.
//   2. Read/write .zsav files inside that folder for each slot.
//   3. The GDK OS layer handles actual network upload / conflict resolution UI.
//
// Compile guard: XBOX_GDK_ENABLED
// ─────────────────────────────────────────────────────────────────────────────

#include "cloud_save_xbox.h"

#include "core/error/error_macros.h"
#include "core/io/dir_access.h"
#include "core/io/file_access.h"
#include "core/io/marshalls.h"

#ifdef XBOX_GDK_ENABLED
// GDK header — provided by the Xbox GDK installation.
// #include <XGameSave.h>
// For compilation without the GDK, these are forward-declared:
typedef unsigned long HRESULT;
#define S_OK 0L
#define E_NOTIMPL ((HRESULT)0x80004001L)
// XGameSaveFilesGetFolderWithUIAsync is an async GDK call.
// A real integration would use XAsyncBlock + callbacks.
// Here we document the expected pattern.
#endif

// ── Helpers ───────────────────────────────────────────────────────────────────

String CloudSaveXbox::_make_file_path(const String &p_slot_name) const {
	String name = p_slot_name.replace("/", "_").replace("\\", "_");
	return _save_folder.path_join(name + ".zsav");
}

PackedByteArray CloudSaveXbox::_serialize_snapshot(const Ref<Snapshot> &p_snapshot) const {
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

Ref<Snapshot> CloudSaveXbox::_deserialize_snapshot(const PackedByteArray &p_bytes) const {
	ERR_FAIL_COND_V(p_bytes.is_empty(), Ref<Snapshot>());
	Variant v;
	int bytes_read = 0;
	Error err = decode_variant(v, p_bytes.ptr(), p_bytes.size(), &bytes_read, false);
	ERR_FAIL_COND_V(err != OK, Ref<Snapshot>());
	return Ref<Snapshot>(v);
}

// ── Lifecycle ─────────────────────────────────────────────────────────────────

CloudSaveProvider::CloudResult CloudSaveXbox::initialize() {
#ifdef XBOX_GDK_ENABLED
	// XGameSaveFilesGetFolderWithUIAsync is asynchronous.
	// A production integration would:
	//   1. Allocate an XAsyncBlock on the heap.
	//   2. Call XGameSaveFilesGetFolderWithUIAsync(user, scid, async_block).
	//   3. Wait for the callback or use XAsyncGetStatus(block, true) to block.
	//   4. Call XGameSaveFilesGetFolderWithUIResult(block, path_buf, path_size).
	//
	// Below is the canonical blocking pattern for simplicity:
	//
	//   char folder[MAX_PATH] = {};
	//   XAsyncBlock block = {};
	//   block.queue = nullptr; // Use default queue
	//   HRESULT hr = XGameSaveFilesGetFolderWithUIAsync(user_handle, scid, &block);
	//   if (SUCCEEDED(hr)) {
	//       hr = XAsyncGetStatus(&block, true); // block = synchronous wait
	//       if (SUCCEEDED(hr)) {
	//           size_t folder_size = 0;
	//           hr = XGameSaveFilesGetFolderWithUIResult(&block, MAX_PATH, folder, &folder_size);
	//           if (SUCCEEDED(hr)) {
	//               _save_folder = String(folder);
	//               _initialized = true;
	//               return CLOUD_OK;
	//           }
	//       }
	//   }
	//   WARN_PRINT(vformat("XboxCloudProvider: XGameSaveFilesGetFolderWithUIAsync failed. HRESULT: 0x%08X", hr));
	//   return CLOUD_ERR_NOT_INITIALIZED;

	// Placeholder until GDK headers are available in the build:
	WARN_PRINT("XboxCloudProvider: Xbox GDK headers not linked. Xbox Cloud Save is inactive.");
	return CLOUD_ERR_NOT_INITIALIZED;
#else
	WARN_PRINT("XboxCloudProvider: Engine was not compiled with XBOX_GDK_ENABLED.");
	return CLOUD_ERR_NOT_INITIALIZED;
#endif
}

void CloudSaveXbox::shutdown() {
	_initialized = false;
	_save_folder = "";
}

bool CloudSaveXbox::is_ready() const {
	return _initialized && !_save_folder.is_empty();
}

// ── Core Operations ───────────────────────────────────────────────────────────

CloudSaveProvider::CloudResult CloudSaveXbox::upload(const String &p_slot_name, const Ref<Snapshot> &p_snapshot) {
	if (!is_ready()) {
		return CLOUD_ERR_NOT_INITIALIZED;
	}

	PackedByteArray bytes = _serialize_snapshot(p_snapshot);
	ERR_FAIL_COND_V(bytes.is_empty(), CLOUD_ERR_UNKNOWN);

	String path = _make_file_path(p_slot_name);

	// Atomic write: write to temp file first, then rename.
	// XGameSaveFiles best practice: write to .tmp, then ReplaceFile().
	String tmp_path = path + ".tmp";

	Ref<FileAccess> f = FileAccess::open(tmp_path, FileAccess::WRITE);
	ERR_FAIL_COND_V_MSG(f.is_null(), CLOUD_ERR_NETWORK, vformat("XboxCloudProvider: Cannot open temp file for writing: %s", tmp_path));

	f->store_buffer(bytes.ptr(), bytes.size());
	f->close();

	// Replace the actual save file atomically.
	// On GDK, after this write the OS sync layer picks it up automatically.
	Ref<DirAccess> da = DirAccess::create(DirAccess::ACCESS_FILESYSTEM);
	Error rename_err = da->rename(tmp_path, path);
	ERR_FAIL_COND_V_MSG(rename_err != OK, CLOUD_ERR_NETWORK, vformat("XboxCloudProvider: Rename failed for slot '%s'.", p_slot_name));

	print_verbose(vformat("XboxCloudProvider: Synced slot '%s' to GDK folder (%d bytes).", p_slot_name, bytes.size()));
	return CLOUD_OK;
}

CloudSaveProvider::CloudResult CloudSaveXbox::download(const String &p_slot_name, Ref<Snapshot> &r_snapshot) {
	if (!is_ready()) {
		return CLOUD_ERR_NOT_INITIALIZED;
	}

	String path = _make_file_path(p_slot_name);
	if (!FileAccess::exists(path)) {
		return CLOUD_ERR_NOT_FOUND;
	}

	PackedByteArray bytes = FileAccess::get_file_as_bytes(path);
	if (bytes.is_empty()) {
		return CLOUD_ERR_NOT_FOUND;
	}

	r_snapshot = _deserialize_snapshot(bytes);
	if (r_snapshot.is_null()) {
		WARN_PRINT(vformat("XboxCloudProvider: Deserialization failed for slot '%s'.", p_slot_name));
		return CLOUD_ERR_UNKNOWN;
	}

	print_verbose(vformat("XboxCloudProvider: Read slot '%s' from GDK folder.", p_slot_name));
	return CLOUD_OK;
}

CloudSaveProvider::CloudResult CloudSaveXbox::query_slot(const String &p_slot_name, RemoteSlotInfo &r_info) {
	if (!is_ready()) {
		return CLOUD_ERR_NOT_INITIALIZED;
	}

	String path = _make_file_path(p_slot_name);
	if (!FileAccess::exists(path)) {
		return CLOUD_ERR_NOT_FOUND;
	}

	r_info.slot_name = p_slot_name;
	r_info.size_bytes = FileAccess::get_file_as_bytes(path).size();
	// Use OS modified time as timestamp proxy.
	r_info.timestamp = (uint64_t)FileAccess::get_modified_time(path);
	r_info.checksum = "";
	return CLOUD_OK;
}

CloudSaveProvider::CloudResult CloudSaveXbox::delete_slot(const String &p_slot_name) {
	if (!is_ready()) {
		return CLOUD_ERR_NOT_INITIALIZED;
	}

	String path = _make_file_path(p_slot_name);
	if (FileAccess::exists(path)) {
		DirAccess::remove_absolute(path);
	}
	return CLOUD_OK;
}

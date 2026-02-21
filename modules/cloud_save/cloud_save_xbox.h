/**************************************************************************/
/*  cloud_save_xbox.h                                                     */
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

#pragma once

// ─────────────────────────────────────────────────────────────────────────────
// XboxCloudProvider
//
// Implements CloudSaveProvider using the Xbox GDK XGameSaveFiles API.
//
// XGameSaveFiles is the recommended API for GDK titles (PC Game Pass + Xbox).
// It maps a cloud-synced folder on disk; the engine reads/writes files to that
// folder and the OS handles the actual cloud sync automatically.
//
// Setup requirements:
//   1. Link against xgameruntime.lib (GDK).
//   2. Define XBOX_GDK_ENABLED in build options when targeting Xbox/PC Game Pass.
//   3. XGameSaveFilesGetFolderWithUIAsync must complete before the first
//      upload/download call — this is handled inside initialize().
//
// Thread Safety:
//   XGameSaveFiles uses standard Win32 I/O on the acquired folder path.
//   upload() and download() are safe to call from a worker thread.
//
// Conflict Resolution:
//   The GDK handles conflicts at the OS level and shows a system dialog when
//   needed.  The provider transparently reads/writes the sync folder; conflict
//   outcomes are dictated by the platform.
// ─────────────────────────────────────────────────────────────────────────────

#include "cloud_save_provider.h"

class CloudSaveXbox : public CloudSaveProvider {
	GDCLASS(CloudSaveXbox, CloudSaveProvider);
	bool _initialized = false;
	String _save_folder; // Local path to the XGameSave sync folder

	PackedByteArray _serialize_snapshot(const Ref<Snapshot> &p_snapshot) const;
	Ref<Snapshot> _deserialize_snapshot(const PackedByteArray &p_bytes) const;
	String _make_file_path(const String &p_slot_name) const;

public:
	CloudResult initialize() override;
	void shutdown() override;
	bool is_ready() const override;

	CloudResult upload(const String &p_slot_name, const Ref<Snapshot> &p_snapshot) override;
	CloudResult download(const String &p_slot_name, Ref<Snapshot> &r_snapshot) override;
	CloudResult query_slot(const String &p_slot_name, RemoteSlotInfo &r_info) override;
	CloudResult delete_slot(const String &p_slot_name) override;

	String get_provider_name() const override { return "Xbox"; }
};

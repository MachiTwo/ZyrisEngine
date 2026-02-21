/**************************************************************************/
/*  cloud_save_psn.h                                                      */
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
// PSNCloudProvider
//
// Implements CloudSaveProvider for PlayStation 4 / PlayStation 5.
//
// On PlayStation, cloud save is managed by the OS automatically when the title
// uses the platform's Save Data Management API (sceSaveData* on PS4,
// sceSaveData* on PS5).  The game writes to a local "save data directory"
// and the system handles cloud upload/download via PS Plus on behalf of the
// user.  There is no explicit "upload to PSN" call available to developers
// under NDA.
//
// This provider therefore acts as a pass-through that:
//   1. Writes saves to the designated PS4/PS5 save data mount point.
//   2. Relies on the OS auto-sync to push data to PlayStation Plus cloud.
//   3. Does NOT expose download() — returns CLOUD_ERR_NOT_FOUND to trigger
//      local fallback (the OS will have already synced the local folder).
//
// Compile guard: PLATFORM_PS4 or PLATFORM_PS5
//
// NDA Note:
//   The actual SDK function names (sceAppContentMountSaveData, etc.) are
//   redacted from this public source.  A licensed PlayStation developer should
//   replace the placeholder comments with the appropriate SDK calls.
// ─────────────────────────────────────────────────────────────────────────────

#include "cloud_save_provider.h"

class CloudSavePSN : public CloudSaveProvider {
	GDCLASS(CloudSavePSN, CloudSaveProvider);
	bool _initialized = false;
	String _save_mount_path; // Mount point returned by the PS SDK

	PackedByteArray _serialize_snapshot(const Ref<Snapshot> &p_snapshot) const;
	Ref<Snapshot> _deserialize_snapshot(const PackedByteArray &p_bytes) const;
	String _make_file_path(const String &p_slot_name) const;

public:
	CloudResult initialize() override;
	void shutdown() override;
	bool is_ready() const override;

	// upload() writes to the local save data mount point.
	// The OS handles cloud synchronization transparently.
	CloudResult upload(const String &p_slot_name, const Ref<Snapshot> &p_snapshot) override;

	// download() returns CLOUD_ERR_NOT_FOUND — the OS has already synced the
	// local copy; the SaveServer falls back to reading from local disk.
	CloudResult download(const String &p_slot_name, Ref<Snapshot> &r_snapshot) override;

	CloudResult query_slot(const String &p_slot_name, RemoteSlotInfo &r_info) override;
	CloudResult delete_slot(const String &p_slot_name) override;

	String get_provider_name() const override { return "PlayStation"; }
};

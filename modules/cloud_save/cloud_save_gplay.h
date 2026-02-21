/**************************************************************************/
/*  cloud_save_gplay.h                                                    */
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
// GooglePlayCloudProvider
//
// Implements CloudSaveProvider using the Google Play Games Services
// Saved Games (Snapshots) API for Android.
//
// Architecture:
//   The Google Play Games C++ SDK wraps the Snapshots REST API.  A saved game
//   consists of:
//     • A binary blob  — our serialized Snapshot resource.
//     • Structured metadata (description, cover image, play time, modified).
//
//   Data is stored in the player's Google Drive Application Data Folder
//   (isolated per app, 3 GB limit).
//
//   Up to 5 snapshots can be open simultaneously per session.
//
// Conflict Resolution:
//   The Snapshots API detects conflicts automatically.  If a conflict occurs,
//   download() invokes the conflict callback set via
//   SaveServer.set_cloud_conflict_callback().  If no callback is set, the
//   remote (server) version wins by default.
//
// Compile guard: GOOGLE_PLAY_GAMES_ENABLED
//
// Integration:
//   Link against gpg-cpp-sdk libraries and include gpg/gpg.h.
// ─────────────────────────────────────────────────────────────────────────────

#include "cloud_save_provider.h"

class CloudSaveGPlay : public CloudSaveProvider {
	GDCLASS(CloudSaveGPlay, CloudSaveProvider);
	bool _initialized = false;
	String _client_id; // OAuth2 client ID from Project Settings

	PackedByteArray _serialize_snapshot(const Ref<Snapshot> &p_snapshot) const;
	Ref<Snapshot> _deserialize_snapshot(const PackedByteArray &p_bytes) const;

	// Build the snapshot description metadata.
	static String _make_snapshot_name(const String &p_slot_name);

public:
	void set_client_id(const String &p_id) { _client_id = p_id; }

	CloudResult initialize() override;
	void shutdown() override;
	bool is_ready() const override;

	CloudResult upload(const String &p_slot_name, const Ref<Snapshot> &p_snapshot) override;
	CloudResult download(const String &p_slot_name, Ref<Snapshot> &r_snapshot) override;
	CloudResult query_slot(const String &p_slot_name, RemoteSlotInfo &r_info) override;
	CloudResult delete_slot(const String &p_slot_name) override;

	String get_provider_name() const override { return "Google Play Games"; }
};

/**************************************************************************/
/*  cloud_save_steam.h                                                    */
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
// CloudSaveSteam
//
// Implements CloudSaveProvider using the Steamworks ISteamRemoteStorage API.
//
// Setup requirements (Steam Auto-Cloud is NOT used; we use the Manual API):
//   1. The Steamworks SDK must be linked via the GodotSteam GDExtension or by
//      linking steamworks.lib / libsteam_api.so directly in the project's
//      platform export settings.
//   2. SteamAPI_Init() must be called by the game before SaveServer starts.
//      The engine does NOT call SteamAPI_Init() internally.
//   3. The steam_appid.txt must be present alongside the executable during
//      development (NOT shipped in the final build).
//
// File naming convention inside Steam Remote Storage:
//   "<slot_name>.zsav"  (e.g. "slot_0.zsav")
//
// Conflict Resolution:
//   Uses the ISteamRemoteStorage::FileTimestamp() value to compare local vs.
//   remote modification times. Remote wins if it is strictly newer.
//
// Quota:
//   Each game gets 100 MB of Remote Storage by default.  The provider logs a
//   warning when quota > 80% is used (CLOUD_ERR_QUOTA_EXCEEDED on 100%).
//
// Thread Safety:
//   upload() / download() use ISteamRemoteStorage which is thread-safe after
//   SteamAPI_Init().  SteamAPI_RunCallbacks() must still be driven by the
//   main thread (the game is responsible for this).
// ─────────────────────────────────────────────────────────────────────────────

#include "cloud_save_provider.h"

class CloudSaveSteam : public CloudSaveProvider {
	GDCLASS(CloudSaveSteam, CloudSaveProvider);
	bool _initialized = false;

	// Serialize a Snapshot resource to a raw byte array for Steam storage.
	PackedByteArray _serialize_snapshot(const Ref<Snapshot> &p_snapshot) const;

	// Deserialize bytes from Steam storage back into a Snapshot resource.
	Ref<Snapshot> _deserialize_snapshot(const PackedByteArray &p_bytes) const;

	// Map a Steam EResult error code (int) to a CloudResult.
	CloudResult _map_steam_error(int p_result) const;

	// Build the Steam remote filename for a given slot.
	static String _make_remote_filename(const String &p_slot_name);

public:
	CloudResult initialize() override;
	void shutdown() override;
	bool is_ready() const override;

	CloudResult upload(const String &p_slot_name, const Ref<Snapshot> &p_snapshot) override;
	CloudResult download(const String &p_slot_name, Ref<Snapshot> &r_snapshot) override;
	CloudResult query_slot(const String &p_slot_name, RemoteSlotInfo &r_info) override;
	CloudResult delete_slot(const String &p_slot_name) override;

	String get_provider_name() const override { return "Steam"; }
};

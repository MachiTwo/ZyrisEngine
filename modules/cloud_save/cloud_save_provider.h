/**************************************************************************/
/*  cloud_save_provider.h                                                 */
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

#include "core/error/error_list.h"
#include "core/string/ustring.h"
#include "core/variant/callable.h"
#include "core/variant/dictionary.h"
#include "scene/resources/snapshot.h"

// ─────────────────────────────────────────────────────────────────────────────
// CloudSaveProvider
//
// Abstract interface for cloud save platform backends.
//
// Each platform (Steam, Xbox, PSN, Google Play, Custom HTTP) implements this
// contract. The SaveServer holds a single CloudSaveProvider* and delegates all
// network I/O through it, remaining completely agnostic of the underlying SDK.
//
// Thread Safety:
//   upload() and download() may be called from the save worker thread.
//   Implementations MUST be thread-safe for those two methods.
//   initialize() / shutdown() are called from the main thread only.
//
// Conflict Resolution:
//   When download() detects that the remote snapshot is newer, it returns it
//   directly. The SaveServer compares checksums: if the remote checksum differs
//   from the local one, it treats the remote version as authoritative (last
//   cloud write wins). For advanced conflict handling, the developer may
//   register a conflict_callback via SaveServer.set_cloud_conflict_callback().
// ─────────────────────────────────────────────────────────────────────────────

#include "core/object/ref_counted.h"

class CloudSaveProvider : public RefCounted {
	GDCLASS(CloudSaveProvider, RefCounted);

protected:
	static void _bind_methods();

public:
	// Result codes returned by cloud operations.
	// Kept separate from SaveServer::SaveResult to keep the interface decoupled.
	enum CloudResult {
		CLOUD_OK = 0,
		CLOUD_ERR_NOT_INITIALIZED, // initialize() was not called or failed
		CLOUD_ERR_NOT_AUTHENTICATED, // user is not signed in
		CLOUD_ERR_QUOTA_EXCEEDED, // cloud storage quota reached
		CLOUD_ERR_NETWORK, // generic network / connectivity failure
		CLOUD_ERR_CONFLICT, // a conflict was detected and could not be resolved
		CLOUD_ERR_NOT_FOUND, // requested slot does not exist in the cloud
		CLOUD_ERR_UNAVAILABLE, // platform service temporarily unavailable
		CLOUD_ERR_INVALID_CONFIG, // required credentials / endpoint not set
		CLOUD_ERR_UNKNOWN,
	};

	// Metadata attached to a remote snapshot (used for conflict resolution).
	struct RemoteSlotInfo {
		String slot_name;
		uint64_t timestamp = 0; // Unix epoch, seconds
		String checksum; // Provider-specific (may be etag, md5, sha256…)
		uint64_t size_bytes = 0;
	};

	virtual ~CloudSaveProvider() = default;

	// ── Lifecycle ────────────────────────────────────────────────────────────

	// Called once by SaveServer on startup, after project settings are loaded.
	// Implementations should validate credentials and prepare any SDK handles.
	// Returns CLOUD_OK on success or a descriptive error code.
	virtual CloudResult initialize() = 0;

	// Called once by SaveServer on shutdown before the worker thread exits.
	virtual void shutdown() = 0;

	// Returns true if the provider is ready to accept upload/download calls.
	virtual bool is_ready() const = 0;

	// ── Core Operations ──────────────────────────────────────────────────────

	// Serialize p_snapshot to bytes and upload it to the cloud under p_slot_name.
	// This call BLOCKS until the operation completes (called from worker thread).
	// Returns CLOUD_OK on success.
	virtual CloudResult upload(const String &p_slot_name, const Ref<Snapshot> &p_snapshot) = 0;

	// Download the cloud snapshot for p_slot_name.
	// On success, populates r_snapshot and returns CLOUD_OK.
	// Returns CLOUD_ERR_NOT_FOUND if the slot does not exist remotely.
	// This call BLOCKS (called from worker thread during load).
	virtual CloudResult download(const String &p_slot_name, Ref<Snapshot> &r_snapshot) = 0;

	// Returns remote metadata for a slot without downloading the full blob.
	// Used by the conflict resolution logic.
	virtual CloudResult query_slot(const String &p_slot_name, RemoteSlotInfo &r_info) = 0;

	// Delete a slot from the cloud. Optional — returns CLOUD_OK if unsupported.
	virtual CloudResult delete_slot(const String &p_slot_name) {
		(void)p_slot_name;
		return CLOUD_OK;
	}

	// ── Identification ───────────────────────────────────────────────────────

	// Human-readable name for logging and editor UI (e.g. "Steam", "Xbox").
	virtual String get_provider_name() const = 0;
};

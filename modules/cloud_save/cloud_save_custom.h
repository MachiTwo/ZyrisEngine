/**************************************************************************/
/*  cloud_save_custom.h                                                   */
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
// CustomHttpCloudProvider
//
// Implements CloudSaveProvider for user-hosted VPS / custom REST backends.
//
// REST API Contract (server-side):
//
//   The server must implement the following endpoints:
//
//   POST  /saves/{slot_name}
//         Body:  raw binary (application/octet-stream)
//         Headers: X-Checksum: <sha256hex>
//                  X-Timestamp: <unix_epoch_ms>
//                  Authorization: Bearer <token>
//         Response: 200 OK  { "version": "<server_etag>" }
//                   409 Conflict { "server_timestamp": <int>, "server_checksum": "<hex>" }
//                   413 Too large
//
//   GET   /saves/{slot_name}
//         Headers: Authorization: Bearer <token>
//         Response: 200 OK  raw binary body
//                            Header: X-Timestamp: <unix_epoch_ms>
//                                    X-Checksum: <sha256hex>
//                   404 Not Found
//
//   HEAD  /saves/{slot_name}
//         Returns only headers (X-Timestamp, X-Checksum, Content-Length).
//         Used by query_slot() to avoid downloading the full blob.
//
//   DELETE /saves/{slot_name}
//          Authorization: Bearer <token>
//          Response: 204 No Content
//
// Authentication:
//   Bearer token via the api_key project setting.
//   The auth_url endpoint may optionally be called during initialize() to
//   exchange credentials for a short-lived JWT.
//
// Conflict Resolution:
//   Uses an optimistic locking strategy based on the X-Timestamp header.
//   If the server returns 409, the provider:
//     1. Compares server timestamp vs local snapshot's capture time.
//     2. If server is newer AND checksums differ → conflict detected.
//        • Invokes SaveServer conflict callback, if set.
//        • Otherwise, server wins (remote data is downloaded and used).
//     3. If local is newer → retries the upload (server-side bug or clock skew).
//
// Features:
//   - TLS/HTTPS via HTTPClient (uses the engine's SSL layer).
//   - Configurable retry count and timeout.
//   - Automatic token refresh via auth_url.
//
// Configuration (Project Settings):
//   application/persistence/cloud_save/custom/endpoint    (e.g. "https://saves.mygame.com")
//   application/persistence/cloud_save/custom/api_key     (Bearer token or secret)
//   application/persistence/cloud_save/custom/auth_url    (optional OAuth endpoint)
// ─────────────────────────────────────────────────────────────────────────────

#include "cloud_save_provider.h"

#include "core/io/http_client.h"

class CloudSaveCustom : public CloudSaveProvider {
	GDCLASS(CloudSaveCustom, CloudSaveProvider);
	String _endpoint; // Base URL (no trailing slash)
	String _api_key; // Static bearer token
	String _auth_url; // Optional OAuth URL for token exchange
	String _access_token; // Live JWT (refreshed via _auth_url if set)
	bool _initialized = false;

	// Maximum retries on transient HTTP errors (5xx / timeout).
	static constexpr int MAX_RETRIES = 3;
	// Timeout per request, in seconds.
	static constexpr int REQUEST_TIMEOUT_SEC = 15;

	// Serialize / deserialize helpers.
	PackedByteArray _serialize_snapshot(const Ref<Snapshot> &p_snapshot) const;
	Ref<Snapshot> _deserialize_snapshot(const PackedByteArray &p_bytes) const;

	// Build the full URL for a given slot.
	String _make_slot_url(const String &p_slot_name) const;

	// Compute SHA-256 hex string for the given bytes.
	static String _checksum(const PackedByteArray &p_bytes);

	// Perform an HTTP token exchange against _auth_url and populate _access_token.
	CloudResult _refresh_token();

	// Low-level HTTP helpers. Each returns the HTTP status code (or -1 on error).
	// r_response_body receives the raw response bytes.
	// r_headers receives all response headers as KEY: VALUE strings.
	int _http_post(const String &p_url, const PackedByteArray &p_body,
			const Vector<String> &p_extra_headers,
			PackedByteArray &r_response_body, Vector<String> &r_response_headers);

	int _http_get(const String &p_url, const Vector<String> &p_extra_headers,
			PackedByteArray &r_response_body, Vector<String> &r_response_headers);

	int _http_head(const String &p_url, const Vector<String> &p_extra_headers,
			Vector<String> &r_response_headers);

	int _http_delete(const String &p_url, const Vector<String> &p_extra_headers);

	// Extract a header value by key (case-insensitive) from a headers list.
	static String _get_header(const Vector<String> &p_headers, const String &p_key);

	// Build the Authorization header string.
	String _auth_header() const;

public:
	void set_config(const String &p_endpoint, const String &p_api_key, const String &p_auth_url);

	CloudResult initialize() override;
	void shutdown() override;
	bool is_ready() const override;

	CloudResult upload(const String &p_slot_name, const Ref<Snapshot> &p_snapshot) override;
	CloudResult download(const String &p_slot_name, Ref<Snapshot> &r_snapshot) override;
	CloudResult query_slot(const String &p_slot_name, RemoteSlotInfo &r_info) override;
	CloudResult delete_slot(const String &p_slot_name) override;

	String get_provider_name() const override { return "Custom HTTP"; }
};

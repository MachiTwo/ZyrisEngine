/**************************************************************************/
/*  cloud_save_custom.cpp                                                 */
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

#include "cloud_save_custom.h"

#include "core/crypto/crypto_core.h"
#include "core/error/error_macros.h"
#include "core/io/http_client.h"
#include "core/io/json.h"
#include "core/io/marshalls.h"
#include "core/os/os.h"
#include "core/os/time.h"
#include "core/string/print_string.h"

// ── Utilities ────────────────────────────────────────────────────────────────

void CloudSaveCustom::set_config(const String &p_endpoint, const String &p_api_key, const String &p_auth_url) {
	// Strip trailing slash from endpoint for uniform path construction.
	_endpoint = p_endpoint.rstrip("/");
	_api_key = p_api_key;
	_auth_url = p_auth_url;
}

String CloudSaveCustom::_make_slot_url(const String &p_slot_name) const {
	// URL-encode the slot name to handle spaces and special chars.
	String name = p_slot_name.uri_encode().replace("%2F", "/");
	return _endpoint + "/saves/" + name;
}

String CloudSaveCustom::_checksum(const PackedByteArray &p_bytes) {
	// SHA-256 via the engine's CryptoCore.
	unsigned char digest[32] = {};
	CryptoCore::sha256(p_bytes.ptr(), p_bytes.size(), digest);
	String hex;
	for (int i = 0; i < 32; i++) {
		hex += String::num_int64(digest[i] >> 4, 16);
		hex += String::num_int64(digest[i] & 0xF, 16);
	}
	return hex;
}

String CloudSaveCustom::_auth_header() const {
	const String &token = _access_token.is_empty() ? _api_key : _access_token;
	return "Authorization: Bearer " + token;
}

String CloudSaveCustom::_get_header(const Vector<String> &p_headers, const String &p_key) {
	String lower_key = p_key.to_lower();
	for (const String &h : p_headers) {
		int colon = h.find(":");
		if (colon < 0) {
			continue;
		}
		String k = h.substr(0, colon).strip_edges().to_lower();
		if (k == lower_key) {
			return h.substr(colon + 1).strip_edges();
		}
	}
	return "";
}

PackedByteArray CloudSaveCustom::_serialize_snapshot(const Ref<Snapshot> &p_snapshot) const {
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

Ref<Snapshot> CloudSaveCustom::_deserialize_snapshot(const PackedByteArray &p_bytes) const {
	ERR_FAIL_COND_V(p_bytes.is_empty(), Ref<Snapshot>());
	Variant v;
	int bytes_read = 0;
	Error err = decode_variant(v, p_bytes.ptr(), p_bytes.size(), &bytes_read, false);
	ERR_FAIL_COND_V(err != OK, Ref<Snapshot>());
	return Ref<Snapshot>(v);
}

// ── HTTP Core ─────────────────────────────────────────────────────────────────
//
// We use Godot's HTTPClient to avoid adding libcurl as a dependency.
// HTTPClient operates synchronously when poll() is called in a tight loop,
// which is safe inside the SaveServer worker thread.

int CloudSaveCustom::_http_post(const String &p_url,
		const PackedByteArray &p_body,
		const Vector<String> &p_extra_headers,
		PackedByteArray &r_response_body,
		Vector<String> &r_response_headers) {
	Ref<HTTPClient> client;
	client.instantiate();

	// Parse the URL into scheme + host + path.
	String scheme, host, path;
	int port = -1;
	bool use_ssl = false;
	{
		String url = p_url;
		if (url.begins_with("https://")) {
			use_ssl = true;
			url = url.substr(8);
			port = 443;
		} else if (url.begins_with("http://")) {
			url = url.substr(7);
			port = 80;
		}
		int slash = url.find("/");
		if (slash >= 0) {
			host = url.substr(0, slash);
			path = url.substr(slash);
		} else {
			host = url;
			path = "/";
		}
		int colon = host.find(":");
		if (colon >= 0) {
			port = host.substr(colon + 1).to_int();
			host = host.substr(0, colon);
		}
	}

	Error err = client->connect_to_host(host, port, use_ssl ? Ref<TLSOptions>(TLSOptions::client()) : Ref<TLSOptions>());
	if (err != OK) {
		WARN_PRINT(vformat("CustomHttpCloudProvider: Cannot connect to '%s'.", host));
		return -1;
	}

	// Wait for connection.
	int timeout_ms = REQUEST_TIMEOUT_SEC * 1000;
	int waited_ms = 0;
	while (client->get_status() == HTTPClient::STATUS_CONNECTING ||
			client->get_status() == HTTPClient::STATUS_RESOLVING) {
		client->poll();
		OS::get_singleton()->delay_usec(5000); // 5 ms
		waited_ms += 5;
		if (waited_ms > timeout_ms) {
			WARN_PRINT("CustomHttpCloudProvider: Connection timeout.");
			return -1;
		}
	}
	if (client->get_status() != HTTPClient::STATUS_CONNECTED) {
		WARN_PRINT(vformat("CustomHttpCloudProvider: Connection failed. Status: %d", client->get_status()));
		return -1;
	}

	// Build header list.
	Vector<String> headers = p_extra_headers;
	headers.push_back("Content-Type: application/octet-stream");
	headers.push_back(vformat("Content-Length: %d", p_body.size()));
	headers.push_back(_auth_header());

	err = client->request_raw(HTTPClient::METHOD_POST, path, headers, p_body);
	if (err != OK) {
		WARN_PRINT("CustomHttpCloudProvider: POST request failed.");
		return -1;
	}

	// Wait for response headers.
	waited_ms = 0;
	while (client->get_status() == HTTPClient::STATUS_REQUESTING) {
		client->poll();
		OS::get_singleton()->delay_usec(5000);
		waited_ms += 5;
		if (waited_ms > timeout_ms) {
			return -1;
		}
	}
	if (client->get_status() != HTTPClient::STATUS_BODY &&
			client->get_status() != HTTPClient::STATUS_CONNECTED) {
		return -1;
	}

	r_response_headers = client->get_response_headers_as_array();
	int status_code = client->get_response_code();

	// Read body.
	while (client->get_status() == HTTPClient::STATUS_BODY) {
		client->poll();
		PackedByteArray chunk = client->read_response_body_chunk();
		if (chunk.size() > 0) {
			r_response_body.append_array(chunk);
		}
		OS::get_singleton()->delay_usec(1000);
	}

	return status_code;
}

int CloudSaveCustom::_http_get(const String &p_url,
		const Vector<String> &p_extra_headers,
		PackedByteArray &r_response_body,
		Vector<String> &r_response_headers) {
	Ref<HTTPClient> client;
	client.instantiate();

	String host, path;
	int port = -1;
	bool use_ssl = false;
	{
		String url = p_url;
		if (url.begins_with("https://")) {
			use_ssl = true;
			url = url.substr(8);
			port = 443;
		} else if (url.begins_with("http://")) {
			url = url.substr(7);
			port = 80;
		}
		int slash = url.find("/");
		if (slash >= 0) {
			host = url.substr(0, slash);
			path = url.substr(slash);
		} else {
			host = url;
			path = "/";
		}
		int colon = host.find(":");
		if (colon >= 0) {
			port = host.substr(colon + 1).to_int();
			host = host.substr(0, colon);
		}
	}

	Error err = client->connect_to_host(host, port, use_ssl ? Ref<TLSOptions>(TLSOptions::client()) : Ref<TLSOptions>());
	if (err != OK) {
		return -1;
	}

	int timeout_ms = REQUEST_TIMEOUT_SEC * 1000;
	int waited_ms = 0;
	while (client->get_status() == HTTPClient::STATUS_CONNECTING ||
			client->get_status() == HTTPClient::STATUS_RESOLVING) {
		client->poll();
		OS::get_singleton()->delay_usec(5000);
		waited_ms += 5;
		if (waited_ms > timeout_ms) {
			return -1;
		}
	}
	if (client->get_status() != HTTPClient::STATUS_CONNECTED) {
		return -1;
	}

	Vector<String> headers = p_extra_headers;
	headers.push_back(_auth_header());

	err = client->request(HTTPClient::METHOD_GET, path, headers);
	if (err != OK) {
		return -1;
	}

	waited_ms = 0;
	while (client->get_status() == HTTPClient::STATUS_REQUESTING) {
		client->poll();
		OS::get_singleton()->delay_usec(5000);
		waited_ms += 5;
		if (waited_ms > timeout_ms) {
			return -1;
		}
	}

	r_response_headers = client->get_response_headers_as_array();
	int status_code = client->get_response_code();

	while (client->get_status() == HTTPClient::STATUS_BODY) {
		client->poll();
		PackedByteArray chunk = client->read_response_body_chunk();
		if (chunk.size() > 0) {
			r_response_body.append_array(chunk);
		}
		OS::get_singleton()->delay_usec(1000);
	}

	return status_code;
}

int CloudSaveCustom::_http_head(const String &p_url,
		const Vector<String> &p_extra_headers,
		Vector<String> &r_response_headers) {
	PackedByteArray dummy_body;
	// Reuse GET but only read headers (HEAD is not universally supported by all simple server setups).
	// A production server should implement HEAD properly.
	return _http_get(p_url, p_extra_headers, dummy_body, r_response_headers);
}

int CloudSaveCustom::_http_delete(const String &p_url, const Vector<String> &p_extra_headers) {
	Ref<HTTPClient> client;
	client.instantiate();

	String host, path;
	int port = -1;
	bool use_ssl = false;
	{
		String url = p_url;
		if (url.begins_with("https://")) {
			use_ssl = true;
			url = url.substr(8);
			port = 443;
		} else if (url.begins_with("http://")) {
			url = url.substr(7);
			port = 80;
		}
		int slash = url.find("/");
		if (slash >= 0) {
			host = url.substr(0, slash);
			path = url.substr(slash);
		} else {
			host = url;
			path = "/";
		}
		int colon = host.find(":");
		if (colon >= 0) {
			port = host.substr(colon + 1).to_int();
			host = host.substr(0, colon);
		}
	}

	client->connect_to_host(host, port, use_ssl ? Ref<TLSOptions>(TLSOptions::client()) : Ref<TLSOptions>());

	int timeout_ms = REQUEST_TIMEOUT_SEC * 1000;
	int waited_ms = 0;
	while (client->get_status() == HTTPClient::STATUS_CONNECTING ||
			client->get_status() == HTTPClient::STATUS_RESOLVING) {
		client->poll();
		OS::get_singleton()->delay_usec(5000);
		waited_ms += 5;
		if (waited_ms > timeout_ms) {
			return -1;
		}
	}
	if (client->get_status() != HTTPClient::STATUS_CONNECTED) {
		return -1;
	}

	Vector<String> headers = p_extra_headers;
	headers.push_back(_auth_header());
	client->request(HTTPClient::METHOD_DELETE, path, headers);

	waited_ms = 0;
	while (client->get_status() == HTTPClient::STATUS_REQUESTING) {
		client->poll();
		OS::get_singleton()->delay_usec(5000);
		waited_ms += 5;
		if (waited_ms > timeout_ms) {
			return -1;
		}
	}

	return client->get_response_code();
}

// ── Token Refresh ─────────────────────────────────────────────────────────────

CloudSaveProvider::CloudResult CloudSaveCustom::_refresh_token() {
	if (_auth_url.is_empty()) {
		// No OAuth endpoint configured; use static api_key as Bearer token.
		_access_token = _api_key;
		return CLOUD_OK;
	}

	// POST to auth_url with the api_key to obtain a short-lived JWT.
	// Expected request body: { "api_key": "<key>" }
	// Expected response:     { "access_token": "<jwt>", "expires_in": <seconds> }

	String json_body = vformat("{\"api_key\":\"%s\"}", _api_key.c_escape());
	PackedByteArray body;
	body.resize(json_body.utf8().size());
	memcpy(body.ptrw(), json_body.utf8().get_data(), json_body.utf8().size());

	Vector<String> extra_headers;
	extra_headers.push_back("Content-Type: application/json");

	PackedByteArray response_body;
	Vector<String> response_headers;
	int code = _http_post(_auth_url, body, extra_headers, response_body, response_headers);

	if (code != 200) {
		WARN_PRINT(vformat("CustomHttpCloudProvider: Token refresh failed (HTTP %d).", code));
		return CLOUD_ERR_NOT_AUTHENTICATED;
	}

	String response_str;
	response_str.parse_utf8((const char *)response_body.ptr(), response_body.size());

	Variant parsed;
	String parse_err;
	int parse_line;
	Error parse_result = JSON::parse(response_str, parsed, parse_err, parse_line);
	ERR_FAIL_COND_V_MSG(parse_result != OK, CLOUD_ERR_NOT_AUTHENTICATED, "CustomHttpCloudProvider: Token response is not valid JSON.");

	Dictionary dict = parsed;
	String token = dict.get("access_token", "");
	if (token.is_empty()) {
		WARN_PRINT("CustomHttpCloudProvider: Token response missing 'access_token' field.");
		return CLOUD_ERR_NOT_AUTHENTICATED;
	}

	_access_token = token;
	return CLOUD_OK;
}

// ── Lifecycle ─────────────────────────────────────────────────────────────────

CloudSaveProvider::CloudResult CloudSaveCustom::initialize() {
	if (_endpoint.is_empty()) {
		ERR_PRINT("CustomHttpCloudProvider: Endpoint is empty. Set it in Project Settings "
				  "> Application > Persistence > Cloud Save > Custom > Endpoint.");
		return CLOUD_ERR_INVALID_CONFIG;
	}
	if (_api_key.is_empty()) {
		ERR_PRINT("CustomHttpCloudProvider: API key is empty. Set it in Project Settings "
				  "> Application > Persistence > Cloud Save > Custom > API Key.");
		return CLOUD_ERR_INVALID_CONFIG;
	}

	// Attempt to obtain an access token.
	CloudResult auth_result = _refresh_token();
	if (auth_result != CLOUD_OK) {
		return auth_result;
	}

	// Verify connectivity with a HEAD request against the base endpoint.
	Vector<String> dummy_headers;
	PackedByteArray dummy_body;
	int probe_code = _http_get(_endpoint + "/health", {}, dummy_body, dummy_headers);
	if (probe_code < 0 || probe_code >= 500) {
		WARN_PRINT(vformat("CustomHttpCloudProvider: Health check failed (code %d). Will retry on first save.", probe_code));
		// Non-fatal: we still mark as initialized. Upload/download will fail gracefully.
	}

	_initialized = true;
	print_verbose(vformat("CustomHttpCloudProvider: Connected to '%s'.", _endpoint));
	return CLOUD_OK;
}

void CloudSaveCustom::shutdown() {
	_initialized = false;
	_access_token = "";
}

bool CloudSaveCustom::is_ready() const {
	return _initialized && !_endpoint.is_empty();
}

// ── Core Operations ───────────────────────────────────────────────────────────

CloudSaveProvider::CloudResult CloudSaveCustom::upload(const String &p_slot_name, const Ref<Snapshot> &p_snapshot) {
	if (!is_ready()) {
		return CLOUD_ERR_NOT_INITIALIZED;
	}

	PackedByteArray bytes = _serialize_snapshot(p_snapshot);
	ERR_FAIL_COND_V(bytes.is_empty(), CLOUD_ERR_UNKNOWN);

	String url = _make_slot_url(p_slot_name);
	String checksum = _checksum(bytes);
	uint64_t now_ms = (uint64_t)(Time::get_singleton()->get_unix_time_from_system() * 1000.0);

	Vector<String> extra_headers;
	extra_headers.push_back("X-Checksum: " + checksum);
	extra_headers.push_back(vformat("X-Timestamp: %d", now_ms));

	for (int attempt = 0; attempt < MAX_RETRIES; attempt++) {
		if (attempt > 0) {
			// Exponential back-off: 1s, 2s, 4s.
			OS::get_singleton()->delay_usec((1 << (attempt - 1)) * 1000000);
		}

		PackedByteArray response_body;
		Vector<String> response_headers;
		int code = _http_post(url, bytes, extra_headers, response_body, response_headers);

		if (code == 200 || code == 201) {
			print_verbose(vformat("CustomHttpCloudProvider: Uploaded slot '%s' (%d bytes, checksum %s).",
					p_slot_name, bytes.size(), checksum));
			return CLOUD_OK;
		}

		if (code == 409) {
			// Conflict: server has a newer version.
			// Parse server's X-Timestamp from response headers.
			String server_ts_str = _get_header(response_headers, "X-Timestamp");
			uint64_t server_ts = server_ts_str.to_int();

			if (server_ts > now_ms) {
				// Server is strictly newer — do not overwrite.
				WARN_PRINT(vformat("CustomHttpCloudProvider: Conflict on slot '%s' — server has a newer version (server_ts=%d > local_ts=%d). "
								   "Use SaveServer.set_cloud_conflict_callback() to handle this.",
						p_slot_name, server_ts, now_ms));
				return CLOUD_ERR_CONFLICT;
			}
			// Local is newer or equal — this may be a clock skew; retry.
			continue;
		}

		if (code == 401 || code == 403) {
			// Token expired — attempt one refresh and retry immediately.
			if (attempt == 0 && !_auth_url.is_empty()) {
				CloudResult refresh_result = _refresh_token();
				if (refresh_result == CLOUD_OK) {
					// Re-push the auth header with the new token.
					// The retry loop will re-issue the request with the updated _access_token.
					attempt--; // Don't count this as a real attempt.
					continue;
				}
			}
			WARN_PRINT(vformat("CustomHttpCloudProvider: Authentication failed for slot '%s' (HTTP %d).", p_slot_name, code));
			return CLOUD_ERR_NOT_AUTHENTICATED;
		}

		if (code == 413) {
			WARN_PRINT(vformat("CustomHttpCloudProvider: Slot '%s' exceeds server size limit.", p_slot_name));
			return CLOUD_ERR_QUOTA_EXCEEDED;
		}

		if (code < 0 || code >= 500) {
			WARN_PRINT(vformat("CustomHttpCloudProvider: Server error on upload (HTTP %d). Retry %d/%d.", code, attempt + 1, MAX_RETRIES));
			continue; // Retry on 5xx.
		}

		// Unexpected non-retriable error.
		WARN_PRINT(vformat("CustomHttpCloudProvider: Unexpected HTTP %d on upload of slot '%s'.", code, p_slot_name));
		return CLOUD_ERR_NETWORK;
	}

	WARN_PRINT(vformat("CustomHttpCloudProvider: Upload of slot '%s' failed after %d retries.", p_slot_name, MAX_RETRIES));
	return CLOUD_ERR_NETWORK;
}

CloudSaveProvider::CloudResult CloudSaveCustom::download(const String &p_slot_name, Ref<Snapshot> &r_snapshot) {
	if (!is_ready()) {
		return CLOUD_ERR_NOT_INITIALIZED;
	}

	String url = _make_slot_url(p_slot_name);

	for (int attempt = 0; attempt < MAX_RETRIES; attempt++) {
		if (attempt > 0) {
			OS::get_singleton()->delay_usec((1 << (attempt - 1)) * 1000000);
		}

		PackedByteArray response_body;
		Vector<String> response_headers;
		int code = _http_get(url, {}, response_body, response_headers);

		if (code == 200) {
			// Validate checksum if server provides one.
			String server_checksum = _get_header(response_headers, "X-Checksum");
			if (!server_checksum.is_empty()) {
				String local_checksum = _checksum(response_body);
				if (local_checksum != server_checksum) {
					WARN_PRINT(vformat("CustomHttpCloudProvider: Checksum mismatch on download of '%s'. "
									   "Expected '%s', got '%s'. Retrying.",
							p_slot_name, server_checksum, local_checksum));
					continue;
				}
			}

			r_snapshot = _deserialize_snapshot(response_body);
			if (r_snapshot.is_null()) {
				WARN_PRINT(vformat("CustomHttpCloudProvider: Deserialization failed for slot '%s'.", p_slot_name));
				return CLOUD_ERR_UNKNOWN;
			}

			print_verbose(vformat("CustomHttpCloudProvider: Downloaded slot '%s' (%d bytes).", p_slot_name, response_body.size()));
			return CLOUD_OK;
		}

		if (code == 404) {
			return CLOUD_ERR_NOT_FOUND;
		}

		if (code == 401 || code == 403) {
			if (attempt == 0 && !_auth_url.is_empty()) {
				_refresh_token();
				attempt--;
				continue;
			}
			return CLOUD_ERR_NOT_AUTHENTICATED;
		}

		if (code < 0 || code >= 500) {
			continue; // Retry.
		}

		WARN_PRINT(vformat("CustomHttpCloudProvider: Unexpected HTTP %d on download of slot '%s'.", code, p_slot_name));
		return CLOUD_ERR_NETWORK;
	}

	return CLOUD_ERR_NETWORK;
}

CloudSaveProvider::CloudResult CloudSaveCustom::query_slot(const String &p_slot_name, RemoteSlotInfo &r_info) {
	if (!is_ready()) {
		return CLOUD_ERR_NOT_INITIALIZED;
	}

	String url = _make_slot_url(p_slot_name);
	Vector<String> response_headers;

	// Use HEAD (or GET via our helper) to fetch only metadata.
	PackedByteArray dummy;
	int code = _http_get(url + "?meta=1", {}, dummy, response_headers);

	if (code == 404) {
		return CLOUD_ERR_NOT_FOUND;
	}
	if (code != 200) {
		return CLOUD_ERR_NETWORK;
	}

	r_info.slot_name = p_slot_name;
	r_info.timestamp = (uint64_t)_get_header(response_headers, "X-Timestamp").to_int();
	r_info.checksum = _get_header(response_headers, "X-Checksum");
	r_info.size_bytes = (uint64_t)_get_header(response_headers, "Content-Length").to_int();

	return CLOUD_OK;
}

CloudSaveProvider::CloudResult CloudSaveCustom::delete_slot(const String &p_slot_name) {
	if (!is_ready()) {
		return CLOUD_ERR_NOT_INITIALIZED;
	}

	String url = _make_slot_url(p_slot_name);
	int code = _http_delete(url, {});

	if (code == 204 || code == 200 || code == 404) {
		return CLOUD_OK; // 404 means already deleted — treat as success.
	}

	WARN_PRINT(vformat("CustomHttpCloudProvider: Delete of slot '%s' returned HTTP %d.", p_slot_name, code));
	return CLOUD_ERR_NETWORK;
}

/**************************************************************************/
/*  virtual_device.h                                                      */
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

#include "scene/gui/control.h"

class VirtualDevice : public Control {
	GDCLASS(VirtualDevice, Control);

public:
	enum VisibilityMode {
		VISIBILITY_ALWAYS,
		VISIBILITY_TOUCHSCREEN_ONLY,
	};

	enum InputMode {
		INPUT_MODE_NATIVE, // Current behavior - real virtual device
		INPUT_MODE_EMULATED, // New mode - simple emulation like TouchScreenButton
	};

private:
	int device = 0;
	VisibilityMode visibility_mode = VISIBILITY_ALWAYS;
	BitField<MouseButtonMask> action_mask = MouseButtonMask::LEFT;

	// Emulated mode properties
	InputMode input_mode = INPUT_MODE_NATIVE;
	bool emit_input_events = true;
	bool emit_signals = true;
	Vector<StringName> current_emulated_actions;

protected:
	// Enhanced multitouch state tracking
	struct TouchState {
		int index = -1; // Touch index (-1=none, -2=mouse, 0+=fingers)
		Vector2 position; // Current touch position
		Vector2 start_position; // Initial touch position
		bool is_active = false; // Whether this touch is currently tracking
		bool is_primary = false; // Primary touch for this device
		float pressure = 0.0f; // Touch pressure (if supported)
		float timestamp = 0.0f; // When touch started
	};

	TouchState primary_touch; // Main touch controlling the device
	Vector<TouchState> active_touches; // All touches currently on device
	bool pressed = false;
	bool hovering = false;

	// Multitouch configuration
	int max_touches = 10; // Maximum simultaneous touches to track
	bool priority_to_first_touch = true; // First touch gets priority

	// Common properties matching BaseButton for familiarity

	virtual void _update_theme_item_cache() override;

	void _notification(int p_what);
	void _on_input_type_changed(int p_type);
	static void _bind_methods();

	// Input handling core
	virtual void gui_input(const Ref<InputEvent> &p_event) override;

	// Enhanced multitouch methods
	TouchState *find_touch_state(int p_index);
	TouchState *add_touch_state(int p_index, const Vector2 &p_pos);
	void remove_touch_state(int p_index);
	void update_primary_touch();
	void cleanup_inactive_touches();

	// Utility functions
	static bool is_valid_touch_index(int p_index); // Centralized validation

	// virtual hooks for subclasses
	virtual void _on_touch_down(int p_index, const Vector2 &p_pos);
	virtual void _on_touch_up(int p_index, const Vector2 &p_pos);
	virtual void _on_drag(int p_index, const Vector2 &p_pos, const Vector2 &p_relative);

	// Enhanced multitouch hooks
	virtual void _on_multitouch_begin(const Vector<TouchState> &p_touches);
	virtual void _on_multitouch_update(const Vector<TouchState> &p_touches);
	virtual void _on_multitouch_end(const Vector<TouchState> &p_touches);
	virtual void _on_touch_conflict(int p_existing_index, int p_new_index);

	virtual void pressed_state_changed(); // Called when pressed changes

	void _update_emulated_actions();
	void _send_emulated_action(const StringName &p_action, bool p_pressed);

	// Emulated mode virtual hooks for subclasses
	virtual void _get_emulated_actions(Vector<StringName> &r_actions) const = 0;
	virtual void _on_emulated_action_pressed(const StringName &p_action) {}
	virtual void _on_emulated_action_released(const StringName &p_action) {}

public:
	enum DrawMode {
		DRAW_NORMAL,
		DRAW_PRESSED,
		DRAW_HOVER,
		DRAW_HOVER_PRESSED,
	};

	DrawMode get_draw_mode() const;

	void set_device(int p_device);
	int get_device() const;

	void set_visibility_mode(VisibilityMode p_mode);
	VisibilityMode get_visibility_mode() const;

	// Input mode API
	void set_input_mode(InputMode p_mode);
	InputMode get_input_mode() const;

	// Emulated mode configuration
	void set_emit_input_events(bool p_emit);
	bool is_emitting_input_events() const;
	void set_emit_signals(bool p_emit);
	bool is_emitting_signals() const;

	bool is_pressed() const;
	bool is_hovered() const;

	// Enhanced multitouch API
	int get_current_touch_index() const;
	const TouchState &get_primary_touch() const;
	const Vector<TouchState> &get_active_touches() const;
	int get_active_touch_count() const;

	// Multitouch configuration
	void set_max_touches(int p_max);
	int get_max_touches() const;
	void set_priority_to_first_touch(bool p_priority);
	bool get_priority_to_first_touch() const;

	// Touch state queries
	bool is_touch_active(int p_index) const;
	Vector2 get_touch_position(int p_index) const;
	Vector2 get_touch_start_position(int p_index) const;
	float get_touch_pressure(int p_index) const;
	float get_touch_duration(int p_index) const;

	VirtualDevice();
	virtual ~VirtualDevice() = default;
};

VARIANT_ENUM_CAST(VirtualDevice::VisibilityMode);
VARIANT_ENUM_CAST(VirtualDevice::DrawMode);
VARIANT_ENUM_CAST(VirtualDevice::InputMode);

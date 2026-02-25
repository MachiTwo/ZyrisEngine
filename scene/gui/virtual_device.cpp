/**************************************************************************/
/*  virtual_device.cpp                                                    */
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

#include "virtual_device.h"

#include "core/config/engine.h"
#include "core/input/input.h"
#include "core/os/os.h"
#include "scene/main/viewport.h"
#include "scene/theme/theme_db.h"
#include "servers/display/display_server.h"

void VirtualDevice::_update_theme_item_cache() {
	// Base implementation - can be overridden by subclasses
}

void VirtualDevice::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			if (!Engine::get_singleton()->is_editor_hint()) {
				Input *input = Input::get_singleton();
				ERR_FAIL_NULL(input);

				if (!input->is_connected(SNAME("last_input_device_changed"), callable_mp(this, &VirtualDevice::_on_input_type_changed))) {
					input->connect(SNAME("last_input_device_changed"), callable_mp(this, &VirtualDevice::_on_input_type_changed));
				}

				// Initial check
				if (visibility_mode == VISIBILITY_TOUCHSCREEN_ONLY) {
					_on_input_type_changed(Input::get_singleton()->get_last_input_type());
				}
			}
			_update_theme_item_cache();
		} break;
		case NOTIFICATION_THEME_CHANGED: {
			_update_theme_item_cache();
			queue_redraw();
		} break;
		case NOTIFICATION_MOUSE_ENTER: {
			hovering = true;
			queue_redraw();
		} break;
		case NOTIFICATION_MOUSE_EXIT: {
			hovering = false;
			queue_redraw();
		} break;
		case NOTIFICATION_VISIBILITY_CHANGED:
		case NOTIFICATION_EXIT_TREE: {
			if (p_what == NOTIFICATION_EXIT_TREE && !Engine::get_singleton()->is_editor_hint()) {
				Input *input = Input::get_singleton();
				if (input && input->is_connected(SNAME("last_input_device_changed"), callable_mp(this, &VirtualDevice::_on_input_type_changed))) {
					input->disconnect(SNAME("last_input_device_changed"), callable_mp(this, &VirtualDevice::_on_input_type_changed));
				}
			}

			if (p_what == NOTIFICATION_VISIBILITY_CHANGED && is_visible_in_tree()) {
				break;
			}
			// Reset state if hidden or removed
			if (pressed) {
				pressed = false;
				active_touches.clear();
				primary_touch = TouchState();
				_on_multitouch_end(active_touches);
				pressed_state_changed();
			}
			hovering = false;
			queue_redraw();
		} break;
	}
}

void VirtualDevice::_on_input_type_changed(int p_type) {
	if (visibility_mode != VISIBILITY_TOUCHSCREEN_ONLY) {
		return;
	}

	if (p_type == Input::LAST_INPUT_TOUCH) {
		show();
	} else if (p_type == Input::LAST_INPUT_KEYBOARD_MOUSE || p_type == Input::LAST_INPUT_JOYPAD) {
		hide();
	} else if (p_type == Input::LAST_INPUT_UNKNOWN) {
		// Fallback to DisplayServer capability check if we don't have recent history
		if (!DisplayServer::get_singleton()->is_touchscreen_available()) {
			hide();
		} else {
			show();
		}
	}
}

void VirtualDevice::gui_input(const Ref<InputEvent> &p_event) {
	ERR_FAIL_COND(p_event.is_null());
	Input *input = Input::get_singleton();
	ERR_FAIL_NULL(input);

	Ref<InputEventScreenTouch> st = p_event;
	if (st.is_valid()) {
		// Validate touch index
		ERR_FAIL_COND(!is_valid_touch_index(st->get_index())); // -2 is mouse, -1 is no touch, 0+ are touch points
		if (st->is_pressed()) {
			// Enhanced Touch Down - multitouch always enabled
			TouchState *existing_touch = find_touch_state(st->get_index());
			if (existing_touch) {
				// Touch conflict - same index already active
				_on_touch_conflict(existing_touch->index, st->get_index());
				return;
			}

			// Add new touch
			TouchState *new_touch = add_touch_state(st->get_index(), st->get_position());
			if (new_touch) {
				// Update primary touch if needed
				if (!primary_touch.is_active || priority_to_first_touch) {
					update_primary_touch();
				}

				// Check if this should become the controlling touch
				if (new_touch->is_primary) {
					pressed = true;
					_on_touch_down(new_touch->index, new_touch->position);
					if (input_mode == INPUT_MODE_EMULATED) {
						_update_emulated_actions();
					}
					pressed_state_changed();
					queue_redraw();
				}

				// Notify multitouch state change
				_on_multitouch_update(active_touches);
			}
			accept_event();
		} else {
			// Enhanced Touch Up
			TouchState *touch_state = find_touch_state(st->get_index());
			if (touch_state) {
				// Store if this was the primary touch
				bool was_primary = touch_state->is_primary;

				// Remove the touch
				remove_touch_state(st->get_index());

				// Update primary touch if needed
				if (was_primary) {
					update_primary_touch();

					if (primary_touch.is_active) {
						// New primary touch takes over
						_on_touch_down(primary_touch.index, primary_touch.position);
						if (input_mode == INPUT_MODE_EMULATED) {
							_update_emulated_actions();
						}
					} else {
						// No more touches - device released
						pressed = false;
						_on_touch_up(st->get_index(), st->get_position());
						if (input_mode == INPUT_MODE_EMULATED) {
							_update_emulated_actions();
						}
						pressed_state_changed();
					}
				} else {
					// Non-primary touch released
					_on_touch_up(st->get_index(), st->get_position());
				}

				// Notify multitouch state change
				if (active_touches.size() > 0) {
					_on_multitouch_update(active_touches);
				} else {
					_on_multitouch_end(active_touches);
				}

				queue_redraw();
			}
			accept_event();
		}
		return;
	}

	Ref<InputEventScreenDrag> sd = p_event;
	if (sd.is_valid()) {
		// Enhanced drag handling with multitouch support
		ERR_FAIL_COND(!is_valid_touch_index(sd->get_index()));
		TouchState *touch_state = find_touch_state(sd->get_index());
		if (touch_state) {
			// Update touch position
			touch_state->position = sd->get_position();

			// Handle drag for this touch
			_on_drag(sd->get_index(), sd->get_position(), sd->get_relative());

			if (input_mode == INPUT_MODE_EMULATED && touch_state->is_primary) {
				_update_emulated_actions();
			}

			// Notify multitouch update if this is the primary touch
			if (touch_state->is_primary) {
				_on_multitouch_update(active_touches);
			}

			accept_event();
		}
		return;
	}

	// Enhanced Mouse fallback for testing on Desktop
	Ref<InputEventMouseButton> mb = p_event;
	if (mb.is_valid()) {
		if (mb->is_pressed() && mb->get_button_index() == MouseButton::LEFT) {
			if (!primary_touch.is_active) {
				// Mouse acts as touch index -2
				TouchState *mouse_touch = add_touch_state(-2, mb->get_position());
				if (mouse_touch) {
					mouse_touch->pressure = 1.0f; // Mouse has full pressure
					update_primary_touch();
					pressed = true;
					_on_touch_down(-2, mb->get_position());
					if (input_mode == INPUT_MODE_EMULATED) {
						_update_emulated_actions();
					}
					pressed_state_changed();
					_on_multitouch_update(active_touches);
					queue_redraw();
				}
			}
			accept_event();
		} else if (mb->is_released() && mb->get_button_index() == MouseButton::LEFT) {
			TouchState *mouse_touch = find_touch_state(-2);
			if (mouse_touch) {
				bool was_primary = mouse_touch->is_primary;
				remove_touch_state(-2);
				if (was_primary) {
					update_primary_touch();
					if (primary_touch.is_active) {
						_on_touch_down(primary_touch.index, primary_touch.position);
						if (input_mode == INPUT_MODE_EMULATED) {
							_update_emulated_actions();
						}
					} else {
						pressed = false;
						_on_touch_up(-2, mb->get_position());
						if (input_mode == INPUT_MODE_EMULATED) {
							_update_emulated_actions();
						}
						pressed_state_changed();
					}
				} else {
					_on_touch_up(-2, mb->get_position());
				}
				_on_multitouch_end(active_touches);

				queue_redraw();
			}
			accept_event();
		}
		return;
	}

	Ref<InputEventMouseMotion> mm = p_event;
	if (mm.is_valid()) {
		TouchState *mouse_touch = find_touch_state(-2);
		if (mouse_touch) {
			mouse_touch->position = mm->get_position();
			_on_drag(-2, mm->get_position(), mm->get_relative());

			if (input_mode == INPUT_MODE_EMULATED && mouse_touch->is_primary) {
				_update_emulated_actions();
			}

			if (mouse_touch->is_primary) {
				_on_multitouch_update(active_touches);
			}

			accept_event();
		}
	}
}

// Utility functions
bool VirtualDevice::is_valid_touch_index(int p_index) {
	return p_index >= -2; // -2 is mouse, -1 is no touch, 0+ are touch points
}

void VirtualDevice::_on_touch_down(int p_index, const Vector2 &p_pos) {
	ERR_FAIL_COND(!is_valid_touch_index(p_index));
	// Virtual - base implementation does nothing
}

void VirtualDevice::_on_touch_up(int p_index, const Vector2 &p_pos) {
	ERR_FAIL_COND(!is_valid_touch_index(p_index));
	// Virtual - base implementation does nothing
}

void VirtualDevice::_on_drag(int p_index, const Vector2 &p_pos, const Vector2 &p_relative) {
	ERR_FAIL_COND(!is_valid_touch_index(p_index));
	// Virtual - base implementation does nothing
}

void VirtualDevice::pressed_state_changed() {
	// Virtual hook
}

// Enhanced multitouch core methods implementation
VirtualDevice::TouchState *VirtualDevice::find_touch_state(int p_index) {
	for (int i = 0; i < active_touches.size(); i++) {
		if (active_touches[i].index == p_index && active_touches[i].is_active) {
			return &active_touches.write[i];
		}
	}
	return nullptr;
}

VirtualDevice::TouchState *VirtualDevice::add_touch_state(int p_index, const Vector2 &p_pos) {
	// Check if we've reached maximum touches
	if (active_touches.size() >= max_touches) {
		return nullptr;
	}

	// Check if touch already exists
	if (find_touch_state(p_index)) {
		return nullptr;
	}

	// Create new touch state
	TouchState new_touch;
	new_touch.index = p_index;
	new_touch.position = p_pos;
	new_touch.start_position = p_pos;
	new_touch.is_active = true;
	new_touch.is_primary = (active_touches.size() == 0); // First touch is primary
	new_touch.pressure = (p_index == -2) ? 1.0f : 0.5f; // Mouse has full pressure
	new_touch.timestamp = OS::get_singleton()->get_ticks_msec() / 1000.0f;

	active_touches.push_back(new_touch);

	// Notify multitouch begin if this is the first touch
	if (active_touches.size() == 1) {
		_on_multitouch_begin(active_touches);
	}

	return &active_touches.write[active_touches.size() - 1];
}

void VirtualDevice::remove_touch_state(int p_index) {
	for (int i = 0; i < active_touches.size(); i++) {
		if (active_touches[i].index == p_index) {
			active_touches.write[i].is_active = false;
			active_touches.remove_at(i);
			return;
		}
	}
}

void VirtualDevice::update_primary_touch() {
	primary_touch = TouchState(); // Reset

	if (active_touches.size() == 0) {
		return;
	}

	if (priority_to_first_touch) {
		// First active touch becomes primary
		for (const TouchState &touch : active_touches) {
			if (touch.is_active) {
				primary_touch = touch;
				primary_touch.is_primary = true;
				break;
			}
		}
	} else {
		// Last active touch becomes primary
		for (int i = active_touches.size() - 1; i >= 0; i--) {
			if (active_touches[i].is_active) {
				primary_touch = active_touches[i];
				primary_touch.is_primary = true;
				break;
			}
		}
	}
}

void VirtualDevice::cleanup_inactive_touches() {
	for (int i = active_touches.size() - 1; i >= 0; i--) {
		if (!active_touches[i].is_active) {
			active_touches.remove_at(i);
		}
	}
}

// Enhanced multitouch virtual hooks (base implementations)
void VirtualDevice::_on_multitouch_begin(const Vector<TouchState> &p_touches) {
	// Virtual hook - called when first touch begins
}

void VirtualDevice::_on_multitouch_update(const Vector<TouchState> &p_touches) {
	// Virtual hook - called when touch state changes
}

void VirtualDevice::_on_multitouch_end(const Vector<TouchState> &p_touches) {
	// Virtual hook - called when all touches end
}

void VirtualDevice::_on_touch_conflict(int p_existing_index, int p_new_index) {
	// Virtual hook - called when touch conflict occurs
	// Default behavior: prioritize existing touch
}

VirtualDevice::DrawMode VirtualDevice::get_draw_mode() const {
	if (pressed) {
		if (hovering) {
			return DRAW_HOVER_PRESSED;
		}
		return DRAW_PRESSED;
	}
	if (hovering) {
		return DRAW_HOVER;
	}
	return DRAW_NORMAL;
}

void VirtualDevice::set_device(int p_device) {
	device = p_device;
}

int VirtualDevice::get_device() const {
	return device;
}

void VirtualDevice::set_visibility_mode(VisibilityMode p_mode) {
	visibility_mode = p_mode;
	if (visibility_mode == VISIBILITY_TOUCHSCREEN_ONLY && is_inside_tree() && !Engine::get_singleton()->is_editor_hint()) {
		_on_input_type_changed(Input::get_singleton()->get_last_input_type());
	} else if (visibility_mode == VISIBILITY_ALWAYS) {
		show();
	}
}

VirtualDevice::VisibilityMode VirtualDevice::get_visibility_mode() const {
	return visibility_mode;
}

bool VirtualDevice::is_pressed() const {
	return pressed;
}

bool VirtualDevice::is_hovered() const {
	return hovering;
}

int VirtualDevice::get_current_touch_index() const {
	return primary_touch.index;
}

const VirtualDevice::TouchState &VirtualDevice::get_primary_touch() const {
	return primary_touch;
}

const Vector<VirtualDevice::TouchState> &VirtualDevice::get_active_touches() const {
	return active_touches;
}

int VirtualDevice::get_active_touch_count() const {
	return active_touches.size();
}

// Multitouch configuration
void VirtualDevice::set_max_touches(int p_max) {
	max_touches = CLAMP(p_max, 1, 32);
}

int VirtualDevice::get_max_touches() const {
	return max_touches;
}

void VirtualDevice::set_priority_to_first_touch(bool p_priority) {
	priority_to_first_touch = p_priority;
	if (active_touches.size() > 1) {
		update_primary_touch();
	}
}

bool VirtualDevice::get_priority_to_first_touch() const {
	return priority_to_first_touch;
}

// Touch state queries
bool VirtualDevice::is_touch_active(int p_index) const {
	for (const TouchState &touch : active_touches) {
		if (touch.index == p_index && touch.is_active) {
			return true;
		}
	}
	return false;
}

Vector2 VirtualDevice::get_touch_position(int p_index) const {
	for (const TouchState &touch : active_touches) {
		if (touch.index == p_index && touch.is_active) {
			return touch.position;
		}
	}
	return Vector2();
}

Vector2 VirtualDevice::get_touch_start_position(int p_index) const {
	for (const TouchState &touch : active_touches) {
		if (touch.index == p_index && touch.is_active) {
			return touch.start_position;
		}
	}
	return Vector2();
}

float VirtualDevice::get_touch_pressure(int p_index) const {
	for (const TouchState &touch : active_touches) {
		if (touch.index == p_index && touch.is_active) {
			return touch.pressure;
		}
	}
	return 0.0f;
}

float VirtualDevice::get_touch_duration(int p_index) const {
	for (const TouchState &touch : active_touches) {
		if (touch.index == p_index && touch.is_active) {
			float current_time = OS::get_singleton()->get_ticks_msec() / 1000.0f;
			return current_time - touch.timestamp;
		}
	}
	return 0.0f;
}

void VirtualDevice::set_input_mode(InputMode p_mode) {
	input_mode = p_mode;
}

VirtualDevice::InputMode VirtualDevice::get_input_mode() const {
	return input_mode;
}

void VirtualDevice::set_emit_input_events(bool p_emit) {
	emit_input_events = p_emit;
}

bool VirtualDevice::is_emitting_input_events() const {
	return emit_input_events;
}

void VirtualDevice::set_emit_signals(bool p_emit) {
	emit_signals = p_emit;
}

bool VirtualDevice::is_emitting_signals() const {
	return emit_signals;
}

void VirtualDevice::_update_emulated_actions() {
	Vector<StringName> next_actions;
	if (pressed) {
		_get_emulated_actions(next_actions);
	}

	// 1. Find actions that were released (present in current, missing in next)
	for (const StringName &action : current_emulated_actions) {
		if (next_actions.find(action) == -1) {
			_send_emulated_action(action, false);
			_on_emulated_action_released(action);
		}
	}

	// 2. Find actions that were pressed (missing in current, present in next)
	for (const StringName &action : next_actions) {
		if (current_emulated_actions.find(action) == -1) {
			_send_emulated_action(action, true);
			_on_emulated_action_pressed(action);
		}
	}

	current_emulated_actions = next_actions;
}

void VirtualDevice::_send_emulated_action(const StringName &p_action, bool p_pressed) {
	if (emit_input_events) {
		Ref<InputEventAction> ev;
		ev.instantiate();
		ev->set_action(p_action);
		ev->set_pressed(p_pressed);
		ev->set_device(device);
		Input::get_singleton()->parse_input_event(ev);
	}
	if (emit_signals) {
		emit_signal(p_pressed ? SNAME("action_pressed") : SNAME("action_released"), p_action);
	}
}

VirtualDevice::VirtualDevice() {
	set_mouse_filter(MOUSE_FILTER_STOP);
}

void VirtualDevice::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_device", "device"), &VirtualDevice::set_device);
	ClassDB::bind_method(D_METHOD("get_device"), &VirtualDevice::get_device);
	ClassDB::bind_method(D_METHOD("set_visibility_mode", "mode"), &VirtualDevice::set_visibility_mode);
	ClassDB::bind_method(D_METHOD("get_visibility_mode"), &VirtualDevice::get_visibility_mode);
	ClassDB::bind_method(D_METHOD("is_pressed"), &VirtualDevice::is_pressed);
	ClassDB::bind_method(D_METHOD("is_hovered"), &VirtualDevice::is_hovered);
	ClassDB::bind_method(D_METHOD("get_current_touch_index"), &VirtualDevice::get_current_touch_index);
	ClassDB::bind_method(D_METHOD("get_draw_mode"), &VirtualDevice::get_draw_mode);
	ClassDB::bind_method(D_METHOD("_on_input_type_changed", "type"), &VirtualDevice::_on_input_type_changed);

	ClassDB::bind_method(D_METHOD("set_input_mode", "mode"), &VirtualDevice::set_input_mode);
	ClassDB::bind_method(D_METHOD("get_input_mode"), &VirtualDevice::get_input_mode);
	ClassDB::bind_method(D_METHOD("set_emit_input_events", "emit"), &VirtualDevice::set_emit_input_events);
	ClassDB::bind_method(D_METHOD("is_emitting_input_events"), &VirtualDevice::is_emitting_input_events);
	ClassDB::bind_method(D_METHOD("set_emit_signals", "emit"), &VirtualDevice::set_emit_signals);
	ClassDB::bind_method(D_METHOD("is_emitting_signals"), &VirtualDevice::is_emitting_signals);

	ClassDB::bind_method(D_METHOD("get_active_touch_count"), &VirtualDevice::get_active_touch_count);
	ClassDB::bind_method(D_METHOD("get_max_touches"), &VirtualDevice::get_max_touches);
	ClassDB::bind_method(D_METHOD("set_max_touches", "max"), &VirtualDevice::set_max_touches);
	ClassDB::bind_method(D_METHOD("get_priority_to_first_touch"), &VirtualDevice::get_priority_to_first_touch);
	ClassDB::bind_method(D_METHOD("set_priority_to_first_touch", "priority"), &VirtualDevice::set_priority_to_first_touch);

	ClassDB::bind_method(D_METHOD("is_touch_active", "index"), &VirtualDevice::is_touch_active);
	ClassDB::bind_method(D_METHOD("get_touch_position", "index"), &VirtualDevice::get_touch_position);
	ClassDB::bind_method(D_METHOD("get_touch_start_position", "index"), &VirtualDevice::get_touch_start_position);
	ClassDB::bind_method(D_METHOD("get_touch_pressure", "index"), &VirtualDevice::get_touch_pressure);
	ClassDB::bind_method(D_METHOD("get_touch_duration", "index"), &VirtualDevice::get_touch_duration);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "device"), "set_device", "get_device");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "visibility_mode", PROPERTY_HINT_ENUM, "Always,TouchScreen Only"), "set_visibility_mode", "get_visibility_mode");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "input_mode", PROPERTY_HINT_ENUM, "Native,Emulated"), "set_input_mode", "get_input_mode");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "emit_input_events"), "set_emit_input_events", "is_emitting_input_events");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "emit_signals"), "set_emit_signals", "is_emitting_signals");

	ADD_PROPERTY(PropertyInfo(Variant::INT, "max_touches", PROPERTY_HINT_RANGE, "1,32"), "set_max_touches", "get_max_touches");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "priority_to_first_touch"), "set_priority_to_first_touch", "get_priority_to_first_touch");

	ADD_SIGNAL(MethodInfo("action_pressed", PropertyInfo(Variant::STRING_NAME, "action")));
	ADD_SIGNAL(MethodInfo("action_released", PropertyInfo(Variant::STRING_NAME, "action")));

	BIND_ENUM_CONSTANT(VISIBILITY_ALWAYS);
	BIND_ENUM_CONSTANT(VISIBILITY_TOUCHSCREEN_ONLY);
	BIND_ENUM_CONSTANT(DRAW_NORMAL);
	BIND_ENUM_CONSTANT(DRAW_PRESSED);
	BIND_ENUM_CONSTANT(DRAW_HOVER);
	BIND_ENUM_CONSTANT(DRAW_HOVER_PRESSED);
	BIND_ENUM_CONSTANT(INPUT_MODE_NATIVE);
	BIND_ENUM_CONSTANT(INPUT_MODE_EMULATED);
}

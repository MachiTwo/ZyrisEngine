/**************************************************************************/
/*  virtual_joystick.cpp                                                  */
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

#include "virtual_joystick.h"

#include "core/config/engine.h"
#include "core/input/input.h"
#include "scene/resources/style_box.h"
#include "scene/resources/texture.h"
#include "scene/theme/theme_db.h"

void VirtualJoystick::_notification(int p_what) {
	VirtualDevice::_notification(p_what);
	switch (p_what) {
		case NOTIFICATION_READY: {
			original_base_pos = get_size() / 2.0;
			_reset_joystick();
		} break;
		case NOTIFICATION_DRAW: {
			_draw_joystick();
		} break;
		case NOTIFICATION_THEME_CHANGED: {
			_update_theme_item_cache();
			queue_redraw();
		} break;
		case NOTIFICATION_RESIZED: {
			if (joystick_mode == JOYSTICK_MODE_FIXED) {
				Size2 s = get_size();
				if (Math::abs(s.width - s.height) > 0.001f) {
					float side = MIN(s.width, s.height);
					set_size(Size2(side, side));
				}
			}
			if (!is_pressed()) {
				original_base_pos = get_size() / 2.0;
				_reset_joystick();
			}
			queue_redraw();
		} break;
	}
}

float VirtualJoystick::get_joystick_radius() const {
	Size2 s = get_size();
	return MIN(s.x, s.y) / 2.0f;
}

void VirtualJoystick::_draw_joystick() {
	if (joystick_mode == JOYSTICK_MODE_DYNAMIC && !is_pressed() && !Engine::get_singleton()->is_editor_hint()) {
		return;
	}

	Size2 s = get_size();
	float radius = get_joystick_radius();

	Vector2 drawing_base = (joystick_mode == JOYSTICK_MODE_FIXED) ? (s / 2.0) : base_pos;
	Vector2 drawing_tip = (joystick_mode == JOYSTICK_MODE_FIXED && !is_pressed()) ? drawing_base : tip_pos;

	Rect2 base_rect = Rect2(drawing_base - Vector2(radius, radius), Vector2(radius * 2, radius * 2));

	Ref<Texture2D> b_tex = base_texture;
	if (b_tex.is_valid()) {
		Color b_col = base_color * theme_cache.base_color_theme;
		draw_texture_rect(b_tex, base_rect, false, b_col);
	} else if (theme_cache.base_style.is_valid()) {
		theme_cache.base_style->draw(get_canvas_item(), base_rect);
	}

	float t_radius = radius * tip_scale;
	Rect2 tip_rect = Rect2(drawing_tip - Vector2(t_radius, t_radius), Vector2(t_radius * 2, t_radius * 2));

	Ref<Texture2D> t_tex = tip_texture;
	if (t_tex.is_valid()) {
		Color t_col = tip_color * theme_cache.tip_color_theme;
		draw_texture_rect(t_tex, tip_rect, false, t_col);
	} else if (theme_cache.tip_style.is_valid()) {
		theme_cache.tip_style->draw(get_canvas_item(), tip_rect);
	}
}

void VirtualJoystick::_on_touch_down(int p_index, const Vector2 &p_pos) {
	if (joystick_mode == JOYSTICK_MODE_DYNAMIC) {
		base_pos = p_pos;
		tip_pos = p_pos;
	} else {
		tip_pos = p_pos;
	}
	_update_input_vector();
	queue_redraw();
}

void VirtualJoystick::_on_touch_up(int p_index, const Vector2 &p_pos) {
	_reset_joystick();
	queue_redraw();
}

void VirtualJoystick::_on_drag(int p_index, const Vector2 &p_pos, const Vector2 &p_relative) {
	tip_pos = p_pos;

	_update_input_vector();
	queue_redraw();
}

void VirtualJoystick::_update_input_vector() {
	Size2 s = get_size();

	Vector2 current_base = (joystick_mode == JOYSTICK_MODE_FIXED) ? (s / 2.0) : base_pos;
	Vector2 diff = tip_pos - current_base;
	float dist = diff.length();

	float radius = get_joystick_radius();
	float effective_clamp_zone = radius * clamp_zone_size;

	if (dist <= deadzone_size) {
		input_vector = Vector2();
	} else {
		// Clamp visual tip
		if (dist > effective_clamp_zone) {
			diff = diff.normalized() * effective_clamp_zone;
			tip_pos = current_base + diff;
		}

		input_vector = diff / effective_clamp_zone;
	}

	if (input_vector.is_equal_approx(last_sent_vector)) {
		return;
	}
	last_sent_vector = input_vector;

	int axis_x = (joystick_hand == JOYSTICK_HAND_LEFT) ? 0 : 2;
	int axis_y = (joystick_hand == JOYSTICK_HAND_LEFT) ? 1 : 3;

	Ref<InputEventVirtualMotion> ie_x;
	ie_x.instantiate();
	ie_x->set_device(get_device());
	ie_x->set_axis(axis_x);
	ie_x->set_axis_value(input_vector.x);
	Input *input = Input::get_singleton();
	if (input && get_input_mode() == INPUT_MODE_NATIVE) {
		input->parse_input_event(ie_x);
	}

	Ref<InputEventVirtualMotion> ie_y;
	ie_y.instantiate();
	ie_y->set_device(get_device());
	ie_y->set_axis(axis_y);
	ie_y->set_axis_value(input_vector.y);
	if (input && get_input_mode() == INPUT_MODE_NATIVE) {
		input->parse_input_event(ie_y);
	}
}

void VirtualJoystick::_reset_joystick() {
	input_vector = Vector2();
	last_sent_vector = Vector2();
	base_pos = original_base_pos;
	tip_pos = original_base_pos;

	int axis_x = (joystick_hand == JOYSTICK_HAND_LEFT) ? 0 : 2;
	int axis_y = (joystick_hand == JOYSTICK_HAND_LEFT) ? 1 : 3;

	Ref<InputEventVirtualMotion> ie_x;
	ie_x.instantiate();
	ie_x->set_device(get_device());
	ie_x->set_axis(axis_x);
	ie_x->set_axis_value(0.0);
	Input *input = Input::get_singleton();
	if (input) {
		input->parse_input_event(ie_x);
	}

	Ref<InputEventVirtualMotion> ie_y;
	ie_y.instantiate();
	ie_y->set_device(get_device());
	ie_y->set_axis(axis_y);
	ie_y->set_axis_value(0.0);
	if (input && get_input_mode() == INPUT_MODE_NATIVE) {
		input->parse_input_event(ie_y);
	}
}

#ifdef TOOLS_ENABLED
int VirtualJoystick::_edit_get_handle_count() const {
	return 3;
}

String VirtualJoystick::_edit_get_handle_name(int p_idx) const {
	switch (p_idx) {
		case 0:
			return "Deadzone";
		case 1:
			return "Clamp Zone";
	}
	return "";
}

Variant VirtualJoystick::_edit_get_handle_value(int p_idx) const {
	Vector2 center = get_size() / 2.0;

	switch (p_idx) {
		case 0:
			return center + Vector2(deadzone_size, 0);
		case 1:
			return center + Vector2(clamp_zone_size, 0);
	}
	return Variant();
}

void VirtualJoystick::_edit_set_handle(int p_idx, const Variant &p_value) {
	Vector2 center = get_size() / 2.0;
	Vector2 pos = p_value;
	float val = (pos - center).x;

	switch (p_idx) {
		case 0:
			set_deadzone_size(MAX(0.0, val));
			break;
		case 1:
			set_clamp_zone_size(MAX(1.0, val));
			break;
	}
}
#endif

void VirtualJoystick::pressed_state_changed() {
	if (!is_pressed()) {
		_reset_joystick();
	}
}

void VirtualJoystick::set_deadzone_size(float p_size) {
	deadzone_size = p_size;
	queue_redraw();
}

float VirtualJoystick::get_deadzone_size() const {
	return deadzone_size;
}

void VirtualJoystick::set_clamp_zone_size(float p_size) {
	clamp_zone_size = p_size;
	queue_redraw();
}

float VirtualJoystick::get_clamp_zone_size() const {
	return clamp_zone_size;
}

void VirtualJoystick::set_tip_scale(float p_scale) {
	tip_scale = p_scale;
	queue_redraw();
}

float VirtualJoystick::get_tip_scale() const {
	return tip_scale;
}

Vector2 VirtualJoystick::get_output() const {
	return input_vector;
}

Size2 VirtualJoystick::get_minimum_size() const {
	if (joystick_mode == JOYSTICK_MODE_FIXED) {
		return Size2(100, 100); // Default min size
	}
	Size2 ms = VirtualDevice::get_minimum_size();
	return MAX(ms, Size2(20, 20));
}

void VirtualJoystick::set_joystick_mode(JoystickMode p_mode) {
	joystick_mode = p_mode;
	queue_redraw();
}

void VirtualJoystick::set_joystick_hand(JoystickHand p_hand) {
	joystick_hand = p_hand;
}

void VirtualJoystick::set_base_texture(const Ref<Texture2D> &p_texture) {
	base_texture = p_texture;
	queue_redraw();
}

Ref<Texture2D> VirtualJoystick::get_base_texture() const {
	return base_texture;
}

void VirtualJoystick::set_tip_texture(const Ref<Texture2D> &p_texture) {
	tip_texture = p_texture;
	queue_redraw();
}

Ref<Texture2D> VirtualJoystick::get_tip_texture() const {
	return tip_texture;
}

void VirtualJoystick::set_base_color(const Color &p_color) {
	base_color = p_color;
	queue_redraw();
}

Color VirtualJoystick::get_base_color() const {
	return base_color;
}

void VirtualJoystick::set_tip_color(const Color &p_color) {
	tip_color = p_color;
	queue_redraw();
}

Color VirtualJoystick::get_tip_color() const {
	return tip_color;
}
void VirtualJoystick::set_action(int p_direction, const StringName &p_action) {
	switch (p_direction) {
		case DIR_UP:
			action_up = p_action;
			break;
		case DIR_DOWN:
			action_down = p_action;
			break;
		case DIR_LEFT:
			action_left = p_action;
			break;
		case DIR_RIGHT:
			action_right = p_action;
			break;
		default:
			break;
	}
}

StringName VirtualJoystick::get_action(int p_direction) const {
	switch (p_direction) {
		case DIR_UP:
			return action_up;
		case DIR_DOWN:
			return action_down;
		case DIR_LEFT:
			return action_left;
		case DIR_RIGHT:
			return action_right;
		default:
			return StringName();
	}
}
void VirtualJoystick::_update_theme_item_cache() {
	VirtualDevice::_update_theme_item_cache();

	theme_cache.base_style = get_theme_stylebox(SNAME("base_style"), SNAME("VirtualJoystick"));
	theme_cache.tip_style = get_theme_stylebox(SNAME("tip_style"), SNAME("VirtualJoystick"));

	theme_cache.base_texture_theme = get_theme_icon(SNAME("base"), SNAME("VirtualJoystick"));
	theme_cache.tip_texture_theme = get_theme_icon(SNAME("tip"), SNAME("VirtualJoystick"));

	theme_cache.base_color_theme = get_theme_color(SNAME("base_color"), SNAME("VirtualJoystick"));
	theme_cache.tip_color_theme = get_theme_color(SNAME("tip_color"), SNAME("VirtualJoystick"));
}

VirtualJoystick::VirtualJoystick() {
	action_up = "ui_up";
	action_down = "ui_down";
	action_left = "ui_left";
	action_right = "ui_right";

	set_mouse_filter(MOUSE_FILTER_PASS); // Let users click
}

void VirtualJoystick::_bind_methods() {
#ifdef TOOLS_ENABLED
	ClassDB::bind_method(D_METHOD("_edit_get_handle_count"), &VirtualJoystick::_edit_get_handle_count);
	ClassDB::bind_method(D_METHOD("_edit_get_handle_name", "idx"), &VirtualJoystick::_edit_get_handle_name);
	ClassDB::bind_method(D_METHOD("_edit_get_handle_value", "idx"), &VirtualJoystick::_edit_get_handle_value);
	ClassDB::bind_method(D_METHOD("_edit_set_handle", "idx", "value"), &VirtualJoystick::_edit_set_handle);
#endif
	ClassDB::bind_method(D_METHOD("set_deadzone_size", "deadzone_size"), &VirtualJoystick::set_deadzone_size);
	ClassDB::bind_method(D_METHOD("get_deadzone_size"), &VirtualJoystick::get_deadzone_size);

	ClassDB::bind_method(D_METHOD("set_clamp_zone_size", "size"), &VirtualJoystick::set_clamp_zone_size);
	ClassDB::bind_method(D_METHOD("get_clamp_zone_size"), &VirtualJoystick::get_clamp_zone_size);

	ClassDB::bind_method(D_METHOD("get_output"), &VirtualJoystick::get_output);

	ClassDB::bind_method(D_METHOD("set_joystick_mode", "mode"), &VirtualJoystick::set_joystick_mode);
	ClassDB::bind_method(D_METHOD("get_joystick_mode"), &VirtualJoystick::get_joystick_mode);

	ClassDB::bind_method(D_METHOD("set_joystick_hand", "hand"), &VirtualJoystick::set_joystick_hand);
	ClassDB::bind_method(D_METHOD("get_joystick_hand"), &VirtualJoystick::get_joystick_hand);

	ClassDB::bind_method(D_METHOD("set_base_texture", "texture"), &VirtualJoystick::set_base_texture);
	ClassDB::bind_method(D_METHOD("get_base_texture"), &VirtualJoystick::get_base_texture);

	ClassDB::bind_method(D_METHOD("set_tip_texture", "texture"), &VirtualJoystick::set_tip_texture);
	ClassDB::bind_method(D_METHOD("get_tip_texture"), &VirtualJoystick::get_tip_texture);

	ClassDB::bind_method(D_METHOD("set_base_color", "color"), &VirtualJoystick::set_base_color);
	ClassDB::bind_method(D_METHOD("get_base_color"), &VirtualJoystick::get_base_color);

	ClassDB::bind_method(D_METHOD("set_tip_color", "color"), &VirtualJoystick::set_tip_color);
	ClassDB::bind_method(D_METHOD("get_tip_color"), &VirtualJoystick::get_tip_color);

	ClassDB::bind_method(D_METHOD("set_tip_scale", "scale"), &VirtualJoystick::set_tip_scale);
	ClassDB::bind_method(D_METHOD("get_tip_scale"), &VirtualJoystick::get_tip_scale);

	ClassDB::bind_method(D_METHOD("set_action", "direction", "action"), &VirtualJoystick::set_action);
	ClassDB::bind_method(D_METHOD("get_action", "direction"), &VirtualJoystick::get_action);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "joystick_hand", PROPERTY_HINT_ENUM, "Left,Right"), "set_joystick_hand", "get_joystick_hand");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "tip_scale", PROPERTY_HINT_RANGE, "0.01,1.0,0.01"), "set_tip_scale", "get_tip_scale");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "deadzone_size", PROPERTY_HINT_RANGE, "0,0.15,0.001"), "set_deadzone_size", "get_deadzone_size");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "clamp_zone_size", PROPERTY_HINT_RANGE, "0.5,2.0,0.01"), "set_clamp_zone_size", "get_clamp_zone_size");

	ADD_GROUP("Emulated Mode", "action_");
	ADD_PROPERTYI(PropertyInfo(Variant::STRING_NAME, "action_up", PROPERTY_HINT_INPUT_NAME, "show_builtin,loose_mode"), "set_action", "get_action", DIR_UP);
	ADD_PROPERTYI(PropertyInfo(Variant::STRING_NAME, "action_down", PROPERTY_HINT_INPUT_NAME, "show_builtin,loose_mode"), "set_action", "get_action", DIR_DOWN);
	ADD_PROPERTYI(PropertyInfo(Variant::STRING_NAME, "action_left", PROPERTY_HINT_INPUT_NAME, "show_builtin,loose_mode"), "set_action", "get_action", DIR_LEFT);
	ADD_PROPERTYI(PropertyInfo(Variant::STRING_NAME, "action_right", PROPERTY_HINT_INPUT_NAME, "show_builtin,loose_mode"), "set_action", "get_action", DIR_RIGHT);

	ADD_GROUP("Visuals", "");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "base_texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "set_base_texture", "get_base_texture");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "tip_texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "set_tip_texture", "get_tip_texture");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "base_color"), "set_base_color", "get_base_color");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "tip_color"), "set_tip_color", "get_tip_color");

	BIND_ENUM_CONSTANT(JOYSTICK_MODE_FIXED);
	BIND_ENUM_CONSTANT(JOYSTICK_MODE_DYNAMIC);

	BIND_ENUM_CONSTANT(JOYSTICK_HAND_LEFT);
	BIND_ENUM_CONSTANT(JOYSTICK_HAND_RIGHT);

	BIND_THEME_ITEM(Theme::DATA_TYPE_STYLEBOX, VirtualJoystick, base_style);
	BIND_THEME_ITEM(Theme::DATA_TYPE_STYLEBOX, VirtualJoystick, tip_style);
	BIND_THEME_ITEM_CUSTOM(Theme::DATA_TYPE_ICON, VirtualJoystick, base_texture_theme, "base");
	BIND_THEME_ITEM_CUSTOM(Theme::DATA_TYPE_ICON, VirtualJoystick, tip_texture_theme, "tip");
	BIND_THEME_ITEM_CUSTOM(Theme::DATA_TYPE_COLOR, VirtualJoystick, base_color_theme, "base_color");
	BIND_THEME_ITEM_CUSTOM(Theme::DATA_TYPE_COLOR, VirtualJoystick, tip_color_theme, "tip_color");
}

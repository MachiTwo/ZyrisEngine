/**************************************************************************/
/*  virtual_dpad.cpp                                                      */
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

#include "virtual_dpad.h"

#include "core/input/input.h"
#include "scene/resources/style_box_flat.h"
#include "scene/theme/theme_db.h"

void VirtualDPad::_update_theme_item_cache() {
	VirtualDevice::_update_theme_item_cache();

	theme_cache.normal_style = get_theme_stylebox(SNAME("normal"), SNAME("VirtualDPad"));
	theme_cache.pressed_style = get_theme_stylebox(SNAME("pressed"), SNAME("VirtualDPad"));
	theme_cache.center_style = get_theme_stylebox(SNAME("center"), SNAME("VirtualDPad"));

	theme_cache.dpad_texture_theme = get_theme_icon(SNAME("texture"), SNAME("VirtualDPad"));
	theme_cache.base_color = get_theme_color(SNAME("base_color"), SNAME("VirtualDPad"));
	theme_cache.border_color = get_theme_color(SNAME("border_color"), SNAME("VirtualDPad"));
	theme_cache.border_width = get_theme_constant(SNAME("border_width"), SNAME("VirtualDPad"));

	auto prepare_style = [&](Ref<StyleBox> p_src) -> Ref<StyleBox> {
		if (p_src.is_null()) {
			return p_src;
		}
		Ref<StyleBoxFlat> flat = p_src;
		if (flat.is_valid() && theme_cache.border_width > 0) {
			flat = flat->duplicate();
			ERR_FAIL_COND_V_MSG(flat.is_null(), p_src, "Failed to duplicate StyleBoxFlat");
			flat->set_border_width_all(theme_cache.border_width);
			flat->set_border_color(theme_cache.border_color);
			return flat;
		}
		return p_src;
	};

	theme_cache.effective_normal_style = prepare_style(theme_cache.normal_style);
	theme_cache.effective_pressed_style = prepare_style(theme_cache.pressed_style);
	theme_cache.effective_center_style = prepare_style(theme_cache.center_style);
}

void VirtualDPad::_notification(int p_what) {
	VirtualDevice::_notification(p_what);
	switch (p_what) {
		case NOTIFICATION_DRAW: {
			Ref<Texture2D> tex = texture;
			if (tex.is_null() && has_theme_icon(SNAME("texture"), SNAME("VirtualDPad"))) {
				tex = theme_cache.dpad_texture_theme;
			}

			if (tex.is_valid()) {
				draw_texture_rect(tex, Rect2(Point2(), get_size()), false, theme_cache.base_color);
				return;
			}

			// Procedural DPad with Independent Arms
			Size2 s = get_size();
			Vector2 c = s / 2.0;
			float radius = MIN(s.width, s.height) / 2.0;
			float thickness = radius * 2.0 * button_width_ratio;

			// Define Rects
			Rect2 rect_center(c.x - thickness / 2.0, c.y - thickness / 2.0, thickness, thickness);
			Rect2 rect_up(c.x - thickness / 2.0, c.y - radius, thickness, radius - thickness / 2.0);
			Rect2 rect_down(c.x - thickness / 2.0, c.y + thickness / 2.0, thickness, radius - thickness / 2.0);
			Rect2 rect_left(c.x - radius, c.y - thickness / 2.0, radius - thickness / 2.0, thickness);
			Rect2 rect_right(c.x + thickness / 2.0, c.y - thickness / 2.0, radius - thickness / 2.0, thickness);

			// Prepare styles (use Theme or Generate Fallback)
			Ref<StyleBox> normal_style = theme_cache.effective_normal_style;
			Ref<StyleBox> pressed_style = theme_cache.effective_pressed_style;
			Ref<StyleBox> center_style = theme_cache.effective_center_style;

			// Fallback generation if no style is provided
			if (normal_style.is_null()) {
				Ref<StyleBoxFlat> f;
				f.instantiate();
				f->set_bg_color(theme_cache.base_color);
				f->set_corner_radius_all(2); // Slight rounding for polish
				if (theme_cache.border_width > 0) {
					f->set_border_width_all(theme_cache.border_width);
					f->set_border_color(theme_cache.border_color);
				}
				normal_style = f;
			}

			if (pressed_style.is_null()) {
				Ref<StyleBoxFlat> f;
				f.instantiate();
				// Darken or lighten based on base color for feedback
				Color bg_color = theme_cache.base_color;
				bg_color.a = CLAMP(bg_color.a * 0.8, 0.0, 1.0); // Simple interaction feedback
				if (bg_color.r > 0.5 && bg_color.g > 0.5 && bg_color.b > 0.5) {
					bg_color = bg_color.darkened(0.2);
				} else {
					bg_color = bg_color.lightened(0.2);
				}

				f->set_bg_color(bg_color);
				f->set_corner_radius_all(2);
				if (theme_cache.border_width > 0) {
					f->set_border_width_all(theme_cache.border_width);
					f->set_border_color(theme_cache.border_color);
				}
				pressed_style = f;
			}

			if (center_style.is_null()) {
				center_style = normal_style;
			}

			RID ci = get_canvas_item();

			// Draw Center
			center_style->draw(ci, rect_center);

			// Draw Arms
			(pressed_up ? pressed_style : normal_style)->draw(ci, rect_up);
			(pressed_down ? pressed_style : normal_style)->draw(ci, rect_down);
			(pressed_left ? pressed_style : normal_style)->draw(ci, rect_left);
			(pressed_right ? pressed_style : normal_style)->draw(ci, rect_right);
		} break;
		case NOTIFICATION_RESIZED: {
			Size2 s = get_size();
			if (Math::abs(s.width - s.height) > 0.001f) {
				float side = MIN(s.width, s.height);
				set_size(Size2(side, side));
			}
			queue_redraw();
		} break;
		case NOTIFICATION_THEME_CHANGED: {
			_update_theme_item_cache();
			queue_redraw();
		} break;
	}
}

// ... (methods) ...
void VirtualDPad::_on_touch_down(int p_index, const Vector2 &p_pos) {
	touches[p_index] = p_pos;
	_update_dpad();
}

void VirtualDPad::_on_touch_up(int p_index, const Vector2 &p_pos) {
	touches.erase(p_index);
	_update_dpad();
}

void VirtualDPad::pressed_state_changed() {
	if (!is_pressed()) {
		touches.clear();
		_update_dpad();
	}
}

void VirtualDPad::_on_drag(int p_index, const Vector2 &p_pos, const Vector2 &p_relative) {
	touches[p_index] = p_pos;
	_update_dpad();
}

void VirtualDPad::_update_dpad() {
	// Store previous state
	bool prev_up = pressed_up;
	bool prev_down = pressed_down;
	bool prev_left = pressed_left;
	bool prev_right = pressed_right;

	// Reset current state locally (will be re-calculated)
	bool next_up = false;
	bool next_down = false;
	bool next_left = false;
	bool next_right = false;

	float radius = MIN(get_size().x, get_size().y) / 2.0;
	float half_thickness = radius * button_width_ratio;
	float final_deadzone = MAX(deadzone_size, half_thickness);
	Vector2 center = get_size() / 2.0;
	float diagonal_threshold = Math::sin(Math::PI / 8.0); // ~0.38
	// Calculate 8-way directional state based on active touches
	for (const KeyValue<int, Vector2> &E : touches) {
		Vector2 diff = E.value - center;

		if (Math::abs(diff.x) > final_deadzone || Math::abs(diff.y) > final_deadzone) {
			Vector2 dir = diff.normalized();

			if (dir.y < -diagonal_threshold) {
				next_up = true;
			}
			if (dir.y > diagonal_threshold) {
				next_down = true;
			}
			if (dir.x < -diagonal_threshold) {
				next_left = true;
			}
			if (dir.x > diagonal_threshold) {
				next_right = true;
			}
		}
	}

	// SOCD Neutral: Cancel opposing directions
	if (next_up && next_down) {
		next_up = false;
		next_down = false;
	}
	if (next_left && next_right) {
		next_left = false;
		next_right = false;
	}

	// Apply State
	pressed_up = next_up;
	pressed_down = next_down;
	pressed_left = next_left;
	pressed_right = next_right;

	bool changed = false;
	if (pressed_up != prev_up) {
		_press_direction(DIR_UP, pressed_up);
		changed = true;
	}
	if (pressed_down != prev_down) {
		_press_direction(DIR_DOWN, pressed_down);
		changed = true;
	}
	if (pressed_left != prev_left) {
		_press_direction(DIR_LEFT, pressed_left);
		changed = true;
	}
	if (pressed_right != prev_right) {
		_press_direction(DIR_RIGHT, pressed_right);
		changed = true;
	}

	if (changed) {
		queue_redraw();
	}
}

void VirtualDPad::_press_direction(DPadDirection p_dir, bool p_pressed) {
	int btn = -1;
	switch (p_dir) {
		case DIR_UP:
			btn = up_button_index;
			break;
		case DIR_DOWN:
			btn = down_button_index;
			break;
		case DIR_LEFT:
			btn = left_button_index;
			break;
		case DIR_RIGHT:
			btn = right_button_index;
			break;
		default:
			break;
	}

	if (btn != -1) {
		Ref<InputEventVirtualButton> ie;
		ie.instantiate();
		ie->set_device(get_device());
		ie->set_button_index(btn);
		ie->set_pressed(p_pressed);
		Input *input = Input::get_singleton();
		if (input && get_input_mode() == INPUT_MODE_NATIVE) {
			input->parse_input_event(ie);
		}
	}
}

void VirtualDPad::set_button_width_ratio(float p_ratio) {
	button_width_ratio = CLAMP(p_ratio, 0.1, 1.0);
	queue_redraw();
}

float VirtualDPad::get_button_width_ratio() const {
	return button_width_ratio;
}

void VirtualDPad::set_texture(const Ref<Texture2D> &p_texture) {
	texture = p_texture;
	update_minimum_size();
	queue_redraw();
}

Ref<Texture2D> VirtualDPad::get_texture() const {
	return texture;
}

void VirtualDPad::set_deadzone_size(float p_size) {
	deadzone_size = p_size;
	queue_redraw();
}

float VirtualDPad::get_deadzone_size() const {
	return deadzone_size;
}

void VirtualDPad::set_up_button_index(int p_index) {
	up_button_index = p_index;
}

int VirtualDPad::get_up_button_index() const {
	return up_button_index;
}

void VirtualDPad::set_down_button_index(int p_index) {
	down_button_index = p_index;
}

int VirtualDPad::get_down_button_index() const {
	return down_button_index;
}

void VirtualDPad::set_left_button_index(int p_index) {
	left_button_index = p_index;
}

int VirtualDPad::get_left_button_index() const {
	return left_button_index;
}

void VirtualDPad::set_right_button_index(int p_index) {
	right_button_index = p_index;
}

int VirtualDPad::get_right_button_index() const {
	return right_button_index;
}

void VirtualDPad::set_action(int p_direction, const StringName &p_action) {
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

StringName VirtualDPad::get_action(int p_direction) const {
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

Size2 VirtualDPad::get_minimum_size() const {
	Size2 ms = VirtualDevice::get_minimum_size();
	float s = MAX(MAX(ms.width, ms.height), 20.0f);
	return Size2(s, s);
}

VirtualDPad::VirtualDPad() {
	up_button_index = 12;
	down_button_index = 13;
	left_button_index = 14;
	right_button_index = 15;

	action_up = "ui_up";
	action_down = "ui_down";
	action_left = "ui_left";
	action_right = "ui_right";
}

void VirtualDPad::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_texture", "texture"), &VirtualDPad::set_texture);
	ClassDB::bind_method(D_METHOD("get_texture"), &VirtualDPad::get_texture);
	ClassDB::bind_method(D_METHOD("set_deadzone_size", "size"), &VirtualDPad::set_deadzone_size);
	ClassDB::bind_method(D_METHOD("get_deadzone_size"), &VirtualDPad::get_deadzone_size);
	ClassDB::bind_method(D_METHOD("set_button_width_ratio", "ratio"), &VirtualDPad::set_button_width_ratio);
	ClassDB::bind_method(D_METHOD("get_button_width_ratio"), &VirtualDPad::get_button_width_ratio);

	ClassDB::bind_method(D_METHOD("set_up_button_index", "index"), &VirtualDPad::set_up_button_index);
	ClassDB::bind_method(D_METHOD("get_up_button_index"), &VirtualDPad::get_up_button_index);
	ClassDB::bind_method(D_METHOD("set_down_button_index", "index"), &VirtualDPad::set_down_button_index);
	ClassDB::bind_method(D_METHOD("get_down_button_index"), &VirtualDPad::get_down_button_index);
	ClassDB::bind_method(D_METHOD("set_left_button_index", "index"), &VirtualDPad::set_left_button_index);
	ClassDB::bind_method(D_METHOD("get_left_button_index"), &VirtualDPad::get_left_button_index);
	ClassDB::bind_method(D_METHOD("set_right_button_index", "index"), &VirtualDPad::set_right_button_index);
	ClassDB::bind_method(D_METHOD("get_right_button_index"), &VirtualDPad::get_right_button_index);

	ClassDB::bind_method(D_METHOD("set_action", "direction", "action"), &VirtualDPad::set_action);
	ClassDB::bind_method(D_METHOD("get_action", "direction"), &VirtualDPad::get_action);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "set_texture", "get_texture");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "deadzone_size"), "set_deadzone_size", "get_deadzone_size");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "button_width_ratio", PROPERTY_HINT_RANGE, "0.1,1.0,0.01"), "set_button_width_ratio", "get_button_width_ratio");

	ADD_GROUP("Button Indices", "");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "up_index"), "set_up_button_index", "get_up_button_index");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "down_index"), "set_down_button_index", "get_down_button_index");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "left_index"), "set_left_button_index", "get_left_button_index");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "right_index"), "set_right_button_index", "get_right_button_index");

	ADD_GROUP("Emulated Mode", "action_");
	ADD_PROPERTYI(PropertyInfo(Variant::STRING_NAME, "action_up", PROPERTY_HINT_INPUT_NAME, "show_builtin,loose_mode"), "set_action", "get_action", DIR_UP);
	ADD_PROPERTYI(PropertyInfo(Variant::STRING_NAME, "action_down", PROPERTY_HINT_INPUT_NAME, "show_builtin,loose_mode"), "set_action", "get_action", DIR_DOWN);
	ADD_PROPERTYI(PropertyInfo(Variant::STRING_NAME, "action_left", PROPERTY_HINT_INPUT_NAME, "show_builtin,loose_mode"), "set_action", "get_action", DIR_LEFT);
	ADD_PROPERTYI(PropertyInfo(Variant::STRING_NAME, "action_right", PROPERTY_HINT_INPUT_NAME, "show_builtin,loose_mode"), "set_action", "get_action", DIR_RIGHT);

	BIND_THEME_ITEM(Theme::DATA_TYPE_STYLEBOX, VirtualDPad, normal_style);
	BIND_THEME_ITEM(Theme::DATA_TYPE_STYLEBOX, VirtualDPad, pressed_style);
	BIND_THEME_ITEM(Theme::DATA_TYPE_STYLEBOX, VirtualDPad, center_style);
	BIND_THEME_ITEM_CUSTOM(Theme::DATA_TYPE_ICON, VirtualDPad, dpad_texture_theme, "texture");
	BIND_THEME_ITEM_CUSTOM(Theme::DATA_TYPE_COLOR, VirtualDPad, base_color, "base_color");
	BIND_THEME_ITEM_CUSTOM(Theme::DATA_TYPE_COLOR, VirtualDPad, border_color, "border_color");
	BIND_THEME_ITEM_CUSTOM(Theme::DATA_TYPE_CONSTANT, VirtualDPad, border_width, "border_width");
}

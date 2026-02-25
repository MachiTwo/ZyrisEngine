/**************************************************************************/
/*  virtual_touch_pad.cpp                                                 */
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

#include "virtual_touch_pad.h"

#include "core/config/engine.h"
#include "core/input/input.h"
#include "scene/2d/cpu_particles_2d.h"
#include "scene/2d/gpu_particles_2d.h"
#include "scene/theme/theme_db.h"

void VirtualTouchPad::_update_theme_item_cache() {
	VirtualDevice::_update_theme_item_cache();

	theme_cache.style_panel = get_theme_stylebox(SNAME("panel"), SNAME("VirtualTouchPad"));
	theme_cache.trace_color = get_theme_color(SNAME("trace_color"), SNAME("VirtualTouchPad"));
	if (has_theme_icon(SNAME("trace_texture"), SNAME("VirtualTouchPad"))) {
		theme_cache.trace_texture = get_theme_icon(SNAME("trace_texture"), SNAME("VirtualTouchPad"));
	} else {
		theme_cache.trace_texture = Ref<Texture2D>();
	}

	theme_cache.trace_width = get_theme_constant(SNAME("trace_width"), SNAME("VirtualTouchPad"));
}

void VirtualTouchPad::_notification(int p_what) {
	VirtualDevice::_notification(p_what);
	switch (p_what) {
		case NOTIFICATION_INTERNAL_PROCESS: {
			if (is_pressed()) {
				Vector2 deflection;
				float d = get_process_delta_time();
				if (d > 0) {
					Size2 viewport_size = get_viewport_rect().size;
					float ref_side = MIN(viewport_size.x, viewport_size.y);
					if (ref_side > 0) {
						Vector2 vel = accumulated_relative / (ref_side * d);
						deflection = vel * sensitivity;
					}
				}

				deflection.x = CLAMP(deflection.x, -1.0, 1.0);
				deflection.y = CLAMP(deflection.y, -1.0, 1.0);

				if (deflection != last_sent_deflection || deflection != Vector2()) {
					_send_axis_events(deflection);
					last_sent_deflection = deflection;
				}

				accumulated_relative = Vector2();
			}

			if (mode == MODE_TRACE && trace_points.size() > 0) {
				float delta = get_process_delta_time();
				fade_accumulator += delta;
				float interval = fade_duration / (float)MAX(1, trace_length);
				while (fade_accumulator >= interval && trace_points.size() > 0) {
					trace_points.remove_at(0);
					fade_accumulator -= interval;
					queue_redraw();
				}
				if (trace_points.is_empty()) {
					fade_accumulator = 0.0f;
				}
			}
		} break;
		case NOTIFICATION_DRAW: {
			if (Engine::get_singleton()->is_editor_hint()) {
				if (theme_cache.style_panel.is_valid()) {
					theme_cache.style_panel->draw(get_canvas_item(), Rect2(Point2(), get_size()));
				} else {
					draw_rect(Rect2(Point2(), get_size()), Color(0.5, 0.5, 0.5, 0.2), true);
					draw_rect(Rect2(Point2(), get_size()), Color(1, 1, 1, 0.5), false);
				}
			}

			if (mode == MODE_CIRCLE) {
				if (is_pressed()) {
					draw_arc(current_pos, circle_radius, 0, Math::TAU, 32, circle_color, circle_width, true);
				}
			} else if (mode == MODE_TRACE) {
				if (theme_cache.trace_texture.is_valid()) {
					if (is_pressed()) {
						Vector2 size = theme_cache.trace_texture->get_size();
						draw_texture_rect(theme_cache.trace_texture, Rect2(current_pos - size / 2, size), false, theme_cache.trace_color);
					}
				} else if (trace_points.size() >= 2) {
					Color col = (base_color.a == 0) ? theme_cache.trace_color : base_color;
					float width = (base_width <= 0) ? (float)theme_cache.trace_width : base_width;

					int point_count = trace_points.size();

					if (trace_style == TRACE_STYLE_SOLID) {
						// Use polygon generation for smooth continuous transparency and variable width
						if (point_count >= 2) {
							ribbon_points.resize(point_count * 2);
							ribbon_colors.resize(point_count * 2);

							for (int j = 0; j < point_count; j++) {
								float pt_alpha = (float)(j + 1) / point_count;
								Color pt_col = col;
								pt_col.a *= pt_alpha;
								float pt_width = width;
								if (tapering) {
									pt_width *= pt_alpha;
								}

								Vector2 dir;
								if (j == 0) {
									dir = (trace_points[1] - trace_points[0]).normalized();
								} else if (j == point_count - 1) {
									dir = (trace_points[j] - trace_points[j - 1]).normalized();
								} else {
									Vector2 d1 = (trace_points[j] - trace_points[j - 1]).normalized();
									Vector2 d2 = (trace_points[j + 1] - trace_points[j]).normalized();
									dir = (d1 + d2).normalized();
								}

								Vector2 normal = Vector2(-dir.y, dir.x) * (MAX(1.0f, pt_width) * 0.5f);
								ribbon_points.write[j] = trace_points[j] + normal;
								ribbon_points.write[point_count * 2 - 1 - j] = trace_points[j] - normal;

								ribbon_colors.write[j] = pt_col;
								ribbon_colors.write[point_count * 2 - 1 - j] = pt_col;
							}
							draw_polygon(ribbon_points, ribbon_colors);
						}
					} else {
						// Segment-based styles (Dotted, Dashed, Squares)
						float pattern_offset_accumulator = 0.0f;
						const float STROKE_SPACING_FACTOR = 2.0f; // Gap size relative to width

						for (int i = 0; i < point_count - 1; i++) {
							float alpha = (float)(i + 1) / point_count;
							Color c = col;
							c.a *= alpha;

							float w = width;
							if (tapering) {
								w *= alpha;
							}

							float segment_len = trace_points[i].distance_to(trace_points[i + 1]);

							switch (trace_style) {
								case TRACE_STYLE_DOTTED: {
									// Only draw if we have moved enough
									pattern_offset_accumulator += segment_len;
									float dot_spacing = MAX(1.0f, w) * STROKE_SPACING_FACTOR;
									if (pattern_offset_accumulator >= dot_spacing) {
										draw_circle(trace_points[i], MAX(1.0f, w) * 0.5f, c);
										pattern_offset_accumulator = 0; // Reset or decrement
									}
								} break;
								case TRACE_STYLE_DASHED: {
									if (i % 2 == 0) {
										draw_line(trace_points[i], trace_points[i + 1], c, MAX(1.0f, w), true);
									}
								} break;
								case TRACE_STYLE_SQUARES: {
									pattern_offset_accumulator += segment_len;
									float square_spacing = MAX(1.0f, w) * STROKE_SPACING_FACTOR;
									if (pattern_offset_accumulator >= square_spacing) {
										draw_rect(Rect2(trace_points[i] - Vector2(w, w) * 0.5f, Vector2(w, w)), c, true);
										pattern_offset_accumulator = 0;
									}
								} break;
								default:
									break;
							}
						}
					}
					// Draw last point for dots/squares to ensure the cursor tip is visible
					if (point_count > 0) {
						if (trace_style == TRACE_STYLE_DOTTED) {
							draw_circle(trace_points[point_count - 1], MAX(1.0f, width) * 0.5f, col);
						} else if (trace_style == TRACE_STYLE_SQUARES) {
							draw_rect(Rect2(trace_points[point_count - 1] - Vector2(width, width) * 0.5f, Vector2(width, width)), col, true);
						}
					}
				}
			}
		} break;
		case NOTIFICATION_THEME_CHANGED: {
			_update_theme_item_cache();
			queue_redraw();
		} break;
	}
}

void _update_fx(Node *p_root, const Vector2 &p_pos, bool p_emitting) {
	ERR_FAIL_NULL(p_root);
	for (int i = 0; i < p_root->get_child_count(); i++) {
		Node *child = p_root->get_child(i);
		if (!child || !child->is_inside_tree()) {
			continue;
		}

		if (CPUParticles2D *cpu_part = Object::cast_to<CPUParticles2D>(child)) {
			cpu_part->set_emitting(p_emitting);
			if (p_emitting && cpu_part->is_inside_tree()) {
				cpu_part->set_position(p_pos);
			}
		} else if (GPUParticles2D *gpu_part = Object::cast_to<GPUParticles2D>(child)) {
			gpu_part->set_emitting(p_emitting);
			if (p_emitting && gpu_part->is_inside_tree()) {
				gpu_part->set_position(p_pos);
			}
		}
	}
}

void VirtualTouchPad::_on_drag(int p_index, const Vector2 &p_pos, const Vector2 &p_relative) {
	accumulated_relative += p_relative;

	last_pos = current_pos;
	current_pos = p_pos;

	trace_points.push_back(p_pos);
	if (trace_points.size() > trace_length) {
		trace_points.remove_at(0);
	}

	_update_fx(this, current_pos, true);

	queue_redraw();
}

void VirtualTouchPad::_send_axis_events(const Vector2 &p_deflection) {
	int axis_x = (hand == HAND_LEFT) ? 0 : 2;
	int axis_y = (hand == HAND_LEFT) ? 1 : 3;

	Input *input = Input::get_singleton();

	if (axis_x != -1) {
		Ref<InputEventVirtualMotion> ie_x;
		ie_x.instantiate();
		ie_x->set_device(get_device());
		ie_x->set_axis(axis_x);
		ie_x->set_axis_value(p_deflection.x);
		if (input && get_input_mode() == INPUT_MODE_NATIVE) {
			input->parse_input_event(ie_x);
		}
	}

	if (axis_y != -1) {
		Ref<InputEventVirtualMotion> ie_y;
		ie_y.instantiate();
		ie_y->set_device(get_device());
		ie_y->set_axis(axis_y);
		ie_y->set_axis_value(p_deflection.y);
		if (input && get_input_mode() == INPUT_MODE_NATIVE) {
			input->parse_input_event(ie_y);
		}
	}
}

void VirtualTouchPad::_reset_touchpad() {
	_send_axis_events(Vector2());
	last_sent_deflection = Vector2();
	accumulated_relative = Vector2();
	_update_fx(this, current_pos, false);
}

void VirtualTouchPad::pressed_state_changed() {
	if (!is_pressed()) {
		_reset_touchpad();
	}
}

void VirtualTouchPad::set_hand(TouchPadHand p_hand) {
	hand = p_hand;
}

void VirtualTouchPad::_on_touch_down(int p_index, const Vector2 &p_pos) {
	last_pos = p_pos;
	current_pos = p_pos;
	accumulated_relative = Vector2();
	trace_points.clear();
	trace_points.push_back(p_pos);
	fade_accumulator = 0.0f; // Reset fade accumulator on new touch
	_update_fx(this, current_pos, true);
	queue_redraw();
}

void VirtualTouchPad::_on_touch_up(int p_index, const Vector2 &p_pos) {
	_reset_touchpad();
	queue_redraw();
}

void VirtualTouchPad::set_sensitivity(float p_sensitivity) {
	sensitivity = p_sensitivity;
}

float VirtualTouchPad::get_sensitivity() const {
	return sensitivity;
}

void VirtualTouchPad::set_trace_length(int p_length) {
	trace_length = MAX(2, p_length);
}

int VirtualTouchPad::get_trace_length() const {
	return trace_length;
}

void VirtualTouchPad::set_fade_duration(float p_duration) {
	fade_duration = MAX(0.01f, p_duration);
}

float VirtualTouchPad::get_fade_duration() const {
	return fade_duration;
}

void VirtualTouchPad::set_base_color(const Color &p_color) {
	base_color = p_color;
	queue_redraw();
}

Color VirtualTouchPad::get_base_color() const {
	return base_color;
}

void VirtualTouchPad::set_base_width(float p_width) {
	base_width = p_width;
	queue_redraw();
}

float VirtualTouchPad::get_base_width() const {
	return base_width;
}

void VirtualTouchPad::set_tapering(bool p_tapering) {
	tapering = p_tapering;
	queue_redraw();
}

bool VirtualTouchPad::is_tapering() const {
	return tapering;
}

void VirtualTouchPad::set_mode(TouchPadMode p_mode) {
	mode = p_mode;
	queue_redraw();
}

VirtualTouchPad::TouchPadMode VirtualTouchPad::get_mode() const {
	return mode;
}

void VirtualTouchPad::set_trace_style(TraceStyle p_style) {
	trace_style = p_style;
	queue_redraw();
}

VirtualTouchPad::TraceStyle VirtualTouchPad::get_trace_style() const {
	return trace_style;
}

void VirtualTouchPad::set_circle_radius(float p_radius) {
	circle_radius = p_radius;
	queue_redraw();
}

float VirtualTouchPad::get_circle_radius() const {
	return circle_radius;
}

void VirtualTouchPad::set_circle_width(float p_width) {
	circle_width = p_width;
	queue_redraw();
}

float VirtualTouchPad::get_circle_width() const {
	return circle_width;
}

void VirtualTouchPad::set_circle_color(const Color &p_color) {
	circle_color = p_color;
	queue_redraw();
}

Color VirtualTouchPad::get_circle_color() const {
	return circle_color;
}

void VirtualTouchPad::set_action(int p_direction, const StringName &p_action) {
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

StringName VirtualTouchPad::get_action(int p_direction) const {
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

VirtualTouchPad::VirtualTouchPad() {
	action_up = "ui_up";
	action_down = "ui_down";
	action_left = "ui_left";
	action_right = "ui_right";

	set_process_internal(true);
}

VirtualTouchPad::~VirtualTouchPad() {
}

Size2 VirtualTouchPad::get_minimum_size() const {
	return Size2(20, 20);
}

void VirtualTouchPad::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_sensitivity", "sensitivity"), &VirtualTouchPad::set_sensitivity);
	ClassDB::bind_method(D_METHOD("get_sensitivity"), &VirtualTouchPad::get_sensitivity);
	ClassDB::bind_method(D_METHOD("set_hand", "hand"), &VirtualTouchPad::set_hand);
	ClassDB::bind_method(D_METHOD("get_hand"), &VirtualTouchPad::get_hand);

	ClassDB::bind_method(D_METHOD("set_trace_length", "length"), &VirtualTouchPad::set_trace_length);
	ClassDB::bind_method(D_METHOD("get_trace_length"), &VirtualTouchPad::get_trace_length);

	ClassDB::bind_method(D_METHOD("set_fade_duration", "duration"), &VirtualTouchPad::set_fade_duration);
	ClassDB::bind_method(D_METHOD("get_fade_duration"), &VirtualTouchPad::get_fade_duration);

	ClassDB::bind_method(D_METHOD("set_base_color", "color"), &VirtualTouchPad::set_base_color);
	ClassDB::bind_method(D_METHOD("get_base_color"), &VirtualTouchPad::get_base_color);

	ClassDB::bind_method(D_METHOD("set_base_width", "width"), &VirtualTouchPad::set_base_width);
	ClassDB::bind_method(D_METHOD("get_base_width"), &VirtualTouchPad::get_base_width);

	ClassDB::bind_method(D_METHOD("set_tapering", "tapering"), &VirtualTouchPad::set_tapering);
	ClassDB::bind_method(D_METHOD("is_tapering"), &VirtualTouchPad::is_tapering);

	ClassDB::bind_method(D_METHOD("set_mode", "mode"), &VirtualTouchPad::set_mode);
	ClassDB::bind_method(D_METHOD("get_mode"), &VirtualTouchPad::get_mode);

	ClassDB::bind_method(D_METHOD("set_trace_style", "style"), &VirtualTouchPad::set_trace_style);
	ClassDB::bind_method(D_METHOD("get_trace_style"), &VirtualTouchPad::get_trace_style);

	ClassDB::bind_method(D_METHOD("set_circle_radius", "radius"), &VirtualTouchPad::set_circle_radius);
	ClassDB::bind_method(D_METHOD("get_circle_radius"), &VirtualTouchPad::get_circle_radius);

	ClassDB::bind_method(D_METHOD("set_circle_width", "width"), &VirtualTouchPad::set_circle_width);
	ClassDB::bind_method(D_METHOD("get_circle_width"), &VirtualTouchPad::get_circle_width);

	ClassDB::bind_method(D_METHOD("set_circle_color", "color"), &VirtualTouchPad::set_circle_color);
	ClassDB::bind_method(D_METHOD("get_circle_color"), &VirtualTouchPad::get_circle_color);

	ClassDB::bind_method(D_METHOD("set_action", "direction", "action"), &VirtualTouchPad::set_action);
	ClassDB::bind_method(D_METHOD("get_action", "direction"), &VirtualTouchPad::get_action);

	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "sensitivity"), "set_sensitivity", "get_sensitivity");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "hand", PROPERTY_HINT_ENUM, "Left,Right"), "set_hand", "get_hand");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "mode", PROPERTY_HINT_ENUM, "None,Circle,Trace"), "set_mode", "get_mode");

	ADD_GROUP("Emulated Mode", "action_");
	ADD_PROPERTYI(PropertyInfo(Variant::STRING_NAME, "action_up", PROPERTY_HINT_INPUT_NAME, "show_builtin,loose_mode"), "set_action", "get_action", DIR_UP);
	ADD_PROPERTYI(PropertyInfo(Variant::STRING_NAME, "action_down", PROPERTY_HINT_INPUT_NAME, "show_builtin,loose_mode"), "set_action", "get_action", DIR_DOWN);
	ADD_PROPERTYI(PropertyInfo(Variant::STRING_NAME, "action_left", PROPERTY_HINT_INPUT_NAME, "show_builtin,loose_mode"), "set_action", "get_action", DIR_LEFT);
	ADD_PROPERTYI(PropertyInfo(Variant::STRING_NAME, "action_right", PROPERTY_HINT_INPUT_NAME, "show_builtin,loose_mode"), "set_action", "get_action", DIR_RIGHT);

	ADD_GROUP("Trace", "trace_");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "trace_style", PROPERTY_HINT_ENUM, "Solid,Dotted,Dashed,Squares"), "set_trace_style", "get_trace_style");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "trace_length", PROPERTY_HINT_RANGE, "2,100,1"), "set_trace_length", "get_trace_length");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "trace_fade_duration", PROPERTY_HINT_RANGE, "0.01,5.0,0.01,suffix:s"), "set_fade_duration", "get_fade_duration");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "trace_base_color"), "set_base_color", "get_base_color");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "trace_base_width", PROPERTY_HINT_RANGE, "0,50,0.1,or_greater"), "set_base_width", "get_base_width");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "trace_tapering"), "set_tapering", "is_tapering");

	ADD_GROUP("Circle", "circle_");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "circle_radius", PROPERTY_HINT_RANGE, "1.0,200.0,0.5"), "set_circle_radius", "get_circle_radius");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "circle_width", PROPERTY_HINT_RANGE, "0.5,50.0,0.5"), "set_circle_width", "get_circle_width");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "circle_color"), "set_circle_color", "get_circle_color");

	BIND_ENUM_CONSTANT(MODE_NONE);
	BIND_ENUM_CONSTANT(MODE_CIRCLE);
	BIND_ENUM_CONSTANT(MODE_TRACE);

	BIND_ENUM_CONSTANT(TRACE_STYLE_SOLID);
	BIND_ENUM_CONSTANT(TRACE_STYLE_DOTTED);
	BIND_ENUM_CONSTANT(TRACE_STYLE_DASHED);
	BIND_ENUM_CONSTANT(TRACE_STYLE_SQUARES);

	BIND_ENUM_CONSTANT(HAND_LEFT);
	BIND_ENUM_CONSTANT(HAND_RIGHT);

	BIND_THEME_ITEM(Theme::DATA_TYPE_STYLEBOX, VirtualTouchPad, style_panel);
	BIND_THEME_ITEM(Theme::DATA_TYPE_CONSTANT, VirtualTouchPad, trace_width);
	BIND_THEME_ITEM_CUSTOM(Theme::DATA_TYPE_COLOR, VirtualTouchPad, trace_color, "trace_color");
	BIND_THEME_ITEM_CUSTOM(Theme::DATA_TYPE_ICON, VirtualTouchPad, trace_texture, "trace_texture");
}

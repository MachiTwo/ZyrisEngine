/**************************************************************************/
/*  virtual_touch_pad.h                                                   */
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

#include "scene/gui/virtual_device.h"

class Timer;

class VirtualTouchPad : public VirtualDevice {
	GDCLASS(VirtualTouchPad, VirtualDevice);

public:
	enum TouchPadHand {
		HAND_LEFT,
		HAND_RIGHT,
	};

	enum TouchPadMode {
		MODE_NONE,
		MODE_CIRCLE,
		MODE_TRACE,
	};

	enum TraceStyle {
		TRACE_STYLE_SOLID,
		TRACE_STYLE_DOTTED,
		TRACE_STYLE_DASHED,
		TRACE_STYLE_SQUARES,
	};

	enum TouchPadDirection {
		DIR_UP,
		DIR_DOWN,
		DIR_LEFT,
		DIR_RIGHT,
	};

private:
	// Axis Mapping is derived from hand property

protected:
	// Settings - accessible to derived touchpads
	float sensitivity = 1.0f;
	TouchPadHand hand = HAND_LEFT;

	// Emulated mode actions
	StringName action_up = "ui_up";
	StringName action_down = "ui_down";
	StringName action_left = "ui_left";
	StringName action_right = "ui_right";

	// Visuals
	TouchPadMode mode = MODE_CIRCLE;

	// Trace Settings
	TraceStyle trace_style = TRACE_STYLE_SOLID;
	int trace_length = 30; // Max points in trail
	float fade_duration = 1.0f; // Time to fade out completely
	Color base_color = Color(1, 1, 1, 0.5); // If alpha is 0, use theme color
	float base_width = 4.0f; // If <= 0, use theme constant
	bool tapering = false;

	// Circle Settings
	float circle_radius = 50.0f;
	float circle_width = 4.0f;
	Color circle_color = Color(1, 1, 1, 0.5);

	Vector<Vector2> trace_points;
	Vector<Vector2> ribbon_points;
	Vector<Color> ribbon_colors;
	Vector2 last_pos;
	Vector2 current_pos;
	Vector2 accumulated_relative;
	Vector2 last_sent_deflection;
	float fade_accumulator = 0.0f;

	void _send_axis_events(const Vector2 &p_deflection);

	struct ThemeCache {
		Ref<StyleBox> style_panel;
		Color trace_color;
		int trace_width = 2; // Default
		Ref<Texture2D> trace_texture; // Optional pattern
	} theme_cache;

	virtual void _update_theme_item_cache() override;
	void _notification(int p_what);
	static void _bind_methods();

	virtual void _on_touch_down(int p_index, const Vector2 &p_pos) override;
	virtual void _on_touch_up(int p_index, const Vector2 &p_pos) override;
	virtual void _on_drag(int p_index, const Vector2 &p_pos, const Vector2 &p_relative) override;

	void _reset_touchpad();
	virtual void pressed_state_changed() override;

	virtual void _get_emulated_actions(Vector<StringName> &r_actions) const override {
		if (last_sent_deflection.y < -0.3f && !action_up.is_empty()) {
			r_actions.push_back(action_up);
		} else if (last_sent_deflection.y > 0.3f && !action_down.is_empty()) {
			r_actions.push_back(action_down);
		}
		if (last_sent_deflection.x < -0.3f && !action_left.is_empty()) {
			r_actions.push_back(action_left);
		} else if (last_sent_deflection.x > 0.3f && !action_right.is_empty()) {
			r_actions.push_back(action_right);
		}
	}
	virtual Size2 get_minimum_size() const override;

public:
	void set_sensitivity(float p_sensitivity);
	float get_sensitivity() const;

	void set_hand(TouchPadHand p_hand);
	TouchPadHand get_hand() const { return hand; }

	void set_trace_length(int p_length);
	int get_trace_length() const;

	void set_fade_duration(float p_duration);
	float get_fade_duration() const;

	void set_base_color(const Color &p_color);
	Color get_base_color() const;

	void set_base_width(float p_width);
	float get_base_width() const;

	void set_tapering(bool p_tapering);
	bool is_tapering() const;

	void set_mode(TouchPadMode p_mode);
	TouchPadMode get_mode() const;

	void set_trace_style(TraceStyle p_style);
	TraceStyle get_trace_style() const;

	void set_circle_radius(float p_radius);
	float get_circle_radius() const;

	void set_circle_width(float p_width);
	float get_circle_width() const;

	void set_circle_color(const Color &p_color);
	Color get_circle_color() const;

	void set_action(int p_direction, const StringName &p_action);
	StringName get_action(int p_direction) const;

	VirtualTouchPad();
	~VirtualTouchPad() override;
};

VARIANT_ENUM_CAST(VirtualTouchPad::TouchPadHand);
VARIANT_ENUM_CAST(VirtualTouchPad::TouchPadMode);
VARIANT_ENUM_CAST(VirtualTouchPad::TraceStyle);

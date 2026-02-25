/**************************************************************************/
/*  virtual_dpad.h                                                        */
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

#include "core/templates/hash_map.h"
#include "scene/gui/virtual_device.h"

class VirtualDPad : public VirtualDevice {
	GDCLASS(VirtualDPad, VirtualDevice);

public:
	enum DPadDirection {
		DIR_UP,
		DIR_DOWN,
		DIR_LEFT,
		DIR_RIGHT,
		DIR_NONE
	};

private:
	// Default to indices 12-15 to leave 0-11 for generic buttons
	// Matches standard gamepad DPad layout conceptually
	int up_button_index = 12;
	int down_button_index = 13;
	int left_button_index = 14;
	int right_button_index = 15;

protected:
	// Direction state - accessible to derived dpads
	DPadDirection current_direction = DIR_NONE;
	bool pressed_up = false;
	bool pressed_down = false;
	bool pressed_left = false;
	bool pressed_right = false;

	// Emulated mode actions
	StringName action_up = "ui_up";
	StringName action_down = "ui_down";
	StringName action_left = "ui_left";
	StringName action_right = "ui_right";

	// Settings
	float deadzone_size = 15.0f;
	float button_width_ratio = 0.33f; // Default ratio

	// Visuals
	Ref<Texture2D> texture; // Base texture (optional override)

	struct ThemeCache {
		Ref<StyleBox> normal_style;
		Ref<StyleBox> pressed_style;
		Ref<StyleBox> center_style; // Optional middle style

		Ref<StyleBox> effective_normal_style;
		Ref<StyleBox> effective_pressed_style;
		Ref<StyleBox> effective_center_style;

		Ref<Texture2D> dpad_texture_theme;
		Color base_color; // Modulation

		Color border_color;
		int border_width;
	} theme_cache;

	virtual void _update_theme_item_cache() override;
	void _notification(int p_what);
	static void _bind_methods();

	virtual void _on_touch_down(int p_index, const Vector2 &p_pos) override;
	virtual void _on_touch_up(int p_index, const Vector2 &p_pos) override;
	virtual void _on_drag(int p_index, const Vector2 &p_pos, const Vector2 &p_relative) override;

	virtual void pressed_state_changed() override;

	virtual void _get_emulated_actions(Vector<StringName> &r_actions) const override {
		if (!action_up.is_empty()) {
			r_actions.push_back(action_up);
		}
		if (!action_down.is_empty()) {
			r_actions.push_back(action_down);
		}
		if (!action_left.is_empty()) {
			r_actions.push_back(action_left);
		}
		if (!action_right.is_empty()) {
			r_actions.push_back(action_right);
		}
	}

	HashMap<int, Vector2> touches;
	void _update_dpad();
	void _press_direction(DPadDirection p_dir, bool p_pressed);

	virtual Size2 get_minimum_size() const override;

public:
	void set_texture(const Ref<Texture2D> &p_texture);
	Ref<Texture2D> get_texture() const;

	void set_deadzone_size(float p_size);
	float get_deadzone_size() const;

	void set_up_button_index(int p_index);
	int get_up_button_index() const;

	void set_down_button_index(int p_index);
	int get_down_button_index() const;

	void set_left_button_index(int p_index);
	int get_left_button_index() const;

	void set_right_button_index(int p_index);
	int get_right_button_index() const;

	void set_button_width_ratio(float p_ratio);
	float get_button_width_ratio() const;

	void set_action(int p_direction, const StringName &p_action);
	StringName get_action(int p_direction) const;

	VirtualDPad();
};

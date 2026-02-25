/**************************************************************************/
/*  virtual_button.cpp                                                    */
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

#include "virtual_button.h"

#include "core/input/input.h"
#include "scene/resources/font.h"
#include "scene/resources/style_box.h"
#include "scene/resources/text_paragraph.h"
#include "scene/resources/texture.h"
#include "scene/theme/theme_db.h"

// _notification is defined later in this file with the complete implementation
void VirtualButton::pressed_state_changed() {
	// Send Event
	Ref<InputEventVirtualButton> ie;
	ie.instantiate();
	ie->set_device(get_device());
	ie->set_button_index(button_index);
	ie->set_pressed(is_pressed());

	Input *input = Input::get_singleton();
	if (input && get_input_mode() == INPUT_MODE_NATIVE) {
		input->parse_input_event(ie);
	}

	queue_redraw();
}

void VirtualButton::_shape() {
	Ref<TextParagraph> buf = text_buf; // Create local copy to avoid race conditions
	if (buf.is_null()) {
		return;
	}
	buf->clear();
	Ref<Font> active_font = font.is_valid() ? font : theme_cache.font_theme;
	int active_size = font_size > 0 ? font_size : theme_cache.font_size_theme;
	if (active_font.is_valid()) {
		buf->add_string(atr(text), active_font, active_size);
	}
}

void VirtualButton::_notification(int p_what) {
	VirtualDevice::_notification(p_what);
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE:
		case NOTIFICATION_TRANSLATION_CHANGED: {
			_shape();
		} break;
		case NOTIFICATION_DRAW: {
			RID ci = get_canvas_item();
			Size2 size = get_size();

			Ref<StyleBox> style = theme_cache.normal_style;
			Color text_color = theme_cache.font_color;
			Color icon_color = theme_cache.icon_normal_color;

			if (is_pressed()) {
				style = theme_cache.pressed_style;
				text_color = theme_cache.font_pressed_color;
				icon_color = theme_cache.icon_pressed_color;
			} else if (is_hovered()) {
				style = theme_cache.hover_style;
				text_color = theme_cache.font_hover_color;
				icon_color = theme_cache.icon_hover_color;
			}

			if (has_focus()) {
				// style = theme_cache.focus;
			}

			if (!flat && style.is_valid()) {
				style->draw(ci, Rect2(Point2(), size));
			}

			// Draw Custom Texture (if provided)
			Ref<Texture2D> active_tex = texture_normal;
			if (is_pressed() && texture_pressed.is_valid()) {
				active_tex = texture_pressed;
			} else if (is_hovered() && texture_hover.is_valid()) {
				active_tex = texture_hover;
			}

			if (active_tex.is_valid()) {
				// Consider StretchMode
				Size2 b_size = active_tex->get_size();
				Rect2 dest_rect = Rect2(Point2(), size);
				bool clip = false;

				switch (stretch_mode) {
					case STRETCH_SCALE: {
						// Default
					} break;
					case STRETCH_TILE: {
						draw_texture_rect(active_tex, dest_rect, true);
						goto skip_draw;
					} break;
					case STRETCH_KEEP: {
						dest_rect.size = b_size;
					} break;
					case STRETCH_KEEP_CENTERED: {
						dest_rect.position = (size - b_size) / 2;
						dest_rect.size = b_size;
					} break;
					case STRETCH_KEEP_ASPECT: {
						float ratio = MIN(size.width / b_size.width, size.height / b_size.height);
						dest_rect.size = b_size * ratio;
					} break;
					case STRETCH_KEEP_ASPECT_CENTERED: {
						float ratio = MIN(size.width / b_size.width, size.height / b_size.height);
						dest_rect.size = b_size * ratio;
						dest_rect.position = (size - dest_rect.size) / 2;
					} break;
					case STRETCH_KEEP_ASPECT_COVERED: {
						float ratio = MAX(size.width / b_size.width, size.height / b_size.height);
						dest_rect.size = b_size * ratio;
						dest_rect.position = (size - dest_rect.size) / 2;
						clip = true;
					} break;
				}

				if (clip) {
					RenderingServer::get_singleton()->canvas_item_set_clip(ci, true);
				}
				draw_texture_rect(active_tex, dest_rect, false);
				if (clip) {
					RenderingServer::get_singleton()->canvas_item_set_clip(ci, false);
				}
			skip_draw:;
			}

			// Draw Icon
			Ref<Texture2D> active_icon = icon;
			if (active_icon.is_null()) {
				if (has_theme_icon(SNAME("icon"), SNAME("VirtualButton"))) {
					active_icon = theme_cache.icon_theme;
				}
			}

			if (active_icon.is_valid()) {
				Size2 icon_size = active_icon->get_size();
				Vector2 icon_pos;
				switch (alignment) {
					case HORIZONTAL_ALIGNMENT_LEFT:
						icon_pos.x = style.is_valid() ? style->get_margin(SIDE_LEFT) : 0;
						break;
					case HORIZONTAL_ALIGNMENT_CENTER:
						icon_pos.x = (size.x - icon_size.x) / 2;
						break;
					case HORIZONTAL_ALIGNMENT_RIGHT:
						icon_pos.x = size.x - icon_size.x - (style.is_valid() ? style->get_margin(SIDE_RIGHT) : 0);
						break;
					default:
						break;
				}
				icon_pos.y = (size.y - icon_size.y) / 2;
				draw_texture_rect(active_icon, Rect2(icon_pos, icon_size), false, icon_color);
			}

			// Draw Text
			Ref<TextParagraph> buf = text_buf; // Create local copy to avoid race conditions
			if (!text.is_empty() && buf.is_valid()) {
				Size2 text_size = buf->get_size();
				Vector2 text_pos;
				switch (alignment) {
					case HORIZONTAL_ALIGNMENT_LEFT:
						text_pos.x = style.is_valid() ? style->get_margin(SIDE_LEFT) : 0;
						break;
					case HORIZONTAL_ALIGNMENT_CENTER:
						text_pos.x = (size.x - text_size.x) / 2;
						break;
					case HORIZONTAL_ALIGNMENT_RIGHT:
						text_pos.x = size.x - text_size.x - (style.is_valid() ? style->get_margin(SIDE_RIGHT) : 0);
						break;
					default:
						break;
				}
				text_pos.y = (size.y - text_size.y) / 2;

				if (theme_cache.outline_size > 0 && theme_cache.font_outline_color.a > 0) {
					buf->draw_outline(ci, text_pos, theme_cache.outline_size, theme_cache.font_outline_color);
				}
				buf->draw(ci, text_pos, text_color);
			}

		} break;
		case NOTIFICATION_THEME_CHANGED: {
			_update_theme_item_cache();
			_shape();
			queue_redraw();
		} break;
	}
}

void VirtualButton::_update_theme_item_cache() {
	VirtualDevice::_update_theme_item_cache();

	theme_cache.normal_style = get_theme_stylebox(SNAME("normal"));
	theme_cache.pressed_style = get_theme_stylebox(SNAME("pressed"));
	theme_cache.hover_style = get_theme_stylebox(SNAME("hover"));
	theme_cache.disabled_style = get_theme_stylebox(SNAME("disabled"));
	theme_cache.focus_style = get_theme_stylebox(SNAME("focus"));

	theme_cache.font_theme = get_theme_font(SNAME("font"));
	theme_cache.font_size_theme = get_theme_font_size(SNAME("font_size"));

	theme_cache.font_color = get_theme_color(SNAME("font_color"));
	theme_cache.font_pressed_color = get_theme_color(SNAME("font_pressed_color"));
	theme_cache.font_hover_color = get_theme_color(SNAME("font_hover_color"));
	theme_cache.font_focus_color = get_theme_color(SNAME("font_focus_color"));
	theme_cache.font_disabled_color = get_theme_color("font_disabled_color");
	theme_cache.font_outline_color = get_theme_color(SNAME("font_outline_color"));
	theme_cache.outline_size = get_theme_constant(SNAME("outline_size"));

	theme_cache.icon_theme = get_theme_icon(SNAME("icon"));
	theme_cache.icon_normal_color = get_theme_color(SNAME("icon_normal_color"));
	theme_cache.icon_pressed_color = get_theme_color(SNAME("icon_pressed_color"));
	theme_cache.icon_hover_color = get_theme_color(SNAME("icon_hover_color"));
	theme_cache.icon_focus_color = get_theme_color(SNAME("icon_focus_color"));
	theme_cache.icon_disabled_color = get_theme_color(SNAME("icon_disabled_color"));
}

void VirtualButton::set_text(const String &p_text) {
	if (text != p_text) {
		text = p_text;
		_shape();
		queue_redraw();
	}
}

String VirtualButton::get_text() const {
	return text;
}

void VirtualButton::set_alignment(HorizontalAlignment p_alignment) {
	if (alignment != p_alignment) {
		alignment = p_alignment;
		queue_redraw();
	}
}

HorizontalAlignment VirtualButton::get_alignment() const {
	return alignment;
}

Size2 VirtualButton::get_minimum_size() const {
	if (ignore_texture_size) {
		return Size2();
	}

	Size2 min_size;

	// Calculate from text/font
	Ref<Font> active_font = font.is_valid() ? font : theme_cache.font_theme;
	int active_size = font_size > 0 ? font_size : theme_cache.font_size_theme;
	if (!text.is_empty() && active_font.is_valid()) {
		min_size = active_font->get_string_size(text, HORIZONTAL_ALIGNMENT_LEFT, -1, active_size);
	}

	// Add stylebox margins
	if (theme_cache.normal_style.is_valid()) {
		min_size += theme_cache.normal_style->get_minimum_size();
	}

	// Consider Icon size
	Ref<Texture2D> active_icon = icon;
	if (active_icon.is_null()) {
		active_icon = theme_cache.icon_theme;
	}
	if (active_icon.is_valid()) {
		min_size = min_size.max(active_icon->get_size());
	}

	return min_size;
}

void VirtualButton::set_button_index(int p_index) {
	button_index = p_index;
}

int VirtualButton::get_button_index() const {
	return button_index;
}

void VirtualButton::set_font(const Ref<Font> &p_font) {
	font = p_font;
	_shape();
	update_minimum_size();
	queue_redraw();
}

Ref<Font> VirtualButton::get_font() const {
	return font;
}

void VirtualButton::set_font_size(int p_size) {
	font_size = p_size;
	_shape();
	update_minimum_size();
	queue_redraw();
}

int VirtualButton::get_font_size() const {
	return font_size;
}

void VirtualButton::set_texture_normal(const Ref<Texture2D> &p_normal) {
	texture_normal = p_normal;
	update_minimum_size();
	queue_redraw();
}

Ref<Texture2D> VirtualButton::get_texture_normal() const {
	return texture_normal;
}

void VirtualButton::set_texture_pressed(const Ref<Texture2D> &p_pressed) {
	texture_pressed = p_pressed;
	queue_redraw();
}

Ref<Texture2D> VirtualButton::get_texture_pressed() const {
	return texture_pressed;
}

void VirtualButton::set_texture_hover(const Ref<Texture2D> &p_hover) {
	texture_hover = p_hover;
	queue_redraw();
}

Ref<Texture2D> VirtualButton::get_texture_hover() const {
	return texture_hover;
}

void VirtualButton::set_texture_disabled(const Ref<Texture2D> &p_disabled) {
	texture_disabled = p_disabled;
	queue_redraw();
}

Ref<Texture2D> VirtualButton::get_texture_disabled() const {
	return texture_disabled;
}

void VirtualButton::set_texture_focused(const Ref<Texture2D> &p_focused) {
	texture_focused = p_focused;
	queue_redraw();
}

Ref<Texture2D> VirtualButton::get_texture_focused() const {
	return texture_focused;
}

void VirtualButton::set_icon(const Ref<Texture2D> &p_icon) {
	icon = p_icon;
	update_minimum_size();
	queue_redraw();
}

Ref<Texture2D> VirtualButton::get_icon() const {
	return icon;
}

void VirtualButton::set_ignore_texture_size(bool p_ignore) {
	ignore_texture_size = p_ignore;
	update_minimum_size();
	queue_redraw();
}

bool VirtualButton::get_ignore_texture_size() const {
	return ignore_texture_size;
}

void VirtualButton::set_stretch_mode(StretchMode p_mode) {
	stretch_mode = p_mode;
	queue_redraw();
}

VirtualButton::StretchMode VirtualButton::get_stretch_mode() const {
	return stretch_mode;
}

void VirtualButton::set_flat(bool p_flat) {
	flat = p_flat;
	queue_redraw();
}

bool VirtualButton::is_flat() const {
	return flat;
}

VirtualButton::VirtualButton() {
	text_buf.instantiate();
	ignore_texture_size = false;
	stretch_mode = STRETCH_SCALE;
}

VirtualButton::~VirtualButton() {
	// Ref<> handles automatic cleanup, but ensure any manual resources are cleared
	text_buf.unref();
}

void VirtualButton::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_button_index", "index"), &VirtualButton::set_button_index);
	ClassDB::bind_method(D_METHOD("get_button_index"), &VirtualButton::get_button_index);

	ClassDB::bind_method(D_METHOD("set_text", "text"), &VirtualButton::set_text);
	ClassDB::bind_method(D_METHOD("get_text"), &VirtualButton::get_text);

	ClassDB::bind_method(D_METHOD("set_font", "font"), &VirtualButton::set_font);
	ClassDB::bind_method(D_METHOD("get_font"), &VirtualButton::get_font);

	ClassDB::bind_method(D_METHOD("set_font_size", "size"), &VirtualButton::set_font_size);
	ClassDB::bind_method(D_METHOD("get_font_size"), &VirtualButton::get_font_size);

	ClassDB::bind_method(D_METHOD("set_icon", "icon"), &VirtualButton::set_icon);
	ClassDB::bind_method(D_METHOD("get_icon"), &VirtualButton::get_icon);

	ClassDB::bind_method(D_METHOD("set_alignment", "alignment"), &VirtualButton::set_alignment);
	ClassDB::bind_method(D_METHOD("get_alignment"), &VirtualButton::get_alignment);

	ClassDB::bind_method(D_METHOD("set_texture_normal", "texture"), &VirtualButton::set_texture_normal);
	ClassDB::bind_method(D_METHOD("get_texture_normal"), &VirtualButton::get_texture_normal);

	ClassDB::bind_method(D_METHOD("set_texture_pressed", "texture"), &VirtualButton::set_texture_pressed);
	ClassDB::bind_method(D_METHOD("get_texture_pressed"), &VirtualButton::get_texture_pressed);

	ClassDB::bind_method(D_METHOD("set_texture_hover", "texture"), &VirtualButton::set_texture_hover);
	ClassDB::bind_method(D_METHOD("get_texture_hover"), &VirtualButton::get_texture_hover);

	ClassDB::bind_method(D_METHOD("set_texture_disabled", "texture"), &VirtualButton::set_texture_disabled);
	ClassDB::bind_method(D_METHOD("get_texture_disabled"), &VirtualButton::get_texture_disabled);

	ClassDB::bind_method(D_METHOD("set_texture_focused", "texture"), &VirtualButton::set_texture_focused);
	ClassDB::bind_method(D_METHOD("get_texture_focused"), &VirtualButton::get_texture_focused);

	ClassDB::bind_method(D_METHOD("set_ignore_texture_size", "ignore"), &VirtualButton::set_ignore_texture_size);
	ClassDB::bind_method(D_METHOD("get_ignore_texture_size"), &VirtualButton::get_ignore_texture_size);

	ClassDB::bind_method(D_METHOD("set_stretch_mode", "mode"), &VirtualButton::set_stretch_mode);
	ClassDB::bind_method(D_METHOD("get_stretch_mode"), &VirtualButton::get_stretch_mode);

	ClassDB::bind_method(D_METHOD("set_flat", "flat"), &VirtualButton::set_flat);
	ClassDB::bind_method(D_METHOD("is_flat"), &VirtualButton::is_flat);

	ClassDB::bind_method(D_METHOD("set_action", "action"), &VirtualButton::set_action);
	ClassDB::bind_method(D_METHOD("get_action"), &VirtualButton::get_action);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "button_index", PROPERTY_HINT_RANGE, "0,15,1"), "set_button_index", "get_button_index");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "text"), "set_text", "get_text");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "font", PROPERTY_HINT_RESOURCE_TYPE, "Font"), "set_font", "get_font");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "font_size", PROPERTY_HINT_RANGE, "1,256,1,or_greater"), "set_font_size", "get_font_size");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "icon", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "set_icon", "get_icon");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "alignment", PROPERTY_HINT_ENUM, "Left,Center,Right"), "set_alignment", "get_alignment");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "flat"), "set_flat", "is_flat");

	ADD_GROUP("Emulated Mode", "action_");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "action", PROPERTY_HINT_INPUT_NAME, "show_builtin,loose_mode"), "set_action", "get_action");

	ADD_GROUP("Textures", "texture_");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "texture_normal", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "set_texture_normal", "get_texture_normal");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "texture_pressed", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "set_texture_pressed", "get_texture_pressed");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "texture_hover", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "set_texture_hover", "get_texture_hover");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "texture_disabled", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "set_texture_disabled", "get_texture_disabled");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "texture_focused", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "set_texture_focused", "get_texture_focused");

	ADD_GROUP("Layout", "");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "ignore_texture_size"), "set_ignore_texture_size", "get_ignore_texture_size");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "stretch_mode", PROPERTY_HINT_ENUM, "Scale,Tile,Keep,Keep Centered,Keep Aspect,Keep Aspect Centered,Keep Aspect Covered"), "set_stretch_mode", "get_stretch_mode");

	BIND_ENUM_CONSTANT(STRETCH_SCALE);
	BIND_ENUM_CONSTANT(STRETCH_TILE);
	BIND_ENUM_CONSTANT(STRETCH_KEEP);
	BIND_ENUM_CONSTANT(STRETCH_KEEP_CENTERED);
	BIND_ENUM_CONSTANT(STRETCH_KEEP_ASPECT);
	BIND_ENUM_CONSTANT(STRETCH_KEEP_ASPECT_CENTERED);
	BIND_ENUM_CONSTANT(STRETCH_KEEP_ASPECT_COVERED);

	BIND_THEME_ITEM_CUSTOM(Theme::DATA_TYPE_STYLEBOX, VirtualButton, normal_style, "normal");
	BIND_THEME_ITEM_CUSTOM(Theme::DATA_TYPE_STYLEBOX, VirtualButton, pressed_style, "pressed");
	BIND_THEME_ITEM_CUSTOM(Theme::DATA_TYPE_STYLEBOX, VirtualButton, hover_style, "hover");
	BIND_THEME_ITEM_CUSTOM(Theme::DATA_TYPE_STYLEBOX, VirtualButton, disabled_style, "disabled");
	BIND_THEME_ITEM_CUSTOM(Theme::DATA_TYPE_STYLEBOX, VirtualButton, focus_style, "focus");

	BIND_THEME_ITEM(Theme::DATA_TYPE_COLOR, VirtualButton, font_color);
	BIND_THEME_ITEM(Theme::DATA_TYPE_COLOR, VirtualButton, font_focus_color);
	BIND_THEME_ITEM(Theme::DATA_TYPE_COLOR, VirtualButton, font_pressed_color);
	BIND_THEME_ITEM(Theme::DATA_TYPE_COLOR, VirtualButton, font_hover_color);
	BIND_THEME_ITEM(Theme::DATA_TYPE_COLOR, VirtualButton, font_disabled_color);
	BIND_THEME_ITEM(Theme::DATA_TYPE_COLOR, VirtualButton, font_outline_color);

	BIND_THEME_ITEM_CUSTOM(Theme::DATA_TYPE_FONT, VirtualButton, font_theme, "font");
	BIND_THEME_ITEM_CUSTOM(Theme::DATA_TYPE_FONT_SIZE, VirtualButton, font_size_theme, "font_size");
	BIND_THEME_ITEM(Theme::DATA_TYPE_CONSTANT, VirtualButton, outline_size);

	BIND_THEME_ITEM(Theme::DATA_TYPE_COLOR, VirtualButton, icon_normal_color);
	BIND_THEME_ITEM(Theme::DATA_TYPE_COLOR, VirtualButton, icon_focus_color);
	BIND_THEME_ITEM(Theme::DATA_TYPE_COLOR, VirtualButton, icon_pressed_color);
	BIND_THEME_ITEM(Theme::DATA_TYPE_COLOR, VirtualButton, icon_hover_color);
	BIND_THEME_ITEM(Theme::DATA_TYPE_COLOR, VirtualButton, icon_disabled_color);

	BIND_THEME_ITEM_CUSTOM(Theme::DATA_TYPE_ICON, VirtualButton, icon_theme, "icon");
}

/**************************************************************************/
/*  ability_system_tags_editor.cpp                                        */
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

#include "ability_system_tags_editor.h"

#include "core/config/project_settings.h"
#include "editor/editor_node.h"
#include "editor/editor_string_names.h"
#include "editor/themes/editor_scale.h"
#include "modules/ability_system/core/ability_system.h"
#include "scene/gui/margin_container.h"
#include "scene/gui/separator.h"

void AbilitySystemTagsEditor::_add_tag() {
	String tag = add_tag_edit->get_text().strip_edges();
	if (tag.is_empty()) {
		return;
	}

	AbilitySystem *as = AbilitySystem::get_singleton();
	if (as) {
		as->register_tag(tag);
		add_tag_edit->clear();
		update_tags();
	}
}

void AbilitySystemTagsEditor::_add_tag_text(const String &p_tag) {
	_add_tag();
}

void AbilitySystemTagsEditor::_tag_removed(Object *p_item, int p_column, int p_id, MouseButton p_button) {
	TreeItem *item = Object::cast_to<TreeItem>(p_item);
	if (!item) {
		return;
	}

	String tag = item->get_text(0);
	AbilitySystem *as = AbilitySystem::get_singleton();
	if (as) {
		as->unregister_tag(tag);
		update_tags();
	}
}

void AbilitySystemTagsEditor::_on_search_changed(const String &p_text) {
	update_tags();
}

void AbilitySystemTagsEditor::update_tags() {
	if (add_tag_edit) {
		add_tag_edit->set_placeholder(TTR("Add new tag (e.g., state.stun)"));
	}
	tags_tree->clear();
	tags_tree->set_columns(2);
	TreeItem *root = tags_tree->create_item();

	AbilitySystem *as = AbilitySystem::get_singleton();
	if (!as) {
		return;
	}

	String search_text = search_edit->get_text().to_lower();

	TypedArray<StringName> tags = as->get_registered_tags();
	tags.sort();

	for (int i = 0; i < tags.size(); i++) {
		String tag_name = tags[i];
		if (!search_text.is_empty() && !tag_name.to_lower().contains(search_text)) {
			continue;
		}

		TreeItem *item = tags_tree->create_item(root);
		item->set_text(0, tag_name);
		item->set_selectable(0, false);
		item->set_selectable(1, false);
		item->set_tooltip_text(0, String());
		item->set_tooltip_text(1, String());

		item->add_button(1, get_editor_theme_icon(SNAME("Remove")), 0, false, TTR("Remove Tag"));
		item->set_button_color(1, 0, Color(1, 1, 1, 0.75));
	}
}

void AbilitySystemTagsEditor::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE:
		case NOTIFICATION_THEME_CHANGED: {
			if (add_tag_button) {
				add_tag_button->set_button_icon(get_editor_theme_icon(SNAME("Add")));
			}
			update_tags();
		} break;
	}
}

void AbilitySystemTagsEditor::_bind_methods() {
}

AbilitySystemTagsEditor::AbilitySystemTagsEditor() {
	set_name("Ability System Tags");
	set_v_size_flags(SIZE_EXPAND_FILL);

	MarginContainer *main_margin = memnew(MarginContainer);
	main_margin->set_anchors_and_offsets_preset(PRESET_FULL_RECT);
	main_margin->set_v_size_flags(SIZE_EXPAND_FILL);
	main_margin->add_theme_constant_override("margin_left", 8 * EDSCALE);
	main_margin->add_theme_constant_override("margin_top", 8 * EDSCALE);
	main_margin->add_theme_constant_override("margin_right", 8 * EDSCALE);
	main_margin->add_theme_constant_override("margin_bottom", 8 * EDSCALE);
	add_child(main_margin);

	VBoxContainer *main_vbox = memnew(VBoxContainer);
	main_vbox->set_v_size_flags(SIZE_EXPAND_FILL);
	main_vbox->add_theme_constant_override("separation", 8 * EDSCALE);
	main_margin->add_child(main_vbox);

	HBoxContainer *search_hbc = memnew(HBoxContainer);
	main_vbox->add_child(search_hbc);

	search_edit = memnew(LineEdit);
	search_edit->set_h_size_flags(SIZE_EXPAND_FILL);
	search_edit->set_placeholder(TTR("Filter Tags"));
	search_edit->set_clear_button_enabled(true);
	search_hbc->add_child(search_edit);
	search_edit->connect("text_changed", callable_mp(this, &AbilitySystemTagsEditor::_on_search_changed));

	HBoxContainer *add_hbc = memnew(HBoxContainer);
	main_vbox->add_child(add_hbc);

	add_tag_edit = memnew(LineEdit);
	add_tag_edit->set_h_size_flags(SIZE_EXPAND_FILL);
	add_tag_edit->set_placeholder(TTR("Add new tag (e.g., state.stun)"));
	add_tag_edit->set_clear_button_enabled(true);
	add_tag_edit->set_keep_editing_on_text_submit(true);
	add_hbc->add_child(add_tag_edit);
	add_tag_edit->connect("text_submitted", callable_mp(this, &AbilitySystemTagsEditor::_add_tag_text));

	add_tag_button = memnew(Button);
	add_tag_button->set_text(TTR("Add"));
	add_hbc->add_child(add_tag_button);
	add_tag_button->connect("pressed", callable_mp(this, &AbilitySystemTagsEditor::_add_tag));

	main_vbox->add_child(memnew(HSeparator));

	MarginContainer *tree_margin = memnew(MarginContainer);
	tree_margin->set_v_size_flags(SIZE_EXPAND_FILL);
	tree_margin->set_theme_type_variation("NoBorderHorizontalBottom");
	main_vbox->add_child(tree_margin);

	tags_tree = memnew(Tree);
	tags_tree->set_v_size_flags(SIZE_EXPAND_FILL);
	tags_tree->set_hide_root(true);
	tags_tree->set_columns(2);
	tags_tree->set_column_expand(0, true);
	tags_tree->set_column_clip_content(0, false);
	tags_tree->set_column_expand(1, false);
	tags_tree->set_column_custom_minimum_width(1, 32 * EDSCALE);
	tags_tree->add_theme_constant_override("h_separation", 8 * EDSCALE);
	tags_tree->set_column_titles_visible(false);
	tree_margin->add_child(tags_tree);
	tags_tree->connect("button_clicked", callable_mp(this, &AbilitySystemTagsEditor::_tag_removed));

	update_tags();
	if (AbilitySystem::get_singleton()) {
		AbilitySystem::get_singleton()->connect("tags_changed", callable_mp(this, &AbilitySystemTagsEditor::update_tags));
	}
}

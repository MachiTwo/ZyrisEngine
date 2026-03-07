/**************************************************************************/
/*  ability_system_tags_panel.cpp                                         */
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

#ifdef ABILITY_SYSTEM_MODULE
#include "modules/ability_system/editor/ability_system_tags_panel.h"
#include "modules/ability_system/core/ability_system.h"
#elif defined(ABILITY_SYSTEM_GDEXTENSION)
#include "src/core/ability_system.h"
#include "src/editor/ability_system_tags_panel.h"
#endif

#ifdef ABILITY_SYSTEM_MODULE
#include "scene/gui/box_container.h"
#else
#include <godot_cpp/classes/h_box_container.hpp>
#endif
#ifdef ABILITY_SYSTEM_MODULE
#include "core/io/resource_loader.h"
#include "scene/gui/margin_container.h"
#include "scene/gui/separator.h"
#include "scene/resources/texture.h"
#else
#include <godot_cpp/classes/h_separator.hpp>
#include <godot_cpp/classes/margin_container.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#endif
#ifdef ABILITY_SYSTEM_MODULE
#include "scene/gui/box_container.h"
#else
#include <godot_cpp/classes/v_box_container.hpp>
#endif
#ifdef ABILITY_SYSTEM_MODULE
#include "core/templates/hash_set.h"
#elif defined(ABILITY_SYSTEM_GDEXTENSION)
#include <godot_cpp/templates/hash_set.hpp>
#endif

namespace godot {

void AbilitySystemTagsPanel::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_add_tag"), &AbilitySystemTagsPanel::_add_tag);
	ClassDB::bind_method(D_METHOD("_add_tag_text", "tag"), &AbilitySystemTagsPanel::_add_tag_text);
	ClassDB::bind_method(D_METHOD("_tag_removed"), &AbilitySystemTagsPanel::_tag_removed);
	ClassDB::bind_method(D_METHOD("_add_sub_tag"), &AbilitySystemTagsPanel::_add_sub_tag);
	ClassDB::bind_method(D_METHOD("_on_search_changed"), &AbilitySystemTagsPanel::_on_search_changed);
	ClassDB::bind_method(D_METHOD("update_tags"), &AbilitySystemTagsPanel::update_tags);
}

void AbilitySystemTagsPanel::_add_tag() {
	String tag = add_tag_edit->get_text().strip_edges();
	if (tag.is_empty()) {
		return;
	}

	AbilitySystem *as = AbilitySystem::get_singleton();
	if (as) {
		AbilitySystem::TagType type = (tabs->get_current_tab() == 0) ? AbilitySystem::TAG_TYPE_NAME : AbilitySystem::TAG_TYPE_CONDITIONAL;
		as->register_tag(tag, type);
		add_tag_edit->clear();
		update_tags();
	}
}

void AbilitySystemTagsPanel::_add_tag_text(const String &p_tag) {
	_add_tag();
}

void AbilitySystemTagsPanel::_tag_removed(Object *p_item, int p_column, int p_id, MouseButton p_button) {
	TreeItem *item = Object::cast_to<TreeItem>(p_item);
	if (!item || p_id != 1) { // 1 is Remove
		return;
	}

	String tag = item->get_metadata(0);
	AbilitySystem *as = AbilitySystem::get_singleton();
	if (as && !tag.is_empty()) {
		as->unregister_tag(tag);
		update_tags();
	}
}

void AbilitySystemTagsPanel::_add_sub_tag(Object *p_item, int p_column, int p_id, MouseButton p_button) {
	TreeItem *item = Object::cast_to<TreeItem>(p_item);
	if (!item || p_id != 0) { // 0 is Add
		return;
	}

	String base_tag = item->get_metadata(0);
	if (!base_tag.is_empty()) {
		add_tag_edit->set_text(base_tag + ".");
		add_tag_edit->grab_focus();
		add_tag_edit->set_caret_column(add_tag_edit->get_text().length());
	}
}

void AbilitySystemTagsPanel::_on_search_changed(const String &p_text) {
	update_tags();
}

void AbilitySystemTagsPanel::update_tags() {
	if (!is_inside_tree()) {
		return;
	}

	AbilitySystem *as = AbilitySystem::get_singleton();
	if (!as) {
		return;
	}

	String search_text = search_edit->get_text().to_lower();

	_update_tree(name_tags_tree, AbilitySystem::TAG_TYPE_NAME, search_text);
	_update_tree(cond_tags_tree, AbilitySystem::TAG_TYPE_CONDITIONAL, search_text);
}

void AbilitySystemTagsPanel::_update_tree(Tree *p_tree, AbilitySystem::TagType p_type, const String &p_search) {
	p_tree->clear();
	TreeItem *root = p_tree->create_item();

	AbilitySystem *as = AbilitySystem::get_singleton();
	TypedArray<StringName> tags = as->get_registered_tags_of_type(p_type);

	_create_tree_items(p_tree, root, "", tags, p_type, p_search);
}

void AbilitySystemTagsPanel::_create_tree_items(Tree *p_tree, TreeItem *p_parent, const String &p_prefix, const TypedArray<StringName> &p_tags, AbilitySystem::TagType p_type, const String &p_search) {
	HashSet<String> children;
	HashSet<String> exact_tags;

	for (int i = 0; i < p_tags.size(); i++) {
		String tag = p_tags[i];
		if (!p_prefix.is_empty()) {
			if (!tag.begins_with(p_prefix + String("."))) {
				continue;
			}
			tag = tag.substr(p_prefix.length() + 1);
		}

		int dot_pos = tag.find(".");
		if (dot_pos != -1) {
			children.insert(tag.substr(0, dot_pos));
		} else {
			exact_tags.insert(tag);
		}
	}

	PackedStringArray sorted_children;
	for (const String &E : children) {
		sorted_children.push_back(E);
	}
	sorted_children.sort();

	PackedStringArray sorted_exact;
	for (const String &E : exact_tags) {
		sorted_exact.push_back(E);
	}
	sorted_exact.sort();

	// Process all unique branches (prefixes and exact tags)
	HashSet<String> processed;

#ifdef ABILITY_SYSTEM_MODULE
	Ref<Texture2D> custom_tag_icon = ResourceLoader::load("res://addons/ability_system/icons/AbilitySystemTag.svg");
#else
	Ref<Texture2D> custom_tag_icon = ResourceLoader::get_singleton()->load("res://addons/ability_system/icons/AbilitySystemTag.svg");
#endif

	// Exact tags first
	for (int i = 0; i < sorted_exact.size(); i++) {
		String tag_name = sorted_exact[i];
		String full_tag = p_prefix.is_empty() ? tag_name : p_prefix + String(".") + tag_name;
		processed.insert(tag_name);

		if (!p_search.is_empty() && !full_tag.to_lower().contains(p_search)) {
			// Check if any child matches search
			bool any_child_matches = false;
			for (int j = 0; j < p_tags.size(); j++) {
				String other = p_tags[j];
				if (other.begins_with(full_tag + String(".")) && other.to_lower().contains(p_search)) {
					any_child_matches = true;
					break;
				}
			}
			if (!any_child_matches) {
				continue;
			}
		}

		TreeItem *item = p_tree->create_item(p_parent);
		item->set_text(0, tag_name);
		item->set_metadata(0, full_tag);
		item->set_selectable(0, false);
		if (custom_tag_icon.is_valid()) {
			item->set_icon(0, custom_tag_icon);
		} else {
			item->set_icon(0, get_theme_icon("Label", "EditorIcons"));
		}

		item->add_button(1, get_theme_icon("Add", "EditorIcons"), 0, false, "Add Sub-tag");
		item->add_button(1, get_theme_icon("Remove", "EditorIcons"), 1, false, "Remove Tag");
		item->set_button_color(1, 1, Color(1, 1, 1, 0.75));

		_create_tree_items(p_tree, item, full_tag, p_tags, p_type, p_search);
	}

	// Prefixes that are not registered as exact tags
	for (int i = 0; i < sorted_children.size(); i++) {
		String child_name = sorted_children[i];
		if (processed.has(child_name)) {
			continue;
		}

		String full_path = p_prefix.is_empty() ? child_name : p_prefix + String(".") + child_name;

		if (!p_search.is_empty()) {
			bool any_child_matches = false;
			for (int j = 0; j < p_tags.size(); j++) {
				String other = p_tags[j];
				if (other.begins_with(full_path + String(".")) && other.to_lower().contains(p_search)) {
					any_child_matches = true;
					break;
				}
			}
			if (!any_child_matches) {
				continue;
			}
		}

		TreeItem *item = p_tree->create_item(p_parent);
		item->set_text(0, child_name);
		item->set_metadata(0, full_path);
		item->set_selectable(0, false);
		item->set_custom_color(0, Color(1, 1, 1, 0.5));
		item->set_icon(0, get_theme_icon("Folder", "EditorIcons"));

		item->add_button(1, get_theme_icon("Add", "EditorIcons"), 0, false, "Add Sub-tag");

		_create_tree_items(p_tree, item, full_path, p_tags, p_type, p_search);
	}
}

void AbilitySystemTagsPanel::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY:
		case NOTIFICATION_THEME_CHANGED: {
			if (add_tag_button) {
				add_tag_button->set_button_icon(get_theme_icon("Add", "EditorIcons"));
			}
			update_tags();
		} break;
	}
}

AbilitySystemTagsPanel::AbilitySystemTagsPanel() {
	set_name("Ability System Tags");
	set_v_size_flags(SIZE_EXPAND_FILL);

	MarginContainer *main_margin = memnew(MarginContainer);
	main_margin->set_anchors_and_offsets_preset(PRESET_FULL_RECT);
	main_margin->set_v_size_flags(SIZE_EXPAND_FILL);
	main_margin->add_theme_constant_override("margin_left", 8);
	main_margin->add_theme_constant_override("margin_top", 8);
	main_margin->add_theme_constant_override("margin_right", 8);
	main_margin->add_theme_constant_override("margin_bottom", 8);
	add_child(main_margin);

	VBoxContainer *main_vbox = memnew(VBoxContainer);
	main_vbox->set_v_size_flags(SIZE_EXPAND_FILL);
	main_vbox->add_theme_constant_override("separation", 8);
	main_margin->add_child(main_vbox);

	HBoxContainer *search_hbc = memnew(HBoxContainer);
	main_vbox->add_child(search_hbc);

	search_edit = memnew(LineEdit);
	search_edit->set_h_size_flags(SIZE_EXPAND_FILL);
	search_edit->set_placeholder("Filter Tags (search branches)");
	search_edit->set_clear_button_enabled(true);
	search_hbc->add_child(search_edit);
	search_edit->connect("text_changed", callable_mp(this, &AbilitySystemTagsPanel::_on_search_changed));

	HBoxContainer *add_hbc = memnew(HBoxContainer);
	main_vbox->add_child(add_hbc);

	add_tag_edit = memnew(LineEdit);
	add_tag_edit->set_h_size_flags(SIZE_EXPAND_FILL);
	add_tag_edit->set_placeholder("Add tag to active tab (e.g., branch.subbranch.tag)");
	add_tag_edit->set_clear_button_enabled(true);
	add_tag_edit->set_keep_editing_on_text_submit(true);
	add_hbc->add_child(add_tag_edit);
	add_tag_edit->connect("text_submitted", callable_mp(this, &AbilitySystemTagsPanel::_add_tag_text));

	add_tag_button = memnew(Button);
	add_tag_button->set_text("Add");
	add_hbc->add_child(add_tag_button);
	add_tag_button->connect("pressed", callable_mp(this, &AbilitySystemTagsPanel::_add_tag));

	main_vbox->add_child(memnew(HSeparator));

	tabs = memnew(TabContainer);
	tabs->set_v_size_flags(SIZE_EXPAND_FILL);
	tabs->set_clip_tabs(false);
	main_vbox->add_child(tabs);

	auto create_tree = [&](const String &p_name) -> Tree * {
		Tree *t = memnew(Tree);
		t->set_name(p_name);
		t->set_v_size_flags(SIZE_EXPAND_FILL);
		t->set_hide_root(true);
		t->set_columns(2);
		t->set_column_expand(0, true);
		t->set_column_expand(1, false);
		t->set_column_custom_minimum_width(1, 64);
		t->add_theme_constant_override("h_separation", 8);
		t->connect("button_clicked", callable_mp(this, &AbilitySystemTagsPanel::_tag_removed));
		t->connect("button_clicked", callable_mp(this, &AbilitySystemTagsPanel::_add_sub_tag));
		return t;
	};

	name_tags_tree = create_tree("Identity Tags (Name)");
	tabs->add_child(name_tags_tree);

	cond_tags_tree = create_tree("Status Tags (Conditional)");
	tabs->add_child(cond_tags_tree);

	if (AbilitySystem::get_singleton()) {
		AbilitySystem::get_singleton()->connect("tags_changed", callable_mp(this, &AbilitySystemTagsPanel::update_tags));
	}
}

} // namespace godot

/**************************************************************************/
/*  ability_system_tags_selector.cpp                                      */
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

#include "ability_system_tags_selector.h"

#include "core/object/callable_method_pointer.h"
#include "modules/ability_system/core/ability_system.h"
#include "scene/gui/box_container.h"
#include "scene/gui/button.h"
#include "scene/gui/dialogs.h"
#include "scene/gui/line_edit.h"
#include "scene/gui/tree.h"

void AbilitySystemTagsSelector::_edit_pressed() {
	_update_tree();
	dialog->popup_centered_ratio(0.4);
}

void AbilitySystemTagsSelector::_update_tree() {
	updating = true;
	tags_tree->clear();
	TreeItem *root = tags_tree->create_item();

	AbilitySystem *as = AbilitySystem::get_singleton();
	if (!as) {
		return;
	}

	TypedArray<StringName> current_tags = get_edited_object()->get(get_edited_property());
	TypedArray<StringName> registered = as->get_registered_tags();
	registered.sort();

	String filter = search_edit->get_text().to_lower();

	for (int i = 0; i < registered.size(); i++) {
		StringName t = registered[i];
		if (!filter.is_empty() && !String(t).to_lower().contains(filter)) {
			continue;
		}

		TreeItem *item = tags_tree->create_item(root);
		item->set_cell_mode(0, TreeItem::CELL_MODE_CHECK);
		item->set_text(0, String(t));
		item->set_editable(0, true);
		item->set_selectable(0, true);

		bool is_checked = false;
		for (int j = 0; j < current_tags.size(); j++) {
			if (StringName(current_tags[j]) == t) {
				is_checked = true;
				break;
			}
		}
		item->set_checked(0, is_checked);
		item->set_metadata(0, t);
	}
	updating = false;
}

void AbilitySystemTagsSelector::_tag_toggled() {
	if (updating) {
		return;
	}

	TreeItem *item = tags_tree->get_edited();
	if (!item) {
		return;
	}

	StringName tag = item->get_metadata(0);
	bool is_checked = item->is_checked(0);

	TypedArray<StringName> current_tags = get_edited_object()->get(get_edited_property());
	if (is_checked) {
		bool found = false;
		for (int i = 0; i < current_tags.size(); i++) {
			if (StringName(current_tags[i]) == tag) {
				found = true;
				break;
			}
		}
		if (!found) {
			current_tags.push_back(tag);
		}
	} else {
		for (int i = 0; i < current_tags.size(); i++) {
			if (StringName(current_tags[i]) == tag) {
				current_tags.remove_at(i);
				break;
			}
		}
	}

	emit_changed(get_edited_property(), current_tags);
	_update_button_text();
}

void AbilitySystemTagsSelector::_update_button_text() {
	TypedArray<StringName> current_tags = get_edited_object()->get(get_edited_property());
	if (current_tags.is_empty()) {
		edit_button->set_text(TTR("(Empty Tags)"));
	} else {
		edit_button->set_text(vformat(TTR("%d Tags Selected"), current_tags.size()));
	}
}

void AbilitySystemTagsSelector::_search_changed(const String &p_text) {
	_update_tree();
}

void AbilitySystemTagsSelector::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_edit_pressed"), &AbilitySystemTagsSelector::_edit_pressed);
	ClassDB::bind_method(D_METHOD("_tag_toggled"), &AbilitySystemTagsSelector::_tag_toggled);
	ClassDB::bind_method(D_METHOD("_search_changed"), &AbilitySystemTagsSelector::_search_changed);
}

void AbilitySystemTagsSelector::update_property() {
	_update_button_text();
}

AbilitySystemTagsSelector::AbilitySystemTagsSelector() {
	edit_button = memnew(Button);
	edit_button->set_clip_text(true);
	edit_button->set_h_size_flags(SizeFlags::SIZE_EXPAND_FILL);
	add_child(edit_button);
	add_focusable(edit_button);
	edit_button->connect("pressed", callable_mp(this, &AbilitySystemTagsSelector::_edit_pressed));

	dialog = memnew(AcceptDialog);
	dialog->set_title(TTR("Select Ability System Tags"));
	add_child(dialog);

	VBoxContainer *vbox = memnew(VBoxContainer);
	dialog->add_child(vbox);

	search_edit = memnew(LineEdit);
	search_edit->set_placeholder(TTR("Filter Tags..."));
	search_edit->set_clear_button_enabled(true);
	vbox->add_child(search_edit);
	search_edit->connect("text_changed", callable_mp(this, &AbilitySystemTagsSelector::_search_changed));

	tags_tree = memnew(Tree);
	tags_tree->set_v_size_flags(SizeFlags::SIZE_EXPAND_FILL);
	tags_tree->set_columns(1);
	tags_tree->set_column_expand(0, true);
	tags_tree->set_hide_root(true);
	vbox->add_child(tags_tree);
	tags_tree->connect("item_edited", callable_mp(this, &AbilitySystemTagsSelector::_tag_toggled));
}

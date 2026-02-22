/**************************************************************************/
/*  ability_system_editor_plugin.cpp                                      */
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

#include "ability_system_editor_plugin.h"
#include "ability_system_tags_editor.h"
#include "ability_system_tags_selector.h"

#define CORE_OBJECT_CALLABLE_METHOD_POINTER_H

#include "core/object/callable_method_pointer.h"
#include "editor/inspector/editor_inspector.h"
#include "editor/inspector/editor_properties.h"
#include "editor/plugins/editor_plugin.h"
#include "editor/settings/project_settings_editor.h"
#include "modules/ability_system/core/ability_system.h"
#include "modules/ability_system/resources/ability_system_ability.h"
#include "modules/ability_system/resources/ability_system_ability_container.h"
#include "modules/ability_system/resources/ability_system_cue.h"
#include "modules/ability_system/resources/ability_system_effect.h"
#include "modules/ability_system/scene/ability_system_component.h"
#include "scene/gui/label.h"
#include "scene/gui/line_edit.h"
#include "scene/gui/option_button.h"

class EditorPropertyResourceName : public EditorProperty {
	GDCLASS(EditorPropertyResourceName, EditorProperty);

	LineEdit *edit = nullptr;
	Label *warning_label = nullptr;
	bool updating = false;

	void _text_changed(const String &p_text) {
		if (updating) {
			return;
		}
		emit_changed(get_edited_property(), p_text);
		_check_uniqueness(p_text);
	}

	void _check_uniqueness(const String &p_text) {
		if (p_text.is_empty()) {
			warning_label->hide();
			return;
		}

		AbilitySystem *as = AbilitySystem::get_singleton();
		if (as) {
			uint64_t owner = as->get_resource_name_owner(p_text);
			if (owner != 0 && owner != (uint64_t)get_edited_object()->get_instance_id()) {
				warning_label->show();
				return;
			}
		}
		warning_label->hide();
	}

protected:
	static void _bind_methods() {
		ClassDB::bind_method(D_METHOD("_text_changed", "text"), &EditorPropertyResourceName::_text_changed);
	}

public:
	virtual void update_property() override {
		Variant v = get_edited_object()->get(get_edited_property());
		String text = v.operator String();
		updating = true;
		if (edit->get_text() != text) {
			edit->set_text(text);
		}
		_check_uniqueness(text);
		updating = false;
	}

	EditorPropertyResourceName() {
		edit = memnew(LineEdit);
		edit->set_h_size_flags(SizeFlags::SIZE_EXPAND_FILL);
		add_child(edit);
		add_focusable(edit);
		edit->connect("text_changed", callable_mp(this, &EditorPropertyResourceName::_text_changed));

		warning_label = memnew(Label);
		warning_label->set_text(TTR("This name is already in use by another resource!"));
		warning_label->add_theme_color_override("font_color", Color(1, 0.9, 0.4));
		warning_label->set_autowrap_mode(TextServer::AUTOWRAP_WORD_SMART);
		warning_label->hide();
		add_child(warning_label);

		set_bottom_editor(warning_label);
	}
};

class EditorPropertyTagSelector : public EditorProperty {
	GDCLASS(EditorPropertyTagSelector, EditorProperty);

	OptionButton *options = nullptr;
	bool updating = false;

	void _option_selected(int p_idx) {
		if (updating) {
			return;
		}
		StringName tag = options->get_item_metadata(p_idx);
		emit_changed(get_edited_property(), tag);
	}

public:
	virtual void update_property() override {
		updating = true;
		StringName current_tag = get_edited_object()->get(get_edited_property());

		options->clear();
		options->add_item(TTR("(Empty)"));
		options->set_item_metadata(0, StringName());

		AbilitySystem *as = AbilitySystem::get_singleton();
		if (as) {
			TypedArray<StringName> tags = as->get_registered_tags();
			bool found = false;
			for (int i = 0; i < tags.size(); i++) {
				StringName t = tags[i];
				options->add_item(t);
				options->set_item_metadata(options->get_item_count() - 1, t);
				if (t == current_tag) {
					options->select(options->get_item_count() - 1);
					found = true;
				}
			}

			if (!found && current_tag != StringName()) {
				options->add_item(vformat(TTR("%s (Non-existent)"), current_tag));
				options->set_item_metadata(options->get_item_count() - 1, current_tag);
				options->select(options->get_item_count() - 1);
				options->set_item_icon(options->get_item_count() - 1, get_editor_theme_icon(SNAME("StatusError")));
			}
		}

		if (current_tag == StringName()) {
			options->select(0);
		}
		updating = false;
	}

	EditorPropertyTagSelector() {
		options = memnew(OptionButton);
		options->set_clip_text(true);
		add_child(options);
		add_focusable(options);
		options->connect("item_selected", callable_mp(this, &EditorPropertyTagSelector::_option_selected));

		AbilitySystem *as = AbilitySystem::get_singleton();
		if (as) {
			as->connect("tags_changed", callable_mp(this, &EditorPropertyTagSelector::update_property));
		}
	}
};

bool AbilitySystemInspectorPlugin::can_handle(Object *p_object) {
	return Object::cast_to<AbilitySystemComponent>(p_object) != nullptr ||
			Object::cast_to<AbilitySystemAbility>(p_object) != nullptr ||
			Object::cast_to<AbilitySystemEffect>(p_object) != nullptr ||
			Object::cast_to<AbilitySystemCue>(p_object) != nullptr ||
			Object::cast_to<AbilitySystemAbilityContainer>(p_object) != nullptr;
}

bool AbilitySystemInspectorPlugin::parse_property(Object *p_object, const Variant::Type p_type, const String &p_path, const PropertyHint p_hint, const String &p_hint_text, const BitField<PropertyUsageFlags> p_usage, const bool p_wide) {
	if (p_path.ends_with("_name")) {
		EditorPropertyResourceName *property_editor = memnew(EditorPropertyResourceName);
		add_property_editor(p_path, property_editor);
		return true;
	}

	if (p_path.ends_with("_tags")) {
		AbilitySystemTagsSelector *property_editor = memnew(AbilitySystemTagsSelector);
		add_property_editor(p_path, property_editor);
		return true;
	}

	if (p_path.ends_with("_tag") || (p_path.contains("/") && p_path.get_slice("/", 0).ends_with("_tags"))) {
		EditorPropertyTagSelector *editor = memnew(EditorPropertyTagSelector);
		add_property_editor(p_path, editor);
		return true;
	}

	return false;
}

AbilitySystemEditorPlugin::AbilitySystemEditorPlugin() {
	Ref<AbilitySystemInspectorPlugin> inspector_plugin;
	inspector_plugin.instantiate();
	add_inspector_plugin(inspector_plugin);

	ProjectSettingsEditor *ps_editor = ProjectSettingsEditor::get_singleton();
	if (ps_editor) {
		TabContainer *tabs = ps_editor->get_tabs();
		for (int i = 0; i < tabs->get_child_count(); i++) {
			Node *c = tabs->get_child(i);
			if (c->get_name() == "Ability System Tags") {
				return; // Already added
			}
		}

		AbilitySystemTagsEditor *tags_editor = memnew(AbilitySystemTagsEditor);
		tags_editor->set_name("Ability System Tags");
		tabs->add_child(tags_editor);
		tabs->set_tab_title(tabs->get_tab_count() - 1, TTR("Ability System Tags"));
		tabs->move_child(tags_editor, 2);
	}
}

AbilitySystemEditorPlugin::~AbilitySystemEditorPlugin() {
}

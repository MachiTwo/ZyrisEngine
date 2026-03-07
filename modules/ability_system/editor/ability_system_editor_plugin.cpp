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

#ifdef ABILITY_SYSTEM_MODULE
#include "modules/ability_system/editor/ability_system_editor_plugin.h"
#include "modules/ability_system/editor/ability_system_inspector_plugin.h"
#include "modules/ability_system/editor/ability_system_tags_panel.h"
#elif defined(ABILITY_SYSTEM_GDEXTENSION)
#include "src/editor/ability_system_editor_plugin.h"
#include "src/editor/ability_system_inspector_plugin.h"
#include "src/editor/ability_system_tags_panel.h"
#endif

#ifdef ABILITY_SYSTEM_MODULE
#include "editor/settings/project_settings_editor.h"
#include "scene/gui/tab_container.h"
#elif defined(ABILITY_SYSTEM_GDEXTENSION)
#include <godot_cpp/classes/tab_container.hpp>
#endif

namespace godot {

void AbilitySystemEditorPlugin::_bind_methods() {
}

AbilitySystemEditorPlugin::AbilitySystemEditorPlugin() {
	Ref<AbilitySystemInspectorPlugin> inspector_plugin;
	inspector_plugin.instantiate();
	add_inspector_plugin(inspector_plugin);

#ifdef ABILITY_SYSTEM_MODULE
	ProjectSettingsEditor *ps_editor = ProjectSettingsEditor::get_singleton();
	if (ps_editor) {
		TabContainer *tabs = ps_editor->get_tabs();
		for (int i = 0; i < tabs->get_child_count(); i++) {
			Node *c = tabs->get_child(i);
			if (c->get_name() == "Ability System Tags") {
				return; // Already added
			}
		}

		AbilitySystemTagsPanel *tags_editor = memnew(AbilitySystemTagsPanel);
		tags_editor->set_name("Ability System Tags");
		tabs->add_child(tags_editor);
		tabs->set_tab_title(tabs->get_tab_count() - 1, TTR("Ability System Tags"));
		tabs->move_child(tags_editor, 2);
	}
#endif
}

AbilitySystemEditorPlugin::~AbilitySystemEditorPlugin() {
}

} //namespace godot

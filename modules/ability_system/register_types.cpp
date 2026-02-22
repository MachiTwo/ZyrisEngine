/**************************************************************************/
/*  register_types.cpp                                                    */
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

#include "register_types.h"

#include "core/config/engine.h"
#include "core/object/class_db.h"
#include "modules/ability_system/core/ability_system.h"
#include "modules/ability_system/core/ability_system_ability_spec.h"
#include "modules/ability_system/core/ability_system_cue_spec.h"
#include "modules/ability_system/core/ability_system_effect_spec.h"
#include "modules/ability_system/core/ability_system_magnitude_calculation.h"
#include "modules/ability_system/core/ability_system_tag_spec.h"
#include "modules/ability_system/core/ability_system_target_data.h"
#include "modules/ability_system/core/ability_system_task.h"
#include "modules/ability_system/resources/ability_system_ability.h"
#include "modules/ability_system/resources/ability_system_ability_container.h"
#include "modules/ability_system/resources/ability_system_attribute.h"
#include "modules/ability_system/resources/ability_system_attribute_set.h"
#include "modules/ability_system/resources/ability_system_cue.h"
#include "modules/ability_system/resources/ability_system_cue_animation.h"
#include "modules/ability_system/resources/ability_system_cue_audio.h"
#include "modules/ability_system/resources/ability_system_effect.h"
#include "modules/ability_system/scene/ability_system_component.h"

#ifdef TOOLS_ENABLED
#include "editor/editor_node.h"
#include "modules/ability_system/editor/ability_system_editor_plugin.h"
#include "modules/ability_system/editor/ability_system_tags_editor.h"
#include "modules/ability_system/editor/ability_system_tags_selector.h"
#endif

static AbilitySystem *ability_system_singleton = nullptr;

#ifdef TOOLS_ENABLED
void register_ability_system_script_templates();
#endif

void initialize_ability_system_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		ClassDB::register_class<AbilitySystem>();
		ClassDB::register_class<AbilitySystemTagSpec>();
		ClassDB::register_class<AbilitySystemAttribute>();
		ClassDB::register_class<AbilitySystemAttributeSet>();
		ClassDB::register_class<AbilitySystemAbilityContainer>();
		ClassDB::register_class<AbilitySystemEffect>();
		ClassDB::register_class<AbilitySystemEffectSpec>();
		ClassDB::register_class<AbilitySystemAbility>();
		ClassDB::register_class<AbilitySystemAbilitySpec>();
		ClassDB::register_class<AbilitySystemComponent>();
		ClassDB::register_class<AbilitySystemCue>();
		ClassDB::register_class<AbilitySystemCueAnimation>();
		ClassDB::register_class<AbilitySystemCueAudio>();
		ClassDB::register_class<AbilitySystemCueSpec>();
		ClassDB::register_class<AbilitySystemMagnitudeCalculation>();
		ClassDB::register_class<AbilitySystemTargetData>();
		ClassDB::register_class<AbilitySystemTask>();

		ability_system_singleton = memnew(AbilitySystem);
		Engine::get_singleton()->add_singleton(Engine::Singleton("AbilitySystem", AbilitySystem::get_singleton()));
	}
#ifdef TOOLS_ENABLED
	if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
		ClassDB::register_class<AbilitySystemEditorPlugin>();
		ClassDB::register_class<AbilitySystemTagsEditor>();
		ClassDB::register_class<AbilitySystemTagsSelector>();
		EditorPlugins::add_by_type<AbilitySystemEditorPlugin>();
		register_ability_system_script_templates();
	}
#endif
}

#ifdef TOOLS_ENABLED
#include "core/io/dir_access.h"
#include "core/io/file_access.h"
#include "editor/file_system/editor_paths.h"
#include "editor/settings/editor_settings.h"
#include "modules/ability_system/editor/script_templates/templates.gen.h"

void register_ability_system_script_templates() {
	if (!EditorPaths::get_singleton()) {
		return;
	}

	String templates_dir = EditorPaths::get_singleton()->get_script_templates_dir();

	Ref<DirAccess> da = DirAccess::create(DirAccess::ACCESS_FILESYSTEM); // Use filesystem access to ensure we can write
	if (da.is_null()) {
		return;
	}

	for (int i = 0; i < ABILITY_SYSTEM_TEMPLATES_ARRAY_SIZE; i++) {
		const AbilitySystemScriptTemplate &t = ABILITY_SYSTEM_TEMPLATES[i];
		String inherit = String(t.inherit);
		String name = String(t.name);
		String description = String(t.description);
		String content_str = String(t.content);

		String target_dir = templates_dir.path_join(inherit);

		if (!da->dir_exists(target_dir)) {
			da->make_dir_recursive(target_dir);
		}

		String extension = "gd"; // Default
		if (content_str.contains("using _BINDINGS_NAMESPACE_;")) {
			extension = "cs";
		}

		String filename = name.to_snake_case() + "." + extension;
		String full_path = target_dir.path_join(filename);

		if (!FileAccess::exists(full_path)) {
			Ref<FileAccess> f = FileAccess::open(full_path, FileAccess::WRITE);
			if (f.is_valid()) {
				// We need to restore indentation from _TS_
				String final_content = content_str.replace("_TS_", "\t");

				// Re-add meta information
				f->store_line(extension == "cs" ? "// meta-name: " + name : "# meta-name: " + name);
				f->store_line(extension == "cs" ? "// meta-description: " + description : "# meta-description: " + description);
				f->store_line("");
				f->store_string(final_content);
			}
		}
	}
}
#endif

void uninitialize_ability_system_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		if (ability_system_singleton) {
			memdelete(ability_system_singleton);
		}
	}
}

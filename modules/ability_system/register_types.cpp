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

#ifdef ABILITY_SYSTEM_MODULE
#include "core/config/engine.h"
#include "core/config/project_settings.h"
#include "core/os/os.h"

#include "modules/ability_system/core/ability_system.h"
#include "modules/ability_system/core/ability_system_ability_spec.h"
#include "modules/ability_system/core/ability_system_cue_spec.h"
#include "modules/ability_system/core/ability_system_effect_spec.h"
#include "modules/ability_system/core/ability_system_tag_spec.h"
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
#include "modules/ability_system/editor/ability_system_editor_plugin.h"
#include "modules/ability_system/editor/ability_system_editor_property.h"
#include "modules/ability_system/editor/ability_system_inspector_plugin.h"
#include "modules/ability_system/editor/ability_system_tags_panel.h"
#endif
#elif defined(ABILITY_SYSTEM_GDEXTENSION)
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "src/core/ability_system.h"
#include "src/core/ability_system_ability_spec.h"
#include "src/core/ability_system_cue_spec.h"
#include "src/core/ability_system_effect_spec.h"
#include "src/core/ability_system_tag_spec.h"
#include "src/core/ability_system_task.h"
#include "src/resources/ability_system_ability.h"
#include "src/resources/ability_system_ability_container.h"
#include "src/resources/ability_system_attribute.h"
#include "src/resources/ability_system_attribute_set.h"
#include "src/resources/ability_system_cue.h"
#include "src/resources/ability_system_cue_animation.h"
#include "src/resources/ability_system_cue_audio.h"
#include "src/resources/ability_system_effect.h"
#include "src/scene/ability_system_component.h"
#ifdef TOOLS_ENABLED
#include "src/editor/ability_system_editor_plugin.h"
#include "src/editor/ability_system_editor_property.h"
#include "src/editor/ability_system_inspector_plugin.h"
#include "src/editor/ability_system_tags_panel.h"
#endif
#endif

#ifdef ABILITY_SYSTEM_TESTS_ENABLED
extern int ability_system_run_tests();
#endif

#ifdef ABILITY_SYSTEM_MODULE
static godot::AbilitySystem *ability_system_singleton = nullptr;

void initialize_ability_system_module(ModuleInitializationLevel p_level) {
#else
namespace godot {
static AbilitySystem *ability_system_singleton = nullptr;

void initialize_ability_system_module(ModuleInitializationLevel p_level) {
#endif
	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		GDREGISTER_CLASS(godot::AbilitySystem);
		GDREGISTER_CLASS(godot::AbilitySystemTagSpec);
		GDREGISTER_CLASS(godot::AbilitySystemTask);
		GDREGISTER_CLASS(godot::AbilitySystemAbility);
		GDREGISTER_CLASS(godot::AbilitySystemAbilitySpec);
		GDREGISTER_CLASS(godot::AbilitySystemAbilityContainer);
		GDREGISTER_CLASS(godot::AbilitySystemAttribute);
		GDREGISTER_CLASS(godot::AbilitySystemAttributeSet);
		GDREGISTER_CLASS(godot::AbilitySystemEffect);
		GDREGISTER_CLASS(godot::AbilitySystemEffectSpec);
		GDREGISTER_CLASS(godot::AbilitySystemCue);
		GDREGISTER_CLASS(godot::AbilitySystemCueSpec);
		GDREGISTER_CLASS(godot::AbilitySystemCueAnimation);
		GDREGISTER_CLASS(godot::AbilitySystemCueAudio);
		GDREGISTER_CLASS(godot::AbilitySystemComponent);

		ability_system_singleton = memnew(godot::AbilitySystem);
#ifdef ABILITY_SYSTEM_MODULE
		Engine::get_singleton()->add_singleton(Engine::Singleton("AbilitySystem", ability_system_singleton));
#else
		Engine::get_singleton()->register_singleton("AbilitySystem", ability_system_singleton);
#endif

#ifdef ABILITY_SYSTEM_TESTS_ENABLED
		PackedStringArray args = OS::get_singleton()->get_cmdline_args();
		if (args.find("--run-ability-tests") != -1) {
			exit(ability_system_run_tests());
		}
#endif
	}

#ifdef TOOLS_ENABLED
	if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
		GDREGISTER_CLASS(godot::AbilitySystemInspectorPlugin);
		GDREGISTER_CLASS(godot::AbilitySystemEditorPropertySelector);
		GDREGISTER_CLASS(godot::AbilitySystemEditorPropertyName);
		GDREGISTER_CLASS(godot::AbilitySystemEditorPropertyTagSelector);
		GDREGISTER_CLASS(godot::AbilitySystemTagsPanel);

#ifdef ABILITY_SYSTEM_MODULE
		EditorPlugins::add_by_type<godot::AbilitySystemEditorPlugin>();
#endif
	}
#endif
}

void uninitialize_ability_system_module(ModuleInitializationLevel p_level) {
	if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
		if (ability_system_singleton) {
#ifdef ABILITY_SYSTEM_MODULE
			Engine::get_singleton()->remove_singleton("AbilitySystem");
#else
			Engine::get_singleton()->unregister_singleton("AbilitySystem");
#endif
			memdelete(ability_system_singleton);
			ability_system_singleton = nullptr;
		}
	}
}

#ifndef ABILITY_SYSTEM_MODULE
} // namespace godot
#endif

#ifdef ABILITY_SYSTEM_GDEXTENSION
extern "C" {
GDExtensionBool GDE_EXPORT ability_system_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, const GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization) {
	godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

	init_obj.register_initializer(godot::initialize_ability_system_module);
	init_obj.register_terminator(godot::uninitialize_ability_system_module);
	init_obj.set_minimum_library_initialization_level(godot::MODULE_INITIALIZATION_LEVEL_SCENE);

	return init_obj.init();
}
}
#endif

/**************************************************************************/
/*  ability_system_inspector_plugin.cpp                                   */
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
#include "modules/ability_system/editor/ability_system_inspector_plugin.h"
#include "modules/ability_system/editor/ability_system_editor_property.h"
#include "modules/ability_system/resources/ability_system_ability.h"
#include "modules/ability_system/resources/ability_system_ability_container.h"
#include "modules/ability_system/resources/ability_system_cue.h"
#include "modules/ability_system/resources/ability_system_effect.h"
#include "modules/ability_system/scene/ability_system_component.h"
#elif defined(ABILITY_SYSTEM_GDEXTENSION)
#include "src/editor/ability_system_editor_property.h"
#include "src/editor/ability_system_inspector_plugin.h"
#include "src/resources/ability_system_ability.h"
#include "src/resources/ability_system_ability_container.h"
#include "src/resources/ability_system_cue.h"
#include "src/resources/ability_system_effect.h"
#include "src/scene/ability_system_component.h"
#endif

#ifdef ABILITY_SYSTEM_MODULE
#include "editor/inspector/editor_inspector.h"
#else
#include <godot_cpp/classes/editor_inspector.hpp>
#endif

namespace godot {

#ifdef ABILITY_SYSTEM_MODULE
bool AbilitySystemInspectorPlugin::can_handle(Object *p_object) {
#else
bool AbilitySystemInspectorPlugin::_can_handle(Object *p_object) const {
#endif
	return Object::cast_to<AbilitySystemComponent>(p_object) != nullptr ||
			Object::cast_to<AbilitySystemAbility>(p_object) != nullptr ||
			Object::cast_to<AbilitySystemEffect>(p_object) != nullptr ||
			Object::cast_to<AbilitySystemCue>(p_object) != nullptr ||
			Object::cast_to<AbilitySystemAbilityContainer>(p_object) != nullptr;
}

void AbilitySystemInspectorPlugin::_bind_methods() {
}

#ifdef ABILITY_SYSTEM_MODULE
bool AbilitySystemInspectorPlugin::parse_property(Object *p_object, Variant::Type p_type, const String &p_path, PropertyHint p_hint, const String &p_hint_text, BitField<PropertyUsageFlags> p_usage, bool p_wide) {
#else
bool AbilitySystemInspectorPlugin::_parse_property(Object *p_object, const Variant::Type p_type, const String &p_path, const PropertyHint p_hint, const String &p_hint_text, const BitField<PropertyUsageFlags> p_usage, const bool p_wide) {
#endif
	if (p_path.ends_with("_name")) {
		AbilitySystemEditorPropertyName *property_editor = memnew(AbilitySystemEditorPropertyName);
		add_property_editor(p_path, property_editor);
		return true;
	}

	if (p_path.ends_with("_tags")) {
		AbilitySystemEditorPropertySelector *property_editor = memnew(AbilitySystemEditorPropertySelector);
		add_property_editor(p_path, property_editor);
		return true;
	}

	if (p_path.ends_with("_tag") || (p_path.contains("/") && p_path.get_slice("/", 0).ends_with("_tags"))) {
		AbilitySystemEditorPropertyTagSelector *property_editor = memnew(AbilitySystemEditorPropertyTagSelector);
		add_property_editor(p_path, property_editor);
		return true;
	}

	return false;
}

} // namespace godot

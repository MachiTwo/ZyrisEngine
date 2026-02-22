/**************************************************************************/
/*  ability_system_cue.cpp                                                */
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

#include "modules/ability_system/resources/ability_system_cue.h"
#include "modules/ability_system/core/ability_system.h"

#include "modules/ability_system/core/ability_system_cue_spec.h"
#include "modules/ability_system/scene/ability_system_component.h"

void AbilitySystemCue::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_cue_name", "name"), &AbilitySystemCue::set_cue_name);
	ClassDB::bind_method(D_METHOD("get_cue_name"), &AbilitySystemCue::get_cue_name);
	ClassDB::bind_method(D_METHOD("set_event_type", "type"), &AbilitySystemCue::set_event_type);
	ClassDB::bind_method(D_METHOD("get_event_type"), &AbilitySystemCue::get_event_type);
	ClassDB::bind_method(D_METHOD("set_cue_tag", "tag"), &AbilitySystemCue::set_cue_tag);
	ClassDB::bind_method(D_METHOD("get_cue_tag"), &AbilitySystemCue::get_cue_tag);
	ClassDB::bind_method(D_METHOD("set_node_name", "name"), &AbilitySystemCue::set_node_name);
	ClassDB::bind_method(D_METHOD("get_node_name"), &AbilitySystemCue::get_node_name);

	GDVIRTUAL_BIND(_on_execute, "spec");
	GDVIRTUAL_BIND(_on_active, "spec");
	GDVIRTUAL_BIND(_on_remove, "spec");

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "cue_name"), "set_cue_name", "get_cue_name");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "event_type", PROPERTY_HINT_ENUM, "OnExecute,OnActive,OnRemove"), "set_event_type", "get_event_type");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "cue_tag"), "set_cue_tag", "get_cue_tag");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "node_name"), "set_node_name", "get_node_name");

	BIND_ENUM_CONSTANT(ON_EXECUTE);
	BIND_ENUM_CONSTANT(ON_ACTIVE);
	BIND_ENUM_CONSTANT(ON_REMOVE);
}

void AbilitySystemCue::set_cue_name(const String &p_name) {
	if (cue_name == p_name) {
		return;
	}

	AbilitySystem *as = AbilitySystem::get_singleton();
	if (as) {
		if (!cue_name.is_empty()) {
			as->unregister_resource_name(cue_name);
		}
		as->register_resource_name(p_name, get_instance_id());
	}
	cue_name = p_name;
}

void AbilitySystemCue::execute(Ref<AbilitySystemCueSpec> p_spec) {
	// Try GDVirtual first (for custom scripts)
	switch (get_event_type()) {
		case ON_EXECUTE:
			GDVIRTUAL_CALL(_on_execute, p_spec);
			break;
		case ON_ACTIVE:
			GDVIRTUAL_CALL(_on_active, p_spec);
			break;
		case ON_REMOVE:
			GDVIRTUAL_CALL(_on_remove, p_spec);
			break;
	}
}

AbilitySystemCue::AbilitySystemCue() {
}

AbilitySystemCue::~AbilitySystemCue() {
}

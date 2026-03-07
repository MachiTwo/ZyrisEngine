/**************************************************************************/
/*  ability_system_cue_spec.cpp                                           */
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
#include "modules/ability_system/core/ability_system_cue_spec.h"
#include "modules/ability_system/core/ability_system_effect_spec.h"
#include "modules/ability_system/resources/ability_system_cue.h"
#include "modules/ability_system/scene/ability_system_component.h"
#elif defined(ABILITY_SYSTEM_GDEXTENSION)
#include "src/core/ability_system_cue_spec.h"
#include "src/core/ability_system_effect_spec.h"
#include "src/resources/ability_system_cue.h"
#include "src/scene/ability_system_component.h"
#endif

namespace godot {

void AbilitySystemCueSpec::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_cue", "cue"), &AbilitySystemCueSpec::set_cue);
	ClassDB::bind_method(D_METHOD("get_cue"), &AbilitySystemCueSpec::get_cue);

	ClassDB::bind_method(D_METHOD("set_effect_spec", "spec"), &AbilitySystemCueSpec::set_effect_spec);
	ClassDB::bind_method(D_METHOD("get_effect_spec"), &AbilitySystemCueSpec::get_effect_spec);

	ClassDB::bind_method(D_METHOD("set_magnitude", "magnitude"), &AbilitySystemCueSpec::set_magnitude);
	ClassDB::bind_method(D_METHOD("get_magnitude"), &AbilitySystemCueSpec::get_magnitude);

	ClassDB::bind_method(D_METHOD("set_source_asc", "asc"), &AbilitySystemCueSpec::set_source_asc);
	ClassDB::bind_method(D_METHOD("get_source_asc"), &AbilitySystemCueSpec::get_source_asc);
	ClassDB::bind_method(D_METHOD("set_target_asc", "asc"), &AbilitySystemCueSpec::set_target_asc);
	ClassDB::bind_method(D_METHOD("get_target_asc"), &AbilitySystemCueSpec::get_target_asc);

	ClassDB::bind_method(D_METHOD("set_extra_data", "data"), &AbilitySystemCueSpec::set_extra_data);
	ClassDB::bind_method(D_METHOD("get_extra_data"), &AbilitySystemCueSpec::get_extra_data);

	ClassDB::bind_method(D_METHOD("set_level", "level"), &AbilitySystemCueSpec::set_level);
	ClassDB::bind_method(D_METHOD("get_level"), &AbilitySystemCueSpec::get_level);

	ClassDB::bind_method(D_METHOD("set_target_node", "node"), &AbilitySystemCueSpec::set_target_node);
	ClassDB::bind_method(D_METHOD("get_target_node"), &AbilitySystemCueSpec::get_target_node);
	ClassDB::bind_method(D_METHOD("set_hit_position", "position"), &AbilitySystemCueSpec::set_hit_position);
	ClassDB::bind_method(D_METHOD("get_hit_position"), &AbilitySystemCueSpec::get_hit_position);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "cue", PROPERTY_HINT_RESOURCE_TYPE, "AbilitySystemCue"), "set_cue", "get_cue");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "effect_spec", PROPERTY_HINT_NONE, "AbilitySystemEffectSpec"), "set_effect_spec", "get_effect_spec");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "magnitude"), "set_magnitude", "get_magnitude");
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "extra_data"), "set_extra_data", "get_extra_data");
	ADD_PROPERTY(PropertyInfo(Variant::NIL, "hit_position", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_NIL_IS_VARIANT), "set_hit_position", "get_hit_position");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "target_node"), "set_target_node", "get_target_node");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "source_asc", PROPERTY_HINT_NONE, "AbilitySystemComponent"), "set_source_asc", "get_source_asc");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "target_asc", PROPERTY_HINT_NONE, "AbilitySystemComponent"), "set_target_asc", "get_target_asc");
}

void AbilitySystemCueSpec::init_from_effect(
		Ref<AbilitySystemCue> p_cue,
		AbilitySystemComponent *p_source,
		AbilitySystemComponent *p_target,
		Ref<AbilitySystemEffectSpec> p_effect_spec,
		float p_magnitude) {
	cue = p_cue;
	set_source_asc(p_source);
	set_target_asc(p_target);
	effect_spec = p_effect_spec;
	magnitude = p_magnitude;

	if (p_effect_spec.is_valid()) {
		set_hit_position(p_effect_spec->get_hit_position());
		set_target_node(p_effect_spec->get_target_node());
		set_level(p_effect_spec->get_level());
	}
}

void AbilitySystemCueSpec::init_manual(
		Ref<AbilitySystemCue> p_cue,
		AbilitySystemComponent *p_owner,
		const Dictionary &p_extra_data) {
	cue = p_cue;
	// For a manual call, the owner is both source and target.
	set_source_asc(p_owner);
	set_target_asc(p_owner);
	extra_data = p_extra_data;
}

void AbilitySystemCueSpec::set_source_asc(AbilitySystemComponent *p_asc) {
	if (p_asc) {
		source_id = p_asc->get_instance_id();
	} else {
		source_id = ObjectID();
	}
}

AbilitySystemComponent *AbilitySystemCueSpec::get_source_asc() const {
	if (source_id.is_null()) {
		return nullptr;
	}
	return Object::cast_to<AbilitySystemComponent>(ObjectDB::get_instance(source_id));
}

void AbilitySystemCueSpec::set_target_asc(AbilitySystemComponent *p_asc) {
	if (p_asc) {
		target_id = p_asc->get_instance_id();
	} else {
		target_id = ObjectID();
	}
}

AbilitySystemComponent *AbilitySystemCueSpec::get_target_asc() const {
	if (target_id.is_null()) {
		return nullptr;
	}
	return Object::cast_to<AbilitySystemComponent>(ObjectDB::get_instance(target_id));
}

void AbilitySystemCueSpec::set_target_node(Object *p_node) {
	if (p_node) {
		target_node_id = p_node->get_instance_id();
	} else {
		target_node_id = ObjectID();
	}
}

Object *AbilitySystemCueSpec::get_target_node() const {
	if (target_node_id.is_null()) {
		return nullptr;
	}
	return ObjectDB::get_instance(target_node_id);
}

void AbilitySystemCueSpec::set_cue(Ref<AbilitySystemCue> p_cue) {
	cue = p_cue;
}

Ref<AbilitySystemCue> AbilitySystemCueSpec::get_cue() const {
	return Ref<AbilitySystemCue>(cue);
}

void AbilitySystemCueSpec::set_effect_spec(Ref<AbilitySystemEffectSpec> p_spec) {
	effect_spec = p_spec;
}

Ref<AbilitySystemEffectSpec> AbilitySystemCueSpec::get_effect_spec() const {
	return Ref<AbilitySystemEffectSpec>(effect_spec);
}

AbilitySystemCueSpec::AbilitySystemCueSpec() {
}

AbilitySystemCueSpec::~AbilitySystemCueSpec() {
}

} // namespace godot

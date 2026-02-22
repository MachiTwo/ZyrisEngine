/**************************************************************************/
/*  ability_system_effect.cpp                                             */
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

#include "modules/ability_system/resources/ability_system_effect.h"
#include "modules/ability_system/core/ability_system.h"
#include "modules/ability_system/core/ability_system_magnitude_calculation.h"

void AbilitySystemEffect::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_effect_name", "name"), &AbilitySystemEffect::set_effect_name);
	ClassDB::bind_method(D_METHOD("get_effect_name"), &AbilitySystemEffect::get_effect_name);
	ClassDB::bind_method(D_METHOD("set_effect_tag", "tag"), &AbilitySystemEffect::set_effect_tag);
	ClassDB::bind_method(D_METHOD("get_effect_tag"), &AbilitySystemEffect::get_effect_tag);
	ClassDB::bind_method(D_METHOD("set_duration_policy", "policy"), &AbilitySystemEffect::set_duration_policy);
	ClassDB::bind_method(D_METHOD("get_duration_policy"), &AbilitySystemEffect::get_duration_policy);
	ClassDB::bind_method(D_METHOD("set_duration_magnitude", "magnitude"), &AbilitySystemEffect::set_duration_magnitude);
	ClassDB::bind_method(D_METHOD("get_duration_magnitude"), &AbilitySystemEffect::get_duration_magnitude);
	ClassDB::bind_method(D_METHOD("set_stacking_policy", "policy"), &AbilitySystemEffect::set_stacking_policy);
	ClassDB::bind_method(D_METHOD("get_stacking_policy"), &AbilitySystemEffect::get_stacking_policy);
	ClassDB::bind_method(D_METHOD("set_period", "period"), &AbilitySystemEffect::set_period);
	ClassDB::bind_method(D_METHOD("get_period"), &AbilitySystemEffect::get_period);
	ClassDB::bind_method(D_METHOD("set_execute_periodic_tick_on_application", "exec"), &AbilitySystemEffect::set_execute_periodic_tick_on_application);
	ClassDB::bind_method(D_METHOD("get_execute_periodic_tick_on_application"), &AbilitySystemEffect::get_execute_periodic_tick_on_application);

	ClassDB::bind_method(D_METHOD("add_modifier", "attribute", "operation", "magnitude", "custom_magnitude"), &AbilitySystemEffect::add_modifier, DEFVAL(Variant()));
	ClassDB::bind_method(D_METHOD("get_modifier_count"), &AbilitySystemEffect::get_modifier_count);
	ClassDB::bind_method(D_METHOD("get_modifier_attribute", "index"), &AbilitySystemEffect::get_modifier_attribute);
	ClassDB::bind_method(D_METHOD("get_modifier_operation", "index"), &AbilitySystemEffect::get_modifier_operation);
	ClassDB::bind_method(D_METHOD("get_modifier_magnitude", "index"), &AbilitySystemEffect::get_modifier_magnitude);
	ClassDB::bind_method(D_METHOD("get_modifier_custom_magnitude", "index"), &AbilitySystemEffect::get_modifier_custom_magnitude);

	ClassDB::bind_method(D_METHOD("set_modifiers_count", "count"), &AbilitySystemEffect::set_modifiers_count);
	ClassDB::bind_method(D_METHOD("get_modifiers_count"), &AbilitySystemEffect::get_modifiers_count);

	ClassDB::bind_method(D_METHOD("set_granted_tags", "tags"), &AbilitySystemEffect::set_granted_tags);
	ClassDB::bind_method(D_METHOD("get_granted_tags"), &AbilitySystemEffect::get_granted_tags);
	ClassDB::bind_method(D_METHOD("set_blocked_tags", "tags"), &AbilitySystemEffect::set_blocked_tags);
	ClassDB::bind_method(D_METHOD("get_blocked_tags"), &AbilitySystemEffect::get_blocked_tags);
	ClassDB::bind_method(D_METHOD("set_removed_tags", "tags"), &AbilitySystemEffect::set_removed_tags);
	ClassDB::bind_method(D_METHOD("get_removed_tags"), &AbilitySystemEffect::get_removed_tags);
	ClassDB::bind_method(D_METHOD("set_cues", "cues"), &AbilitySystemEffect::set_cues);
	ClassDB::bind_method(D_METHOD("get_cues"), &AbilitySystemEffect::get_cues);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "effect_name"), "set_effect_name", "get_effect_name");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "effect_tag"), "set_effect_tag", "get_effect_tag");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "duration_policy", PROPERTY_HINT_ENUM, "Instant,Duration,Infinite"), "set_duration_policy", "get_duration_policy");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "duration_magnitude"), "set_duration_magnitude", "get_duration_magnitude");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "stacking_policy", PROPERTY_HINT_ENUM, "New Instance,Override,Stack Intensity,Stack Duration"), "set_stacking_policy", "get_stacking_policy");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "period"), "set_period", "get_period");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "execute_periodic_tick_on_application"), "set_execute_periodic_tick_on_application", "get_execute_periodic_tick_on_application");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "granted_tags", PROPERTY_HINT_ARRAY_TYPE, "StringName"), "set_granted_tags", "get_granted_tags");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "blocked_tags", PROPERTY_HINT_ARRAY_TYPE, "StringName"), "set_blocked_tags", "get_blocked_tags");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "removed_tags", PROPERTY_HINT_ARRAY_TYPE, "StringName"), "set_removed_tags", "get_removed_tags");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "cues", PROPERTY_HINT_RESOURCE_TYPE, "AbilitySystemCue"), "set_cues", "get_cues");

	ADD_PROPERTY(PropertyInfo(Variant::INT, "modifiers", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_ARRAY), "set_modifiers_count", "get_modifiers_count");

	BIND_ENUM_CONSTANT(INSTANT);
	BIND_ENUM_CONSTANT(DURATION);
	BIND_ENUM_CONSTANT(INFINITE);

	BIND_ENUM_CONSTANT(STACK_NEW_INSTANCE);
	BIND_ENUM_CONSTANT(STACK_OVERRIDE);
	BIND_ENUM_CONSTANT(STACK_INTENSITY);
	BIND_ENUM_CONSTANT(STACK_DURATION);

	BIND_ENUM_CONSTANT(ADD);
	BIND_ENUM_CONSTANT(MULTIPLY);
	BIND_ENUM_CONSTANT(DIVIDE);
	BIND_ENUM_CONSTANT(OVERRIDE);
}

void AbilitySystemEffect::add_modifier(const StringName &p_attr, ModifierOp p_op, float p_mag, Ref<AbilitySystemMagnitudeCalculation> p_custom_mag) {
	ModifierData md;
	md.attribute = p_attr;
	md.operation = p_op;
	md.magnitude = p_mag;
	md.custom_magnitude = p_custom_mag;
	modifiers.push_back(md);
}

Ref<AbilitySystemMagnitudeCalculation> AbilitySystemEffect::get_modifier_custom_magnitude(int p_idx) const {
	ERR_FAIL_INDEX_V(p_idx, modifiers.size(), nullptr);
	return modifiers[p_idx].custom_magnitude;
}

StringName AbilitySystemEffect::get_modifier_attribute(int p_idx) const {
	ERR_FAIL_INDEX_V(p_idx, modifiers.size(), StringName());
	return modifiers[p_idx].attribute;
}

AbilitySystemEffect::ModifierOp AbilitySystemEffect::get_modifier_operation(int p_idx) const {
	ERR_FAIL_INDEX_V(p_idx, modifiers.size(), ADD);
	return modifiers[p_idx].operation;
}

float AbilitySystemEffect::get_modifier_magnitude(int p_idx) const {
	ERR_FAIL_INDEX_V(p_idx, modifiers.size(), 0.0f);
	return modifiers[p_idx].magnitude;
}

void AbilitySystemEffect::set_modifiers_count(int p_count) {
	modifiers.resize(p_count);
}
int AbilitySystemEffect::get_modifiers_count() const {
	return modifiers.size();
}

bool AbilitySystemEffect::_set(const StringName &p_name, const Variant &p_value) {
	String prop_name = p_name;
	if (prop_name.begins_with("modifiers/")) {
		int index = prop_name.get_slicec('/', 1).to_int();
		String what = prop_name.get_slicec('/', 2);

		if (modifiers.size() <= index) {
			modifiers.resize(index + 1);
		}

		if (what == "attribute") {
			modifiers.write[index].attribute = p_value;
			return true;
		} else if (what == "operation") {
			modifiers.write[index].operation = (ModifierOp)(int)p_value;
			return true;
		} else if (what == "magnitude") {
			modifiers.write[index].magnitude = p_value;
			return true;
		} else if (what == "custom_magnitude") {
			modifiers.write[index].custom_magnitude = p_value;
			return true;
		}
	}
	return false;
}

bool AbilitySystemEffect::_get(const StringName &p_name, Variant &r_ret) const {
	String prop_name = p_name;
	if (prop_name.begins_with("modifiers/")) {
		int index = prop_name.get_slicec('/', 1).to_int();
		String what = prop_name.get_slicec('/', 2);

		if (index < 0 || index >= modifiers.size()) {
			return false;
		}

		if (what == "attribute") {
			r_ret = modifiers[index].attribute;
			return true;
		} else if (what == "operation") {
			r_ret = modifiers[index].operation;
			return true;
		} else if (what == "magnitude") {
			r_ret = modifiers[index].magnitude;
			return true;
		} else if (what == "custom_magnitude") {
			r_ret = modifiers[index].custom_magnitude;
			return true;
		}
	}
	return false;
}

void AbilitySystemEffect::_get_property_list(List<PropertyInfo> *p_list) const {
	for (int i = 0; i < modifiers.size(); i++) {
		p_list->push_back(PropertyInfo(Variant::STRING_NAME, vformat("modifiers/%d/attribute", i)));
		p_list->push_back(PropertyInfo(Variant::INT, vformat("modifiers/%d/operation", i), PROPERTY_HINT_ENUM, "Add,Multiply,Divide,Override"));
		p_list->push_back(PropertyInfo(Variant::FLOAT, vformat("modifiers/%d/magnitude", i)));
		p_list->push_back(PropertyInfo(Variant::OBJECT, vformat("modifiers/%d/custom_magnitude", i), PROPERTY_HINT_RESOURCE_TYPE, "AbilitySystemMagnitudeCalculation"));
	}
}

void AbilitySystemEffect::set_effect_name(const String &p_name) {
	if (effect_name == p_name) {
		return;
	}

	AbilitySystem *as = AbilitySystem::get_singleton();
	if (as) {
		if (!effect_name.is_empty()) {
			as->unregister_resource_name(effect_name);
		}
		as->register_resource_name(p_name, get_instance_id());
	}
	effect_name = p_name;
}

void AbilitySystemEffect::set_effect_tag(const StringName &p_tag) {
	effect_tag = p_tag;
	if (AbilitySystem::get_singleton()) {
		AbilitySystem::get_singleton()->register_tag(p_tag, get_instance_id());
	}

	// Propagate to cues
	for (int i = 0; i < cues.size(); i++) {
		Ref<AbilitySystemCue> cue = cues[i];
		if (cue.is_valid()) {
			cue->set_cue_tag(p_tag);
		}
	}
}

void AbilitySystemEffect::set_granted_tags(const TypedArray<StringName> &p_tags) {
	granted_tags = p_tags;
	if (AbilitySystem::get_singleton()) {
		for (int i = 0; i < p_tags.size(); i++) {
			AbilitySystem::get_singleton()->register_tag(p_tags[i]);
		}
	}
}

void AbilitySystemEffect::set_blocked_tags(const TypedArray<StringName> &p_tags) {
	blocked_tags = p_tags;
	if (AbilitySystem::get_singleton()) {
		for (int i = 0; i < p_tags.size(); i++) {
			AbilitySystem::get_singleton()->register_tag(p_tags[i]);
		}
	}
}

void AbilitySystemEffect::set_removed_tags(const TypedArray<StringName> &p_tags) {
	removed_tags = p_tags;
	if (AbilitySystem::get_singleton()) {
		for (int i = 0; i < p_tags.size(); i++) {
			AbilitySystem::get_singleton()->register_tag(p_tags[i]);
		}
	}
}

void AbilitySystemEffect::set_cues(const TypedArray<AbilitySystemCue> &p_cues) {
	cues = p_cues;
	if (effect_tag != StringName()) {
		for (int i = 0; i < cues.size(); i++) {
			Ref<AbilitySystemCue> cue = cues[i];
			if (cue.is_valid()) {
				cue->set_cue_tag(effect_tag);
			}
		}
	}
}

AbilitySystemEffect::AbilitySystemEffect() {
}

AbilitySystemEffect::~AbilitySystemEffect() {
}

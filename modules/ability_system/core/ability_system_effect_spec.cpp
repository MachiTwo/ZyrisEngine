/**************************************************************************/
/*  ability_system_effect_spec.cpp                                        */
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
#include "modules/ability_system/core/ability_system_effect_spec.h"
#include "modules/ability_system/core/ability_system_tag_spec.h"
#include "modules/ability_system/resources/ability_system_effect.h"
#include "modules/ability_system/scene/ability_system_component.h"
#elif defined(ABILITY_SYSTEM_GDEXTENSION)
#include "src/core/ability_system_effect_spec.h"
#include "src/core/ability_system_tag_spec.h"
#include "src/resources/ability_system_effect.h"
#include "src/scene/ability_system_component.h"
#endif

namespace godot {

void AbilitySystemEffectSpec::_bind_methods() {
	ClassDB::bind_method(D_METHOD("init", "effect", "level"), &AbilitySystemEffectSpec::init, DEFVAL(1.0f));
	ClassDB::bind_method(D_METHOD("get_effect"), &AbilitySystemEffectSpec::get_effect);
	ClassDB::bind_method(D_METHOD("get_duration_remaining"), &AbilitySystemEffectSpec::get_duration_remaining);
	ClassDB::bind_method(D_METHOD("set_duration_remaining", "value"), &AbilitySystemEffectSpec::set_duration_remaining);
	ClassDB::bind_method(D_METHOD("get_total_duration"), &AbilitySystemEffectSpec::get_total_duration);
	ClassDB::bind_method(D_METHOD("set_total_duration", "value"), &AbilitySystemEffectSpec::set_total_duration);
	ClassDB::bind_method(D_METHOD("set_magnitude", "name", "value"), &AbilitySystemEffectSpec::set_magnitude);
	ClassDB::bind_method(D_METHOD("get_magnitude", "name"), &AbilitySystemEffectSpec::get_magnitude);
	ClassDB::bind_method(D_METHOD("get_level"), &AbilitySystemEffectSpec::get_level);
	ClassDB::bind_method(D_METHOD("set_level", "level"), &AbilitySystemEffectSpec::set_level);
	ClassDB::bind_method(D_METHOD("get_source_component"), &AbilitySystemEffectSpec::get_source_component);
	ClassDB::bind_method(D_METHOD("get_target_component"), &AbilitySystemEffectSpec::get_target_component);

	ClassDB::bind_method(D_METHOD("get_stack_count"), &AbilitySystemEffectSpec::get_stack_count);
	ClassDB::bind_method(D_METHOD("set_stack_count", "count"), &AbilitySystemEffectSpec::set_stack_count);
	ClassDB::bind_method(D_METHOD("get_period_timer"), &AbilitySystemEffectSpec::get_period_timer);
	ClassDB::bind_method(D_METHOD("set_period_timer", "timer"), &AbilitySystemEffectSpec::set_period_timer);

	ClassDB::bind_method(D_METHOD("get_source_attribute_value", "attribute"), &AbilitySystemEffectSpec::get_source_attribute_value);
	ClassDB::bind_method(D_METHOD("get_target_attribute_value", "attribute"), &AbilitySystemEffectSpec::get_target_attribute_value);

	ClassDB::bind_method(D_METHOD("set_target_node", "node"), &AbilitySystemEffectSpec::set_target_node);
	ClassDB::bind_method(D_METHOD("get_target_node"), &AbilitySystemEffectSpec::get_target_node);
	ClassDB::bind_method(D_METHOD("set_hit_position", "position"), &AbilitySystemEffectSpec::set_hit_position);
	ClassDB::bind_method(D_METHOD("get_hit_position"), &AbilitySystemEffectSpec::get_hit_position);

	ADD_PROPERTY(PropertyInfo(Variant::NIL, "hit_position", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT | PROPERTY_USAGE_NIL_IS_VARIANT), "set_hit_position", "get_hit_position");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "target_node"), "set_target_node", "get_target_node");
}

void AbilitySystemEffectSpec::init(Ref<AbilitySystemEffect> p_effect, float p_level) {
	effect = p_effect;
	level = p_level;
	if (effect.is_valid()) {
		if (effect->get_duration_policy() == AbilitySystemEffect::POLICY_DURATION) {
			total_duration = effect->get_duration_magnitude();
			duration_remaining = total_duration;
		}
	}
}

void AbilitySystemEffectSpec::set_source_component(AbilitySystemComponent *p_comp) {
	if (p_comp) {
		source_id = p_comp->get_instance_id();
	} else {
		source_id = ObjectID();
	}
}

AbilitySystemComponent *AbilitySystemEffectSpec::get_source_component() const {
	if (source_id.is_null()) {
		return nullptr;
	}
	return Object::cast_to<AbilitySystemComponent>(ObjectDB::get_instance(source_id));
}

void AbilitySystemEffectSpec::set_target_component(AbilitySystemComponent *p_comp) {
	if (p_comp) {
		target_id = p_comp->get_instance_id();
	} else {
		target_id = ObjectID();
	}
}

AbilitySystemComponent *AbilitySystemEffectSpec::get_target_component() const {
	if (target_id.is_null()) {
		return nullptr;
	}
	return Object::cast_to<AbilitySystemComponent>(ObjectDB::get_instance(target_id));
}

void AbilitySystemEffectSpec::set_target_node(Object *p_node) {
	if (p_node) {
		target_node_id = p_node->get_instance_id();
	} else {
		target_node_id = ObjectID();
	}
}

Object *AbilitySystemEffectSpec::get_target_node() const {
	if (target_node_id.is_null()) {
		return nullptr;
	}
	return ObjectDB::get_instance(target_node_id);
}

float AbilitySystemEffectSpec::get_source_attribute_value(const StringName &p_attribute) const {
	AbilitySystemComponent *source = get_source_component();
	if (source) {
		return source->get_attribute_value_by_tag(p_attribute);
	}
	return 0.0f;
}

float AbilitySystemEffectSpec::get_target_attribute_value(const StringName &p_attribute) const {
	AbilitySystemComponent *target = get_target_component();
	if (target) {
		return target->get_attribute_value_by_tag(p_attribute);
	}
	return 0.0f;
}

AbilitySystemEffectSpec::AbilitySystemEffectSpec() {
}

AbilitySystemEffectSpec::~AbilitySystemEffectSpec() {
}

} // namespace godot

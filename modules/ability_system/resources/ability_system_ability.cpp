/**************************************************************************/
/*  ability_system_ability.cpp                                            */
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

#include "modules/ability_system/resources/ability_system_ability.h"
#include "modules/ability_system/core/ability_system.h"
#include "modules/ability_system/core/ability_system_ability_spec.h"
#include "modules/ability_system/core/ability_system_cue_spec.h"
#include "modules/ability_system/core/ability_system_effect_spec.h"
#include "modules/ability_system/core/ability_system_tag_spec.h"
#include "modules/ability_system/resources/ability_system_cue.h"
#include "modules/ability_system/resources/ability_system_effect.h"
#include "modules/ability_system/scene/ability_system_component.h"

void AbilitySystemAbility::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_ability_name", "name"), &AbilitySystemAbility::set_ability_name);
	ClassDB::bind_method(D_METHOD("get_ability_name"), &AbilitySystemAbility::get_ability_name);
	ClassDB::bind_method(D_METHOD("set_ability_tag", "tag"), &AbilitySystemAbility::set_ability_tag);
	ClassDB::bind_method(D_METHOD("get_ability_tag"), &AbilitySystemAbility::get_ability_tag);

	ClassDB::bind_method(D_METHOD("set_activation_owned_tags", "tags"), &AbilitySystemAbility::set_activation_owned_tags);
	ClassDB::bind_method(D_METHOD("get_activation_owned_tags"), &AbilitySystemAbility::get_activation_owned_tags);

	ClassDB::bind_method(D_METHOD("set_activation_required_tags", "tags"), &AbilitySystemAbility::set_activation_required_tags);
	ClassDB::bind_method(D_METHOD("get_activation_required_tags"), &AbilitySystemAbility::get_activation_required_tags);

	ClassDB::bind_method(D_METHOD("set_activation_blocked_tags", "tags"), &AbilitySystemAbility::set_activation_blocked_tags);
	ClassDB::bind_method(D_METHOD("get_activation_blocked_tags"), &AbilitySystemAbility::get_activation_blocked_tags);

	ClassDB::bind_method(D_METHOD("set_cues", "cues"), &AbilitySystemAbility::set_cues);
	ClassDB::bind_method(D_METHOD("get_cues"), &AbilitySystemAbility::get_cues);

	ClassDB::bind_method(D_METHOD("set_cost_effect", "effect"), &AbilitySystemAbility::set_cost_effect);
	ClassDB::bind_method(D_METHOD("get_cost_effect"), &AbilitySystemAbility::get_cost_effect);

	ClassDB::bind_method(D_METHOD("set_cooldown_effect", "effect"), &AbilitySystemAbility::set_cooldown_effect);
	ClassDB::bind_method(D_METHOD("get_cooldown_effect"), &AbilitySystemAbility::get_cooldown_effect);

	ClassDB::bind_method(D_METHOD("set_effect", "effect"), &AbilitySystemAbility::set_effect);
	ClassDB::bind_method(D_METHOD("get_effect"), &AbilitySystemAbility::get_effect);

	ClassDB::bind_method(D_METHOD("can_activate_ability", "owner", "spec"), &AbilitySystemAbility::can_activate_ability, DEFVAL(Variant()));
	ClassDB::bind_method(D_METHOD("activate_ability", "owner", "spec"), &AbilitySystemAbility::activate_ability, DEFVAL(Variant()));
	ClassDB::bind_method(D_METHOD("end_ability", "owner", "spec"), &AbilitySystemAbility::end_ability, DEFVAL(Variant()));

	GDVIRTUAL_BIND(_on_activate_ability, "owner", "spec");
	GDVIRTUAL_BIND(_on_can_activate_ability, "owner", "spec");
	GDVIRTUAL_BIND(_on_end_ability, "owner", "spec");

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "ability_name"), "set_ability_name", "get_ability_name");
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "ability_tag"), "set_ability_tag", "get_ability_tag");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "activation_owned_tags", PROPERTY_HINT_ARRAY_TYPE, "StringName"), "set_activation_owned_tags", "get_activation_owned_tags");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "activation_required_tags", PROPERTY_HINT_ARRAY_TYPE, "StringName"), "set_activation_required_tags", "get_activation_required_tags");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "activation_blocked_tags", PROPERTY_HINT_ARRAY_TYPE, "StringName"), "set_activation_blocked_tags", "get_activation_blocked_tags");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "cues", PROPERTY_HINT_ARRAY_TYPE, "AbilitySystemCue"), "set_cues", "get_cues");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "cost_effect", PROPERTY_HINT_RESOURCE_TYPE, "AbilitySystemEffect"), "set_cost_effect", "get_cost_effect");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "cooldown_effect", PROPERTY_HINT_RESOURCE_TYPE, "AbilitySystemEffect"), "set_cooldown_effect", "get_cooldown_effect");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "effect", PROPERTY_HINT_RESOURCE_TYPE, "AbilitySystemEffect"), "set_effect", "get_effect");
}

bool AbilitySystemAbility::can_activate_ability(AbilitySystemComponent *p_owner, Ref<AbilitySystemAbilitySpec> p_spec) const {
	ERR_FAIL_NULL_V(p_owner, false);

	// Tag requirements
	if (!p_owner->get_owned_tags()->has_all_tags(activation_required_tags)) {
		return false;
	}

	// Cooldown check
	if (cooldown_effect.is_valid()) {
		// Check if any active effect on owner has the cooldown tags
		TypedArray<StringName> cd_tags = cooldown_effect->get_granted_tags();
		if (p_owner->get_owned_tags()->has_any_tags(cd_tags)) {
			return false;
		}
	}

	// Cost check
	if (cost_effect.is_valid() && cost_effect->get_duration_policy() == AbilitySystemEffect::INSTANT) {
		// Use a spec to calculate correct magnitudes (including custom calculations)
		float level = p_spec.is_valid() ? (float)p_spec->get_level() : 1.0f;
		Ref<AbilitySystemEffectSpec> cost_spec = p_owner->make_outgoing_spec(cost_effect, level);

		for (int i = 0; i < cost_effect->get_modifier_count(); i++) {
			StringName attr = cost_effect->get_modifier_attribute(i);
			float current = p_owner->get_attribute_value(attr);

			// Calculate magnitude using the spec (handles scalable costs)
			float mag = cost_spec->calculate_modifier_magnitude(i);

			// We only check ADD operations for now, assuming costs are negative additions.
			if (cost_effect->get_modifier_operation(i) == AbilitySystemEffect::ADD) {
				if (current + mag < 0.0f) {
					return false;
				}
			}
		}
	}

	// Script override
	bool script_ret = true;
	if (GDVIRTUAL_CALL(_on_can_activate_ability, p_owner, p_spec, script_ret)) {
		if (!script_ret) {
			return false;
		}
	}

	return true;
}

void AbilitySystemAbility::activate_ability(AbilitySystemComponent *p_owner, Ref<AbilitySystemAbilitySpec> p_spec) {
	ERR_FAIL_NULL(p_owner);

	// Apply Cost
	if (cost_effect.is_valid()) {
		if (!can_activate_ability(p_owner, p_spec)) {
			return;
		}

		float level = p_spec.is_valid() ? (float)p_spec->get_level() : 1.0f;
		Ref<AbilitySystemEffectSpec> cost_spec = p_owner->make_outgoing_spec(cost_effect, level);

		p_owner->apply_effect_spec_to_self(cost_spec);
	}

	// Apply Cooldown
	if (cooldown_effect.is_valid()) {
		float level = p_spec.is_valid() ? (float)p_spec->get_level() : 1.0f;
		Ref<AbilitySystemEffectSpec> cd_spec = p_owner->make_outgoing_spec(cooldown_effect, level);
		p_owner->apply_effect_spec_to_self(cd_spec);
	}

	// Apply Main Effect
	if (effect.is_valid()) {
		float level = p_spec.is_valid() ? (float)p_spec->get_level() : 1.0f;
		Ref<AbilitySystemEffectSpec> effect_spec = p_owner->make_outgoing_spec(effect, level);
		p_owner->apply_effect_spec_to_self(effect_spec);
	}

	// Apply owned tags via ASC so tag_changed signal is emitted
	for (int i = 0; i < activation_owned_tags.size(); i++) {
		p_owner->add_tag(activation_owned_tags[i]);
	}

	// Trigger Cue Tag
	if (ability_tag != StringName()) {
		p_owner->execute_cue(ability_tag);
	}

	// Trigger Cue Resources
	for (int i = 0; i < cues.size(); i++) {
		Ref<AbilitySystemCue> cue = cues[i];
		if (cue.is_valid()) {
			Ref<AbilitySystemCueSpec> cue_spec;
			cue_spec.instantiate();
			cue_spec->init_manual(cue, p_owner, Dictionary());
			p_owner->_execute_cue_with_spec(get_ability_tag(), cue_spec);
		}
	}

	// Script override
	GDVIRTUAL_CALL(_on_activate_ability, p_owner, p_spec);
}

void AbilitySystemAbility::end_ability(AbilitySystemComponent *p_owner, Ref<AbilitySystemAbilitySpec> p_spec) {
	ERR_FAIL_NULL(p_owner);

	// Remove owned tags via ASC so tag_changed signal is emitted
	for (int i = 0; i < activation_owned_tags.size(); i++) {
		p_owner->remove_tag(activation_owned_tags[i]);
	}

	// Trigger Cue Tag
	if (ability_tag != StringName()) {
		p_owner->execute_cue(ability_tag);
	}

	// Trigger Cue Resources
	for (int i = 0; i < cues.size(); i++) {
		Ref<AbilitySystemCue> cue = cues[i];
		if (cue.is_valid()) {
			Ref<AbilitySystemCueSpec> cue_spec;
			cue_spec.instantiate();
			cue_spec->init_manual(cue, p_owner, Dictionary());
			p_owner->_execute_cue_with_spec(get_ability_tag(), cue_spec);
		}
	}

	// Script override
	GDVIRTUAL_CALL(_on_end_ability, p_owner, p_spec);
}

void AbilitySystemAbility::set_cost_effect(Ref<AbilitySystemEffect> p_effect) {
	cost_effect = p_effect;
	if (cost_effect.is_valid() && ability_tag != StringName()) {
		cost_effect->set_effect_tag(ability_tag);
	}
}

Ref<AbilitySystemEffect> AbilitySystemAbility::get_cost_effect() const {
	return cost_effect;
}

void AbilitySystemAbility::set_cooldown_effect(Ref<AbilitySystemEffect> p_effect) {
	cooldown_effect = p_effect;
	if (cooldown_effect.is_valid() && ability_tag != StringName()) {
		cooldown_effect->set_effect_tag(ability_tag);
	}
}

Ref<AbilitySystemEffect> AbilitySystemAbility::get_cooldown_effect() const {
	return cooldown_effect;
}

void AbilitySystemAbility::set_effect(Ref<AbilitySystemEffect> p_effect) {
	effect = p_effect;
	if (effect.is_valid() && ability_tag != StringName()) {
		effect->set_effect_tag(ability_tag);
	}
}

Ref<AbilitySystemEffect> AbilitySystemAbility::get_effect() const {
	return effect;
}

void AbilitySystemAbility::set_activation_owned_tags(const TypedArray<StringName> &p_tags) {
	activation_owned_tags = p_tags;
	if (AbilitySystem::get_singleton()) {
		for (int i = 0; i < p_tags.size(); i++) {
			AbilitySystem::get_singleton()->register_tag(p_tags[i]);
		}
	}
}

void AbilitySystemAbility::set_activation_required_tags(const TypedArray<StringName> &p_tags) {
	activation_required_tags = p_tags;
	if (AbilitySystem::get_singleton()) {
		for (int i = 0; i < p_tags.size(); i++) {
			AbilitySystem::get_singleton()->register_tag(p_tags[i]);
		}
	}
}

void AbilitySystemAbility::set_activation_blocked_tags(const TypedArray<StringName> &p_tags) {
	activation_blocked_tags = p_tags;
	if (AbilitySystem::get_singleton()) {
		for (int i = 0; i < p_tags.size(); i++) {
			AbilitySystem::get_singleton()->register_tag(p_tags[i]);
		}
	}
}

void AbilitySystemAbility::set_cues(const TypedArray<AbilitySystemCue> &p_cues) {
	cues = p_cues;
	if (ability_tag != StringName()) {
		for (int i = 0; i < cues.size(); i++) {
			Ref<AbilitySystemCue> cue = cues[i];
			if (cue.is_valid()) {
				cue->set_cue_tag(ability_tag);
			}
		}
	}
}

void AbilitySystemAbility::set_ability_name(const String &p_name) {
	if (ability_name == p_name) {
		return;
	}

	AbilitySystem *as = AbilitySystem::get_singleton();
	if (as) {
		if (!ability_name.is_empty()) {
			as->unregister_resource_name(ability_name);
		}
		as->register_resource_name(p_name, get_instance_id());
	}
	ability_name = p_name;
}

void AbilitySystemAbility::set_ability_tag(const StringName &p_tag) {
	ability_tag = p_tag;
	if (AbilitySystem::get_singleton()) {
		AbilitySystem::get_singleton()->register_tag(p_tag, get_instance_id());
	}

	// Propagate to internal effects
	if (effect.is_valid()) {
		effect->set_effect_tag(p_tag);
	}
	if (cost_effect.is_valid()) {
		cost_effect->set_effect_tag(p_tag);
	}
	if (cooldown_effect.is_valid()) {
		cooldown_effect->set_effect_tag(p_tag);
	}

	// Propagate to cues
	for (int i = 0; i < cues.size(); i++) {
		Ref<AbilitySystemCue> cue = cues[i];
		if (cue.is_valid()) {
			cue->set_cue_tag(p_tag);
		}
	}
}

AbilitySystemAbility::AbilitySystemAbility() {
}

AbilitySystemAbility::~AbilitySystemAbility() {
}

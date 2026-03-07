/**************************************************************************/
/*  ability_system_ability_spec.cpp                                       */
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
#include "modules/ability_system/core/ability_system_ability_spec.h"
#include "modules/ability_system/core/ability_system_effect_spec.h"
#include "modules/ability_system/scene/ability_system_component.h"
#elif defined(ABILITY_SYSTEM_GDEXTENSION)
#include "src/core/ability_system_ability_spec.h"
#include "src/core/ability_system_effect_spec.h"
#include "src/scene/ability_system_component.h"
#endif

namespace godot {

void AbilitySystemAbilitySpec::_bind_methods() {
	ClassDB::bind_method(D_METHOD("init", "ability", "level"), &AbilitySystemAbilitySpec::init, DEFVAL(1));
	ClassDB::bind_method(D_METHOD("get_ability"), &AbilitySystemAbilitySpec::get_ability);
	ClassDB::bind_method(D_METHOD("get_is_active"), &AbilitySystemAbilitySpec::get_is_active);
	ClassDB::bind_method(D_METHOD("set_is_active", "active"), &AbilitySystemAbilitySpec::set_is_active);
	ClassDB::bind_method(D_METHOD("get_level"), &AbilitySystemAbilitySpec::get_level);
	ClassDB::bind_method(D_METHOD("set_level", "level"), &AbilitySystemAbilitySpec::set_level);
	ClassDB::bind_method(D_METHOD("get_owner"), &AbilitySystemAbilitySpec::get_owner);

	ClassDB::bind_method(D_METHOD("get_cooldown_duration"), &AbilitySystemAbilitySpec::get_cooldown_duration);
	ClassDB::bind_method(D_METHOD("get_cooldown_remaining"), &AbilitySystemAbilitySpec::get_cooldown_remaining);
	ClassDB::bind_method(D_METHOD("is_on_cooldown"), &AbilitySystemAbilitySpec::is_on_cooldown);
	ClassDB::bind_method(D_METHOD("get_cost_amount", "attribute"), &AbilitySystemAbilitySpec::get_cost_amount);
}

void AbilitySystemAbilitySpec::init(Ref<AbilitySystemAbility> p_ability, int p_level) {
	ability = p_ability;
	level = p_level;
}

void AbilitySystemAbilitySpec::set_owner(AbilitySystemComponent *p_owner) {
	if (p_owner) {
		owner_id = p_owner->get_instance_id();
	} else {
		owner_id = ObjectID();
	}
}

AbilitySystemComponent *AbilitySystemAbilitySpec::get_owner() const {
	if (owner_id.is_null()) {
		return nullptr;
	}
	return Object::cast_to<AbilitySystemComponent>(ObjectDB::get_instance(owner_id));
}

float AbilitySystemAbilitySpec::get_cooldown_duration() const {
	if (ability.is_valid()) {
		return ability->get_cooldown_duration();
	}
	return 0.0f;
}

float AbilitySystemAbilitySpec::get_cooldown_remaining() const {
	AbilitySystemComponent *owner = get_owner();
	if (owner && ability.is_valid()) {
		return owner->get_cooldown_remaining(ability->get_ability_tag());
	}
	return 0.0f;
}

bool AbilitySystemAbilitySpec::is_on_cooldown() const {
	AbilitySystemComponent *owner = get_owner();
	if (owner && ability.is_valid()) {
		return owner->is_on_cooldown(ability->get_ability_tag());
	}
	return false;
}

float AbilitySystemAbilitySpec::get_cost_amount(const StringName &p_attribute) const {
	if (ability.is_valid()) {
		return ability->get_cost_amount(p_attribute);
	}
	return 0.0f;
}

void AbilitySystemAbilitySpec::add_active_effect(Ref<AbilitySystemEffectSpec> p_spec) {
	if (p_spec.is_valid() && active_effects.find(p_spec) == -1) {
		active_effects.push_back(p_spec);
	}
}

void AbilitySystemAbilitySpec::remove_active_effect(Ref<AbilitySystemEffectSpec> p_spec) {
	int idx = active_effects.find(p_spec);
	if (idx != -1) {
		active_effects.remove_at(idx);
	}
}

void AbilitySystemAbilitySpec::clear_active_effects() {
	active_effects.clear();
}

AbilitySystemAbilitySpec::AbilitySystemAbilitySpec() {
}

AbilitySystemAbilitySpec::~AbilitySystemAbilitySpec() {
}

} // namespace godot

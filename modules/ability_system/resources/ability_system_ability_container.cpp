/**************************************************************************/
/*  ability_system_ability_container.cpp                                  */
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

#include "modules/ability_system/resources/ability_system_ability_container.h"
#include "modules/ability_system/core/ability_system.h"

#include "modules/ability_system/resources/ability_system_ability.h"
#include "modules/ability_system/resources/ability_system_attribute_set.h"
#include "modules/ability_system/resources/ability_system_cue.h"
#include "modules/ability_system/resources/ability_system_effect.h"

void AbilitySystemAbilityContainer::_bind_methods() {
	// Abilities (catalog)
	ClassDB::bind_method(D_METHOD("set_abilities", "abilities"), &AbilitySystemAbilityContainer::set_abilities);
	ClassDB::bind_method(D_METHOD("get_abilities"), &AbilitySystemAbilityContainer::get_abilities);

	// Effects
	ClassDB::bind_method(D_METHOD("set_effects", "effects"), &AbilitySystemAbilityContainer::set_effects);
	ClassDB::bind_method(D_METHOD("get_effects"), &AbilitySystemAbilityContainer::get_effects);

	// Tags
	ClassDB::bind_method(D_METHOD("set_tags", "tags"), &AbilitySystemAbilityContainer::set_tags);
	ClassDB::bind_method(D_METHOD("get_tags"), &AbilitySystemAbilityContainer::get_tags);

	// AttributeSet
	ClassDB::bind_method(D_METHOD("set_attribute_set", "set"), &AbilitySystemAbilityContainer::set_attribute_set);
	ClassDB::bind_method(D_METHOD("get_attribute_set"), &AbilitySystemAbilityContainer::get_attribute_set);

	// Cues
	ClassDB::bind_method(D_METHOD("set_cues", "cues"), &AbilitySystemAbilityContainer::set_cues);
	ClassDB::bind_method(D_METHOD("get_cues"), &AbilitySystemAbilityContainer::get_cues);

	ClassDB::bind_method(D_METHOD("has_ability", "ability"), &AbilitySystemAbilityContainer::has_ability);
	ClassDB::bind_method(D_METHOD("has_effect", "effect"), &AbilitySystemAbilityContainer::has_effect);
	ClassDB::bind_method(D_METHOD("has_cue", "tag"), &AbilitySystemAbilityContainer::has_cue);

	// Properties
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "attribute_set", PROPERTY_HINT_RESOURCE_TYPE, "AbilitySystemAttributeSet"), "set_attribute_set", "get_attribute_set");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "abilities", PROPERTY_HINT_TYPE_STRING, vformat("%s/%s:%s", Variant::OBJECT, PROPERTY_HINT_RESOURCE_TYPE, "AbilitySystemAbility")), "set_abilities", "get_abilities");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "effects", PROPERTY_HINT_TYPE_STRING, vformat("%s/%s:%s", Variant::OBJECT, PROPERTY_HINT_RESOURCE_TYPE, "AbilitySystemEffect")), "set_effects", "get_effects");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "tags", PROPERTY_HINT_TYPE_STRING, vformat("%s/%s:%s", Variant::STRING_NAME, PROPERTY_HINT_NONE, "")), "set_tags", "get_tags");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "cues", PROPERTY_HINT_TYPE_STRING, vformat("%s/%s:%s", Variant::OBJECT, PROPERTY_HINT_RESOURCE_TYPE, "AbilitySystemCue")), "set_cues", "get_cues");
}

bool AbilitySystemAbilityContainer::has_ability(const Ref<AbilitySystemAbility> &p_ability) const {
	for (int i = 0; i < abilities.size(); i++) {
		if (abilities[i] == p_ability) {
			return true;
		}
	}
	// Check AttributeSet granted abilities as well
	if (attribute_set.is_valid()) {
		TypedArray<AbilitySystemAbility> granted = attribute_set->get_granted_abilities();
		for (int i = 0; i < granted.size(); i++) {
			if (granted[i] == p_ability) {
				return true;
			}
		}
	}
	return false;
}

bool AbilitySystemAbilityContainer::has_effect(const Ref<AbilitySystemEffect> &p_effect) const {
	for (int i = 0; i < effects.size(); i++) {
		if (effects[i] == p_effect) {
			return true;
		}
	}
	// Check if it's a cost or cooldown of an ability we have
	for (int i = 0; i < abilities.size(); i++) {
		Ref<AbilitySystemAbility> ability = abilities[i];
		if (ability.is_valid()) {
			if (ability->get_cost_effect() == p_effect || ability->get_cooldown_effect() == p_effect) {
				return true;
			}
		}
	}
	// Check AttributeSet granted abilities too
	if (attribute_set.is_valid()) {
		TypedArray<AbilitySystemAbility> granted = attribute_set->get_granted_abilities();
		for (int i = 0; i < granted.size(); i++) {
			Ref<AbilitySystemAbility> ability = granted[i];
			if (ability.is_valid()) {
				if (ability->get_cost_effect() == p_effect || ability->get_cooldown_effect() == p_effect) {
					return true;
				}
			}
		}
	}
	return false;
}

bool AbilitySystemAbilityContainer::has_cue(const StringName &p_tag) const {
	for (int i = 0; i < cues.size(); i++) {
		Ref<AbilitySystemCue> cue = cues[i];
		if (cue.is_valid() && cue->get_cue_tag() == p_tag) {
			return true;
		}
	}
	return false;
}

void AbilitySystemAbilityContainer::set_tags(const TypedArray<StringName> &p_tags) {
	tags = p_tags;
	if (AbilitySystem::get_singleton()) {
		for (int i = 0; i < p_tags.size(); i++) {
			AbilitySystem::get_singleton()->register_tag(p_tags[i]);
		}
	}
}

AbilitySystemAbilityContainer::AbilitySystemAbilityContainer() {
}

AbilitySystemAbilityContainer::~AbilitySystemAbilityContainer() {
}

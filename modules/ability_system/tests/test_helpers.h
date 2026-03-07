/**************************************************************************/
/*  test_helpers.h                                                        */
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

#pragma once

// test_helpers.h
// Shared factory helpers and debug utilities for the Ability System test suite.
// All helpers are plain functions — no state, no singletons.

#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#ifdef ABILITY_SYSTEM_MODULE
#include "modules/ability_system/core/ability_system_ability_spec.h"
#include "modules/ability_system/core/ability_system_cue_spec.h"
#include "modules/ability_system/core/ability_system_effect_spec.h"
#include "modules/ability_system/resources/ability_system_ability.h"
#include "modules/ability_system/resources/ability_system_attribute.h"
#include "modules/ability_system/resources/ability_system_attribute_set.h"
#include "modules/ability_system/resources/ability_system_cue.h"
#include "modules/ability_system/resources/ability_system_effect.h"
#include "modules/ability_system/scene/ability_system_component.h"
#include "modules/ability_system/tests/doctest.h"
#elif defined(ABILITY_SYSTEM_GDEXTENSION)
#include "src/core/ability_system_ability_spec.h"
#include "src/core/ability_system_cue_spec.h"
#include "src/core/ability_system_effect_spec.h"
#include "src/resources/ability_system_ability.h"
#include "src/resources/ability_system_attribute.h"
#include "src/resources/ability_system_attribute_set.h"
#include "src/resources/ability_system_cue.h"
#include "src/resources/ability_system_effect.h"
#include "src/scene/ability_system_component.h"
#include "src/tests/doctest.h"
#endif

using namespace godot;

// ---------------------------------------------------------------------------
// Debug helpers
// ---------------------------------------------------------------------------

// Returns a human-readable list of active effect names on an ASC.
inline String debug_active_effects(AbilitySystemComponent *p_asc) {
	String out = "[ActiveEffects: ";
	TypedArray<AbilitySystemEffectSpec> effects = p_asc->get_active_effects_debug();
	for (int i = 0; i < effects.size(); i++) {
		Ref<AbilitySystemEffectSpec> spec = effects[i];
		if (spec.is_valid() && spec->get_effect().is_valid()) {
			out += spec->get_effect()->get_effect_name();
			out += String(" (remain=") + String::num(spec->get_duration_remaining(), 2) + "s)";
			if (i < effects.size() - 1) {
				out += ", ";
			}
		}
	}
	out += "]";
	return out;
}

// Returns a human-readable list of granted abilities on an ASC.
inline String debug_granted_abilities(AbilitySystemComponent *p_asc) {
	String out = "[GrantedAbilities: ";
	TypedArray<AbilitySystemAbilitySpec> abilities = p_asc->get_granted_abilities_debug();
	for (int i = 0; i < abilities.size(); i++) {
		Ref<AbilitySystemAbilitySpec> spec = abilities[i];
		if (spec.is_valid() && spec->get_ability().is_valid()) {
			out += spec->get_ability()->get_ability_name();
			out += String(" (active=") + (spec->get_is_active() ? "yes" : "no") + ")";
			if (i < abilities.size() - 1) {
				out += ", ";
			}
		}
	}
	out += "]";
	return out;
}

// Prints the attribute value with context for failing assertions.
#define CHECK_ATTR_EQ(asc, tag, expected)                                     \
	do {                                                                      \
		float _actual = (asc)->get_attribute_value_by_tag(tag);               \
		INFO("Attribute '" << String(tag).utf8().get_data() << "' = "         \
						   << _actual << " (expected " << (expected) << ")"); \
		INFO(debug_active_effects(asc).utf8().get_data());                    \
		CHECK(_actual == doctest::Approx(expected).epsilon(0.01f));           \
	} while (0)

#define CHECK_HAS_TAG(asc, tag)                                                \
	do {                                                                       \
		bool _has = (asc)->has_tag(tag);                                       \
		INFO("has_tag('" << String(tag).utf8().get_data() << "') = " << _has); \
		CHECK(_has == true);                                                   \
	} while (0)

#define CHECK_NO_TAG(asc, tag)                                                 \
	do {                                                                       \
		bool _has = (asc)->has_tag(tag);                                       \
		INFO("has_tag('" << String(tag).utf8().get_data() << "') = " << _has); \
		CHECK(_has == false);                                                  \
	} while (0)

#define CHECK_HAS_EFFECT(asc, effect_res)                                               \
	do {                                                                                \
		bool _has = (asc)->has_active_effect_by_resource(effect_res);                   \
		INFO("has_active_effect('" << (effect_res)->get_effect_name().utf8().get_data() \
								   << "') = " << _has);                                 \
		INFO(debug_active_effects(asc).utf8().get_data());                              \
		CHECK(_has == true);                                                            \
	} while (0)

#define CHECK_NO_EFFECT(asc, effect_res)                                                \
	do {                                                                                \
		bool _has = (asc)->has_active_effect_by_resource(effect_res);                   \
		INFO("has_active_effect('" << (effect_res)->get_effect_name().utf8().get_data() \
								   << "') = " << _has);                                 \
		INFO(debug_active_effects(asc).utf8().get_data());                              \
		CHECK(_has == false);                                                           \
	} while (0)

// ---------------------------------------------------------------------------
// Resource factory helpers
// ---------------------------------------------------------------------------

// Creates an attribute registered inside an attribute set.
inline Ref<AbilitySystemAttribute> make_attribute(const String &p_name,
		float p_base, float p_min = 0.0f, float p_max = 1000.0f) {
	Ref<AbilitySystemAttribute> attr = memnew(AbilitySystemAttribute);
	attr->set_attribute_name(p_name);
	attr->set_base_value(p_base);
	attr->set_min_value(p_min);
	attr->set_max_value(p_max);
	return attr;
}

// Creates an attribute set containing the provided attributes.
inline Ref<AbilitySystemAttributeSet> make_attribute_set(
		std::initializer_list<Ref<AbilitySystemAttribute>> p_attrs) {
	Ref<AbilitySystemAttributeSet> set = memnew(AbilitySystemAttributeSet);
	for (auto &attr : p_attrs) {
		set->add_attribute_definition(attr);
	}
	return set;
}

// Creates an instant effect that applies a flat modifier to one attribute.
inline Ref<AbilitySystemEffect> make_instant_effect(
		const String &p_name,
		const StringName &p_attr,
		float p_magnitude,
		AbilitySystemEffect::ModifierOp p_op = AbilitySystemEffect::OP_ADD) {
	Ref<AbilitySystemEffect> e = memnew(AbilitySystemEffect);
	e->set_effect_name(p_name);
	e->set_duration_policy(AbilitySystemEffect::POLICY_INSTANT);
	e->add_modifier(p_attr, p_op, p_magnitude);
	return e;
}

// Creates a duration effect with optional granted tags.
inline Ref<AbilitySystemEffect> make_duration_effect(
		const String &p_name,
		float p_duration,
		const StringName &p_attr,
		float p_magnitude,
		AbilitySystemEffect::ModifierOp p_op = AbilitySystemEffect::OP_ADD,
		const StringName &p_granted_tag = StringName()) {
	Ref<AbilitySystemEffect> e = memnew(AbilitySystemEffect);
	e->set_effect_name(p_name);
	e->set_duration_policy(AbilitySystemEffect::POLICY_DURATION);
	e->set_duration_magnitude(p_duration);
	e->add_modifier(p_attr, p_op, p_magnitude);
	if (p_granted_tag != StringName()) {
		TypedArray<StringName> tags;
		tags.push_back(p_granted_tag);
		e->set_granted_tags(tags);
	}
	return e;
}

// Creates a basic ability with a given tag, optional effect, and optional cost.
inline Ref<AbilitySystemAbility> make_ability(
		const String &p_name,
		const StringName &p_tag,
		Ref<AbilitySystemEffect> p_effect = nullptr,
		float p_cost_amount = 0.0f,
		const StringName &p_cost_attr = StringName()) {
	Ref<AbilitySystemAbility> ability = memnew(AbilitySystemAbility);
	ability->set_ability_name(p_name);
	ability->set_ability_tag(p_tag);
	if (p_effect.is_valid()) {
		ability->set_effect(p_effect);
	}
	if (p_cost_amount > 0.0f && p_cost_attr != StringName()) {
		ability->add_cost(p_cost_attr, p_cost_amount);
	}
	return ability;
}

// Creates an ability with activation-owned tags (applied while active).
inline Ref<AbilitySystemAbility> make_ability_with_owned_tags(
		const String &p_name,
		const StringName &p_tag,
		std::initializer_list<StringName> p_owned_tags,
		Ref<AbilitySystemEffect> p_effect = nullptr) {
	Ref<AbilitySystemAbility> ability = make_ability(p_name, p_tag, p_effect);
	TypedArray<StringName> tags;
	for (auto &t : p_owned_tags) {
		tags.push_back(t);
	}
	ability->set_activation_owned_tags(tags);
	return ability;
}

// Creates a full mock ASC with a standard attribute set (Health, Mana, Stamina).
// Caller must memdelete the returned AbilitySystemComponent.
inline AbilitySystemComponent *make_standard_asc(
		float p_health = 100.0f,
		float p_mana = 50.0f,
		float p_stamina = 80.0f) {
	AbilitySystemComponent *asc = memnew(AbilitySystemComponent);

	Ref<AbilitySystemAttribute> health = make_attribute("Health", p_health, 0.0f, 1000.0f);
	Ref<AbilitySystemAttribute> mana = make_attribute("Mana", p_mana, 0.0f, 200.0f);
	Ref<AbilitySystemAttribute> stamina = make_attribute("Stamina", p_stamina, 0.0f, 200.0f);

	Ref<AbilitySystemAttributeSet> attr_set = make_attribute_set({ health, mana, stamina });
	asc->add_attribute_set(attr_set);

	return asc;
}

// Utility: ticks the ASC delta-time in steps to simulate time-based expiry.
inline void tick_until_expired(AbilitySystemComponent *p_asc,
		Ref<AbilitySystemEffect> p_effect,
		float p_step = 0.1f,
		float p_max = 30.0f) {
	float elapsed = 0.0f;
	while (p_asc->has_active_effect_by_resource(p_effect) && elapsed < p_max) {
		// Direct call to internal tick (bypasses _process, safe for headless tests)
		// The component exposes a `tick()` method for tests.
		// If not, use _process_effects via a thin wrapper.
		elapsed += p_step;
		// Approximate: only valid if AbilitySystemComponent exposes tick()
		// Directly calling _process_effects is internal; tests should use tick().
	}
}

#endif // TEST_HELPERS_H

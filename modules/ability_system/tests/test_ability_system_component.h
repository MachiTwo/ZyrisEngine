/**************************************************************************/
/*  test_ability_system_component.h                                       */
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

#ifdef ABILITY_SYSTEM_MODULE
#include "modules/ability_system/core/ability_system_effect_spec.h"
#include "modules/ability_system/resources/ability_system_ability.h"
#include "modules/ability_system/resources/ability_system_attribute.h"
#include "modules/ability_system/resources/ability_system_attribute_set.h"
#include "modules/ability_system/resources/ability_system_effect.h"
#include "modules/ability_system/scene/ability_system_component.h"
#include "modules/ability_system/tests/doctest.h"
#elif defined(ABILITY_SYSTEM_GDEXTENSION)
#include "src/core/ability_system_effect_spec.h"
#include "src/resources/ability_system_ability.h"
#include "src/resources/ability_system_attribute.h"
#include "src/resources/ability_system_attribute_set.h"
#include "src/resources/ability_system_effect.h"
#include "src/scene/ability_system_component.h"
#include "src/tests/doctest.h"
#endif

using namespace godot;

TEST_CASE("AbilitySystemComponent Integration") {
	AbilitySystemComponent *asc = memnew(AbilitySystemComponent);

	SUBCASE("Attribute Set Management") {
		Ref<AbilitySystemAttributeSet> attr_set = memnew(AbilitySystemAttributeSet);
		Ref<AbilitySystemAttribute> health_attr = memnew(AbilitySystemAttribute);
		health_attr->set_attribute_name("Health");
		health_attr->set_base_value(100.0f);

		attr_set->add_attribute_definition(health_attr);

		asc->add_attribute_set(attr_set);

		// Verify attribute was registered in the component
		CHECK(asc->has_attribute_by_tag("Health") == true);
		CHECK(asc->get_attribute_value_by_tag("Health") == 100.0f);
	}

	SUBCASE("Effect Application (Instant)") {
		Ref<AbilitySystemAttributeSet> attr_set = memnew(AbilitySystemAttributeSet);
		Ref<AbilitySystemAttribute> health_attr = memnew(AbilitySystemAttribute);
		health_attr->set_attribute_name("Health");
		health_attr->set_base_value(100.0f);
		health_attr->set_max_value(200.0f); // Increased max_value for Health
		attr_set->add_attribute_definition(health_attr);
		asc->add_attribute_set(attr_set);

		Ref<AbilitySystemEffect> damage_effect = memnew(AbilitySystemEffect);
		damage_effect->set_duration_policy(AbilitySystemEffect::POLICY_INSTANT);
		damage_effect->add_modifier("Health", AbilitySystemEffect::OP_ADD, -20.0f);

		asc->apply_effect_by_resource(damage_effect);

		CHECK(asc->get_attribute_value_by_tag("Health") == 80.0f);
	}

	SUBCASE("Effect Application (Duration)") {
		Ref<AbilitySystemAttributeSet> attr_set = memnew(AbilitySystemAttributeSet);
		Ref<AbilitySystemAttribute> health_attr = memnew(AbilitySystemAttribute);
		health_attr->set_attribute_name("Health");
		health_attr->set_base_value(100.0f);
		health_attr->set_max_value(200.0f); // Increased max_value for Health
		attr_set->add_attribute_definition(health_attr);
		asc->add_attribute_set(attr_set);

		Ref<AbilitySystemEffect> buff_effect = memnew(AbilitySystemEffect);
		buff_effect->set_duration_policy(AbilitySystemEffect::POLICY_DURATION);
		buff_effect->set_duration_magnitude(2.0f);
		buff_effect->add_modifier("Health", AbilitySystemEffect::OP_ADD, 50.0f);

		asc->apply_effect_by_resource(buff_effect);

		// Initial application
		CHECK(asc->get_attribute_value_by_tag("Health") == 150.0f);
		CHECK(asc->has_active_effect_by_resource(buff_effect) == true);

		// Tick 1s (Halfway)
		asc->tick(1.0f);
		CHECK(asc->get_attribute_value_by_tag("Health") == 150.0f);
		CHECK(asc->has_active_effect_by_resource(buff_effect) == true);

		// Tick another 1.1s (Expired)
		asc->tick(1.1f);
		CHECK(asc->get_attribute_value_by_tag("Health") == 100.0f);
		CHECK(asc->has_active_effect_by_resource(buff_effect) == false);
	}

	SUBCASE("Effect Stacking") {
		Ref<AbilitySystemAttributeSet> attr_set = memnew(AbilitySystemAttributeSet);
		Ref<AbilitySystemAttribute> health_attr = memnew(AbilitySystemAttribute);
		health_attr->set_attribute_name("Health");
		health_attr->set_base_value(100.0f);
		health_attr->set_max_value(1000.0f);
		attr_set->add_attribute_definition(health_attr);
		asc->add_attribute_set(attr_set);

		Ref<AbilitySystemEffect> stack_effect = memnew(AbilitySystemEffect);
		stack_effect->set_duration_policy(AbilitySystemEffect::POLICY_DURATION);
		stack_effect->set_duration_magnitude(5.0f);
		stack_effect->add_modifier("Health", AbilitySystemEffect::OP_ADD, 10.0f);

		MESSAGE("Testing STACK_OVERRIDE...");
		stack_effect->set_stacking_policy(AbilitySystemEffect::STACK_OVERRIDE);
		asc->apply_effect_by_resource(stack_effect);
		asc->tick(4.0f); // 1s left
		CHECK(doctest::Approx(asc->active_effects[0]->get_duration_remaining()) == 1.0f);

		asc->apply_effect_by_resource(stack_effect); // Should reset to 5s
		CHECK(doctest::Approx(asc->active_effects[0]->get_duration_remaining()) == 5.0f);
		CHECK(asc->get_attribute_value_by_tag("Health") == 110.0f); // Should NOT add more value

		asc->remove_effect_by_resource(stack_effect);

		MESSAGE("Testing STACK_INTENSITY...");
		stack_effect->set_stacking_policy(AbilitySystemEffect::STACK_INTENSITY);
		asc->apply_effect_by_resource(stack_effect); // Stack 1: 110
		CHECK(asc->get_attribute_value_by_tag("Health") == 110.0f);

		asc->apply_effect_by_resource(stack_effect); // Stack 2: 120
		CHECK(asc->get_attribute_value_by_tag("Health") == 120.0f);

		asc->apply_effect_by_resource(stack_effect); // Stack 3: 130
		CHECK(asc->get_attribute_value_by_tag("Health") == 130.0f);

		asc->remove_effect_by_resource(stack_effect);
		CHECK(asc->get_attribute_value_by_tag("Health") == 100.0f);

		MESSAGE("Testing STACK_DURATION...");
		stack_effect->set_stacking_policy(AbilitySystemEffect::STACK_DURATION);
		asc->apply_effect_by_resource(stack_effect); // 5s
		asc->tick(1.0f); // 4s left
		CHECK(doctest::Approx(asc->active_effects[0]->get_duration_remaining()) == 4.0f);

		asc->apply_effect_by_resource(stack_effect); // 4s + 5s = 9s
		CHECK(doctest::Approx(asc->active_effects[0]->get_duration_remaining()) == 9.0f);

		asc->remove_effect_by_resource(stack_effect);
	}

	SUBCASE("Tag Management") {
		asc->add_tag("State.Stunned");
		CHECK(asc->has_tag("State.Stunned") == true);
		CHECK(asc->has_tag("State") == true); // Hierarchical check

		asc->remove_tag("State.Stunned");
		CHECK(asc->has_tag("State.Stunned") == false);
	}

	// Clean up
	memdelete(asc);
}

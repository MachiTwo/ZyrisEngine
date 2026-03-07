/**************************************************************************/
/*  test_ability_system_attribute_set.h                                   */
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
#include "modules/ability_system/resources/ability_system_attribute.h"
#include "modules/ability_system/resources/ability_system_attribute_set.h"
#include "modules/ability_system/tests/doctest.h"
#elif defined(ABILITY_SYSTEM_GDEXTENSION)
#include "src/resources/ability_system_attribute.h"
#include "src/resources/ability_system_attribute_set.h"
#include "src/tests/doctest.h"
#endif

using namespace godot;

TEST_CASE("AbilitySystemAttributeSet Operations") {
	Ref<AbilitySystemAttributeSet> attr_set = memnew(AbilitySystemAttributeSet);

	SUBCASE("Base value initialization") {
		Ref<AbilitySystemAttribute> health_attr = memnew(AbilitySystemAttribute);
		health_attr->set_attribute_name("Health");
		health_attr->set_base_value(0.0f);
		attr_set->add_attribute_definition(health_attr);

		attr_set->set_attribute_base_value("Health", 100.0);
		CHECK(attr_set->get_attribute_base_value("Health") == 100.0);
		CHECK(attr_set->get_attribute_value("Health") == 100.0);
	}

	SUBCASE("Additive Modifiers") {
		Ref<AbilitySystemAttribute> health_attr = memnew(AbilitySystemAttribute);
		health_attr->set_attribute_name("Health");
		health_attr->set_base_value(100.0f);
		attr_set->add_attribute_definition(health_attr);

		attr_set->add_modifier("Health", 20.0); // Default type: Add
		CHECK(attr_set->get_attribute_value("Health") == 120.0);

		attr_set->remove_modifier("Health", 20.0);
		CHECK(attr_set->get_attribute_value("Health") == 100.0);
	}

	SUBCASE("Multiplicative Modifiers") {
		Ref<AbilitySystemAttribute> atk_attr = memnew(AbilitySystemAttribute);
		atk_attr->set_attribute_name("Attack");
		atk_attr->set_base_value(50.0f);
		attr_set->add_attribute_definition(atk_attr);

		// 50 * 1.5 = 75
		attr_set->add_modifier("Attack", 1.5, AbilitySystemAttributeSet::MODIFIER_MULTIPLY);
		CHECK(attr_set->get_attribute_value("Attack") == 75.0);
	}
}

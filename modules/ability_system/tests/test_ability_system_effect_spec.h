/**************************************************************************/
/*  test_ability_system_effect_spec.h                                     */
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
#include "modules/ability_system/resources/ability_system_effect.h"
#include "modules/ability_system/tests/doctest.h"
#elif defined(ABILITY_SYSTEM_GDEXTENSION)
#include "src/core/ability_system_effect_spec.h"
#include "src/resources/ability_system_effect.h"
#include "src/tests/doctest.h"
#endif

using namespace godot;

TEST_CASE("AbilitySystemEffectSpec Initialization and State") {
	Ref<AbilitySystemEffect> effect = memnew(AbilitySystemEffect);
	Ref<AbilitySystemEffectSpec> spec = memnew(AbilitySystemEffectSpec);

	SUBCASE("Initialization") {
		spec->init(effect, 5.0f); // Level 5
		CHECK(spec->get_effect() == effect);
		CHECK(spec->get_level() == 5.0f);
	}

	SUBCASE("Magnitudes management") {
		spec->set_magnitude("Damage", 50.0f);
		CHECK(spec->get_magnitude("Damage") == 50.0f);
		CHECK(spec->get_magnitude("NonExistent") == 0.0f);
	}

	SUBCASE("Duration tracking") {
		spec->set_total_duration(10.0f);
		spec->set_duration_remaining(10.0f);

		CHECK(spec->get_total_duration() == 10.0f);
		CHECK(spec->get_duration_remaining() == 10.0f);

		spec->set_duration_remaining(5.5f);
		CHECK(spec->get_duration_remaining() == 5.5f);
	}
}

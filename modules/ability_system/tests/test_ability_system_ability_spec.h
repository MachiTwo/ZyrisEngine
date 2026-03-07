/**************************************************************************/
/*  test_ability_system_ability_spec.h                                    */
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
#include "modules/ability_system/core/ability_system_ability_spec.h"
#include "modules/ability_system/resources/ability_system_ability.h"
#include "modules/ability_system/tests/doctest.h"
#elif defined(ABILITY_SYSTEM_GDEXTENSION)
#include "src/core/ability_system_ability_spec.h"
#include "src/resources/ability_system_ability.h"
#include "src/tests/doctest.h"
#endif

using namespace godot;

TEST_CASE("AbilitySystemAbilitySpec Lifecycle") {
	Ref<AbilitySystemAbility> ability = memnew(AbilitySystemAbility);
	Ref<AbilitySystemAbilitySpec> spec = memnew(AbilitySystemAbilitySpec);

	SUBCASE("Basic configuration") {
		spec->init(ability, 2); // Level 2
		CHECK(spec->get_ability() == ability);
		CHECK(spec->get_level() == 2);
		CHECK(spec->get_is_active() == false);
	}

	SUBCASE("Activation state") {
		spec->set_is_active(true);
		CHECK(spec->get_is_active() == true);
		spec->set_is_active(false);
		CHECK(spec->get_is_active() == false);
	}
}

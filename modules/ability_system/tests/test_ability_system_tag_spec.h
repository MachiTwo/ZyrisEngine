/**************************************************************************/
/*  test_ability_system_tag_spec.h                                        */
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
#include "modules/ability_system/core/ability_system.h"
#include "modules/ability_system/core/ability_system_tag_spec.h"
#include "modules/ability_system/tests/doctest.h"
#elif defined(ABILITY_SYSTEM_GDEXTENSION)
#include "src/core/ability_system.h"
#include "src/core/ability_system_tag_spec.h"
#include "src/tests/doctest.h"
#endif

using namespace godot;

TEST_CASE("AbilitySystemTagSpec Operations") {
	// We need a dummy AbilitySystem singleton if it's not and we are in a unit test environment
	// But since these are GDExtension classes, they might expect Godot to be initialized.
	// For pure logic tests, we might need to mock some parts.

	Ref<AbilitySystemTagSpec> spec = memnew(AbilitySystemTagSpec);

	SUBCASE("Basic add/remove") {
		CHECK(spec->add_tag("State.Dead") == true);
		CHECK(spec->has_tag("State.Dead", true) == true);

		// Adding same tag again (refcount increases)
		CHECK(spec->add_tag("State.Dead") == false);
		CHECK(spec->has_tag("State.Dead", true) == true);

		// Removing once should still keep it (count was 2)
		CHECK(spec->remove_tag("State.Dead") == false);
		CHECK(spec->has_tag("State.Dead", true) == true);

		// Removing second time should remove it
		CHECK(spec->remove_tag("State.Dead") == true);
		CHECK(spec->has_tag("State.Dead", true) == false);
	}

	SUBCASE("Hierarchical tag checks") {
		spec->add_tag("State.Dead.Bleeding");

		// Exact check
		CHECK(spec->has_tag("State.Dead.Bleeding", true) == true);
		CHECK(spec->has_tag("State.Dead", true) == false);

		// Hierarchical check
		CHECK(spec->has_tag("State.Dead", false) == true);
		CHECK(spec->has_tag("State", false) == true);

		CHECK(spec->has_tag("Other", false) == false);
	}

	SUBCASE("Multiple tags checks") {
		spec->add_tag("Ability.Fireball");
		spec->add_tag("Element.Fire");

		TypedArray<StringName> any_tags;
		any_tags.push_back(StringName("Element.Fire"));
		any_tags.push_back(StringName("Element.Ice"));
		CHECK(spec->has_any_tags(any_tags) == true);

		TypedArray<StringName> all_tags;
		all_tags.push_back(StringName("Ability.Fireball"));
		all_tags.push_back(StringName("Element.Fire"));
		CHECK(spec->has_all_tags(all_tags) == true);

		all_tags.push_back(StringName("Element.Ice"));
		CHECK(spec->has_all_tags(all_tags) == false);
	}
}

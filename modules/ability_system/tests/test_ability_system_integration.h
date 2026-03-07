/**************************************************************************/
/*  test_ability_system_integration.h                                     */
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

// test_ability_system_integration.h
#ifndef TEST_ABILITY_SYSTEM_INTEGRATION_H
#define TEST_ABILITY_SYSTEM_INTEGRATION_H

#ifdef ABILITY_SYSTEM_MODULE
#include "modules/ability_system/scene/ability_system_component.h"
#elif defined(ABILITY_SYSTEM_GDEXTENSION)
#include "src/scene/ability_system_component.h"
#endif

#include "test_helpers.h"

using namespace godot;

TEST_CASE("High-Density Ability System Integration") {
	AbilitySystemComponent *asc = make_standard_asc();

	// Definitions
	const StringName tag_jump = "Ability.Jump";
	const StringName tag_walk = "Ability.Walk";
	const StringName tag_state_jumping = "State.Jumping";
	const StringName tag_state_walking = "State.Walking";
	const StringName tag_cue_jump = "Cue.Jump";
	const StringName tag_cue_walk = "Cue.Walk";

	// 1. Create a "Jump" ability that grants "State.Jumping" and has a Cue (Magnitude 0 to not interfere with cost test)
	Ref<AbilitySystemEffect> jump_effect = make_duration_effect("JumpEffect", 1.0f, "Stamina", 0.0f, AbilitySystemEffect::OP_ADD, tag_state_jumping);
	Ref<AbilitySystemAbility> jump_ability = make_ability("Jump", tag_jump, jump_effect, 10.0f, "Stamina");

	// Add dummy cue
	Ref<AbilitySystemCue> jump_cue = memnew(AbilitySystemCue);
	jump_cue->set_cue_tag(tag_cue_jump);
	jump_cue->set_event_type(AbilitySystemCue::ON_ACTIVE);
	TypedArray<AbilitySystemCue> jump_cues;
	jump_cues.push_back(jump_cue);
	jump_ability->set_cues(jump_cues);

	asc->give_ability_by_resource(jump_ability);

	// 2. Create a "Walk" ability
	Ref<AbilitySystemEffect> walk_effect = make_duration_effect("WalkEffect", 5.0f, "Stamina", 0.0f, AbilitySystemEffect::OP_ADD, tag_state_walking);
	Ref<AbilitySystemAbility> walk_ability = make_ability("Walk", tag_walk, walk_effect);

	Ref<AbilitySystemCue> walk_cue = memnew(AbilitySystemCue);
	walk_cue->set_cue_tag(tag_cue_walk);
	walk_cue->set_event_type(AbilitySystemCue::ON_ACTIVE);
	TypedArray<AbilitySystemCue> walk_cues;
	walk_cues.push_back(walk_cue);
	walk_ability->set_cues(walk_cues);

	asc->give_ability_by_resource(walk_ability);

	SUBCASE("Scenario: Activate, Validate, Cancel, Swap") {
		MESSAGE("Starting activation of Jump...");
		bool jump_ok = asc->try_activate_ability_by_tag(tag_jump);
		CHECK(jump_ok == true);

		MESSAGE("Validating Jump state...");
		CHECK_HAS_TAG(asc, tag_state_jumping);
		CHECK_HAS_EFFECT(asc, jump_effect);
		// Check stamina cost (init 80, cost 10)
		CHECK_ATTR_EQ(asc, "Stamina", 70.0f);

		MESSAGE("Canceling Jump and swapping to Walk...");
		asc->cancel_ability_by_tag(tag_jump);
		CHECK_NO_TAG(asc, tag_state_jumping);
		CHECK_NO_EFFECT(asc, jump_effect);

		bool walk_ok = asc->try_activate_ability_by_tag(tag_walk);
		CHECK(walk_ok == true);
		CHECK_HAS_TAG(asc, tag_state_walking);
		CHECK_HAS_EFFECT(asc, walk_effect);
	}

	SUBCASE("Scenario: Parallel Abilities (Jump + Walk)") {
		MESSAGE("Activating Walk...");
		asc->try_activate_ability_by_tag(tag_walk);

		MESSAGE("Activating Jump while Walking...");
		bool jump_ok = asc->try_activate_ability_by_tag(tag_jump);
		CHECK(jump_ok == true);

		MESSAGE("Validating both states overlap...");
		CHECK_HAS_TAG(asc, tag_state_walking);
		CHECK_HAS_TAG(asc, tag_state_jumping);

		// Check stamina: 80 - 10 (jump cost) = 70.
		// (Walk has no cost in this mock setup yet)
		CHECK_ATTR_EQ(asc, "Stamina", 70.0f);

		MESSAGE("Validating effect stacking...");
		CHECK_HAS_EFFECT(asc, walk_effect);
		CHECK_HAS_EFFECT(asc, jump_effect);
	}

	SUBCASE("Scenario: Cost and Blocking") {
		asc->set_attribute_base_value_by_tag("Stamina", 5.0f);
		MESSAGE("Attempting Jump with low Stamina (cost=10)...");
		bool jump_fail = asc->try_activate_ability_by_tag(tag_jump);
		CHECK(jump_fail == false);
		CHECK_NO_TAG(asc, tag_state_jumping);

		MESSAGE("Adding a blocking tag: State.Dead");
		asc->add_tag("State.Dead");

		// Ability requirements: check if ability is blocked
		TypedArray<StringName> blocked;
		blocked.push_back(StringName("State.Dead"));
		jump_ability->set_activation_blocked_tags(blocked);

		asc->set_attribute_base_value_by_tag("Stamina", 100.0f);
		MESSAGE("Attempting Jump while Dead...");
		bool jump_blocked = asc->try_activate_ability_by_tag(tag_jump);
		CHECK(jump_blocked == false);
	}

	memdelete(asc);
}

#endif // TEST_ABILITY_SYSTEM_INTEGRATION_H

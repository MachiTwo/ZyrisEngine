/**************************************************************************/
/*  ability_system_effect_spec.cpp                                        */
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

#include "modules/ability_system/core/ability_system_effect_spec.h"
#include "modules/ability_system/core/ability_system_magnitude_calculation.h"
#include "modules/ability_system/core/ability_system_tag_spec.h"
#include "modules/ability_system/resources/ability_system_effect.h"
#include "modules/ability_system/scene/ability_system_component.h"

void AbilitySystemEffectSpec::_bind_methods() {
	ClassDB::bind_method(D_METHOD("init", "effect", "level"), &AbilitySystemEffectSpec::init, DEFVAL(1.0f));
	ClassDB::bind_method(D_METHOD("get_effect"), &AbilitySystemEffectSpec::get_effect);
	ClassDB::bind_method(D_METHOD("get_duration_remaining"), &AbilitySystemEffectSpec::get_duration_remaining);
	ClassDB::bind_method(D_METHOD("set_duration_remaining", "value"), &AbilitySystemEffectSpec::set_duration_remaining);
	ClassDB::bind_method(D_METHOD("get_total_duration"), &AbilitySystemEffectSpec::get_total_duration);
	ClassDB::bind_method(D_METHOD("set_magnitude", "name", "value"), &AbilitySystemEffectSpec::set_magnitude);
	ClassDB::bind_method(D_METHOD("get_magnitude", "name"), &AbilitySystemEffectSpec::get_magnitude);
	ClassDB::bind_method(D_METHOD("calculate_modifier_magnitude", "index"), &AbilitySystemEffectSpec::calculate_modifier_magnitude);
	ClassDB::bind_method(D_METHOD("get_level"), &AbilitySystemEffectSpec::get_level);
	ClassDB::bind_method(D_METHOD("set_level", "level"), &AbilitySystemEffectSpec::set_level);
	ClassDB::bind_method(D_METHOD("get_source_component"), &AbilitySystemEffectSpec::get_source_component);
	ClassDB::bind_method(D_METHOD("get_target_component"), &AbilitySystemEffectSpec::get_target_component);
}

void AbilitySystemEffectSpec::init(Ref<AbilitySystemEffect> p_effect, float p_level) {
	effect = p_effect;
	level = p_level;
	if (effect.is_valid()) {
		if (effect->get_duration_policy() == AbilitySystemEffect::DURATION) {
			total_duration = effect->get_duration_magnitude();
			duration_remaining = total_duration;
		}
	}
}

float AbilitySystemEffectSpec::calculate_modifier_magnitude(int p_modifier_idx) const {
	ERR_FAIL_COND_V(effect.is_null(), 0.0f);

	Ref<AbilitySystemMagnitudeCalculation> mmc = effect->get_modifier_custom_magnitude(p_modifier_idx);
	if (mmc.is_valid()) {
		float result = 0.0f;
		if (GDVIRTUAL_CALL_PTR(mmc.ptr(), _calculate_magnitude, Ref<AbilitySystemEffectSpec>(this), result)) {
			return result;
		}
	}

	return effect->get_modifier_magnitude(p_modifier_idx);
}

AbilitySystemEffectSpec::AbilitySystemEffectSpec() {
}

AbilitySystemEffectSpec::~AbilitySystemEffectSpec() {
}

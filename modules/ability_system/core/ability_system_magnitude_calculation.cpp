/**************************************************************************/
/*  ability_system_magnitude_calculation.cpp                              */
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

#include "modules/ability_system/core/ability_system_magnitude_calculation.h"
#include "modules/ability_system/core/ability_system_effect_spec.h"
#include "modules/ability_system/scene/ability_system_component.h"

void AbilitySystemMagnitudeCalculation::_bind_methods() {
	GDVIRTUAL_BIND(_calculate_magnitude, "spec");

	ClassDB::bind_method(D_METHOD("get_source_attribute_value", "spec", "attribute"), &AbilitySystemMagnitudeCalculation::get_source_attribute_value);
	ClassDB::bind_method(D_METHOD("get_target_attribute_value", "spec", "attribute"), &AbilitySystemMagnitudeCalculation::get_target_attribute_value);
}

float AbilitySystemMagnitudeCalculation::get_source_attribute_value(Ref<AbilitySystemEffectSpec> p_spec, const StringName &p_attribute) const {
	ERR_FAIL_COND_V(p_spec.is_null(), 0.0f);
	AbilitySystemComponent *source = p_spec->get_source_component();
	ERR_FAIL_NULL_V(source, 0.0f);
	return source->get_attribute_value(p_attribute);
}

float AbilitySystemMagnitudeCalculation::get_target_attribute_value(Ref<AbilitySystemEffectSpec> p_spec, const StringName &p_attribute) const {
	ERR_FAIL_COND_V(p_spec.is_null(), 0.0f);
	AbilitySystemComponent *target = p_spec->get_target_component();
	ERR_FAIL_NULL_V(target, 0.0f);
	return target->get_attribute_value(p_attribute);
}

AbilitySystemMagnitudeCalculation::AbilitySystemMagnitudeCalculation() {
}

AbilitySystemMagnitudeCalculation::~AbilitySystemMagnitudeCalculation() {
}

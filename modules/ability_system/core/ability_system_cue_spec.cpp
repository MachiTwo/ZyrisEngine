/**************************************************************************/
/*  ability_system_cue_spec.cpp                                           */
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

#include "modules/ability_system/core/ability_system_cue_spec.h"

#include "modules/ability_system/core/ability_system_effect_spec.h"
#include "modules/ability_system/resources/ability_system_cue.h"
#include "modules/ability_system/scene/ability_system_component.h"

void AbilitySystemCueSpec::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_cue", "cue"), &AbilitySystemCueSpec::set_cue);
	ClassDB::bind_method(D_METHOD("get_cue"), &AbilitySystemCueSpec::get_cue);

	ClassDB::bind_method(D_METHOD("get_source_asc"), &AbilitySystemCueSpec::get_source_asc);
	ClassDB::bind_method(D_METHOD("get_target_asc"), &AbilitySystemCueSpec::get_target_asc);

	ClassDB::bind_method(D_METHOD("set_effect_spec", "spec"), &AbilitySystemCueSpec::set_effect_spec);
	ClassDB::bind_method(D_METHOD("get_effect_spec"), &AbilitySystemCueSpec::get_effect_spec);

	ClassDB::bind_method(D_METHOD("set_magnitude", "magnitude"), &AbilitySystemCueSpec::set_magnitude);
	ClassDB::bind_method(D_METHOD("get_magnitude"), &AbilitySystemCueSpec::get_magnitude);

	ClassDB::bind_method(D_METHOD("set_extra_data", "data"), &AbilitySystemCueSpec::set_extra_data);
	ClassDB::bind_method(D_METHOD("get_extra_data"), &AbilitySystemCueSpec::get_extra_data);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "cue", PROPERTY_HINT_RESOURCE_TYPE, "AbilitySystemCue"), "set_cue", "get_cue");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "effect_spec", PROPERTY_HINT_RESOURCE_TYPE, "AbilitySystemEffectSpec"), "set_effect_spec", "get_effect_spec");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "magnitude"), "set_magnitude", "get_magnitude");
	ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "extra_data"), "set_extra_data", "get_extra_data");
}

void AbilitySystemCueSpec::init_from_effect(
		Ref<AbilitySystemCue> p_cue,
		AbilitySystemComponent *p_source,
		AbilitySystemComponent *p_target,
		Ref<AbilitySystemEffectSpec> p_effect_spec,
		float p_magnitude) {
	cue = p_cue;
	source_asc = p_source;
	target_asc = p_target;
	effect_spec = p_effect_spec;
	magnitude = p_magnitude;
}

void AbilitySystemCueSpec::init_manual(
		Ref<AbilitySystemCue> p_cue,
		AbilitySystemComponent *p_owner,
		const Dictionary &p_extra_data) {
	cue = p_cue;
	// For a manual call, the owner is both source and target.
	source_asc = p_owner;
	target_asc = p_owner;
	extra_data = p_extra_data;
}

void AbilitySystemCueSpec::set_cue(Ref<AbilitySystemCue> p_cue) {
	cue = p_cue;
}

Ref<AbilitySystemCue> AbilitySystemCueSpec::get_cue() const {
	return cue;
}

void AbilitySystemCueSpec::set_effect_spec(Ref<AbilitySystemEffectSpec> p_spec) {
	effect_spec = p_spec;
}

Ref<AbilitySystemEffectSpec> AbilitySystemCueSpec::get_effect_spec() const {
	return effect_spec;
}

AbilitySystemCueSpec::AbilitySystemCueSpec() {
}

AbilitySystemCueSpec::~AbilitySystemCueSpec() {
}

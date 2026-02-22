/**************************************************************************/
/*  ability_system_cue_spec.h                                             */
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

#include "core/io/resource.h"
#include "core/variant/dictionary.h"

class AbilitySystemCue;
class AbilitySystemComponent;
class AbilitySystemEffectSpec;

// Runtime context for a single Cue execution.
// Analogous to AbilitySystemEffectSpec / AbilitySystemAbilitySpec:
// wraps the definition Resource (AbilitySystemCue) with the execution
// context needed by the Cue's virtual callbacks.
class AbilitySystemCueSpec : public Resource {
	GDCLASS(AbilitySystemCueSpec, Resource);

protected:
	static void _bind_methods();

private:
	// The Cue definition that is being executed.
	Ref<AbilitySystemCue> cue;

	// ASC that originated this cue (e.g. the attacker). May be null for
	// cues triggered by non-ASC code.
	AbilitySystemComponent *source_asc = nullptr;

	// ASC that is the target / owner of this cue (e.g. the defender).
	AbilitySystemComponent *target_asc = nullptr;

	// The EffectSpec that triggered this cue, if any. Null when the cue
	// was dispatched directly from an ability or manual code.
	Ref<AbilitySystemEffectSpec> effect_spec;

	// Pre-calculated magnitude from the triggering effect (damage, heal…).
	// Zero when no effect is involved.
	float magnitude = 0.0f;

	// Arbitrary extra data passed at the call site.
	Dictionary extra_data;

public:
	// Initialize from an EffectSpec (called by apply_effect_spec_to_self).
	void init_from_effect(Ref<AbilitySystemCue> p_cue,
			AbilitySystemComponent *p_source,
			AbilitySystemComponent *p_target,
			Ref<AbilitySystemEffectSpec> p_effect_spec,
			float p_magnitude);

	// Initialize lightweight (called by manual execute_cue).
	void init_manual(Ref<AbilitySystemCue> p_cue,
			AbilitySystemComponent *p_owner,
			const Dictionary &p_extra_data);

	// --- Cue ---
	void set_cue(Ref<AbilitySystemCue> p_cue);
	Ref<AbilitySystemCue> get_cue() const;

	// --- Source / Target ---
	void set_source_asc(AbilitySystemComponent *p_asc) { source_asc = p_asc; }
	AbilitySystemComponent *get_source_asc() const { return source_asc; }

	void set_target_asc(AbilitySystemComponent *p_asc) { target_asc = p_asc; }
	AbilitySystemComponent *get_target_asc() const { return target_asc; }

	// --- Effect Spec ---
	void set_effect_spec(Ref<AbilitySystemEffectSpec> p_spec);
	Ref<AbilitySystemEffectSpec> get_effect_spec() const;

	// --- Magnitude ---
	void set_magnitude(float p_mag) { magnitude = p_mag; }
	float get_magnitude() const { return magnitude; }

	// --- Extra Data ---
	void set_extra_data(const Dictionary &p_data) { extra_data = p_data; }
	Dictionary get_extra_data() const { return extra_data; }

	AbilitySystemCueSpec();
	~AbilitySystemCueSpec();
};

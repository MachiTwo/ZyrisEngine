/**************************************************************************/
/*  ability_system_effect_spec.h                                          */
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
#include "modules/ability_system/resources/ability_system_effect.h"

class AbilitySystemComponent;

class AbilitySystemEffectSpec : public Resource {
	GDCLASS(AbilitySystemEffectSpec, Resource);

protected:
	static void _bind_methods();

private:
	Ref<AbilitySystemEffect> effect;
	float duration_remaining = 0.0f;
	float total_duration = 0.0f;
	float level = 1.0f;
	AbilitySystemComponent *source_component = nullptr;
	AbilitySystemComponent *target_component = nullptr;

	HashMap<StringName, float> set_magnitudes;

	// Stacking runtime state
	int stack_count = 1;

	// Periodic tick runtime state
	float period_timer = 0.0f;

public:
	void init(Ref<AbilitySystemEffect> p_effect, float p_level = 1.0f);
	Ref<AbilitySystemEffect> get_effect() const { return effect; }

	float get_duration_remaining() const { return duration_remaining; }
	void set_duration_remaining(float p_val) { duration_remaining = p_val; }

	float get_total_duration() const { return total_duration; }

	void set_magnitude(const StringName &p_name, float p_val) { set_magnitudes[p_name] = p_val; }
	float get_magnitude(const StringName &p_name) const { return set_magnitudes.has(p_name) ? set_magnitudes[p_name] : 0.0f; }

	void set_source_component(AbilitySystemComponent *p_comp) { source_component = p_comp; }
	AbilitySystemComponent *get_source_component() const { return source_component; }

	void set_target_component(AbilitySystemComponent *p_comp) { target_component = p_comp; }
	AbilitySystemComponent *get_target_component() const { return target_component; }

	float get_level() const { return level; }
	void set_level(float p_level) { level = p_level; }

	// Stacking interface
	int get_stack_count() const { return stack_count; }
	void set_stack_count(int p_count) { stack_count = p_count; }

	// Periodic tick interface
	float get_period_timer() const { return period_timer; }
	void set_period_timer(float p_timer) { period_timer = p_timer; }

	float calculate_modifier_magnitude(int p_modifier_idx) const;

	AbilitySystemEffectSpec();
	~AbilitySystemEffectSpec();
};

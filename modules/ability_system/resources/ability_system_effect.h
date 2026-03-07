/**************************************************************************/
/*  ability_system_effect.h                                               */
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
#include "modules/ability_system/resources/ability_system_cue.h"
#elif defined(ABILITY_SYSTEM_GDEXTENSION)
#include "src/resources/ability_system_cue.h"
#endif

#ifdef ABILITY_SYSTEM_MODULE
#include "core/io/resource.h"
#include "core/templates/vector.h"
#include "core/variant/typed_array.h"
#elif defined(ABILITY_SYSTEM_GDEXTENSION)
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#endif

namespace godot {

class AbilitySystemCue;
class AbilitySystemEffectSpec;

class AbilitySystemEffect : public Resource {
	GDCLASS(AbilitySystemEffect, Resource);

public:
	enum DurationPolicy {
		POLICY_INSTANT,
		POLICY_DURATION,
		POLICY_INFINITE
	};

	enum StackingPolicy {
		STACK_NEW_INSTANCE, // Each application is independent.
		STACK_OVERRIDE, // The new replaces the old (resets the timer).
		STACK_INTENSITY, // Increases the magnitude (adds to stack_count).
		STACK_DURATION // Extends the remaining duration.
	};

	enum ModifierOp {
		OP_ADD,
		OP_MULTIPLY,
		OP_DIVIDE,
		OP_OVERRIDE
	};

	struct Modifier {
		StringName attribute;
		ModifierOp operation = OP_ADD;
		float magnitude = 0.0f;
	};

protected:
	static void _bind_methods();
	bool _set(const StringName &p_name, const Variant &p_value);
	bool _get(const StringName &p_name, Variant &r_ret) const;
	void _get_property_list(List<PropertyInfo> *p_list) const;

private:
	String effect_name;
	StringName effect_tag;
	DurationPolicy duration_policy = POLICY_INSTANT;
	float duration_magnitude = 0.0f;
	bool use_custom_duration = false;

	StackingPolicy stacking_policy = STACK_NEW_INSTANCE;

	float period = 0.0f; // 0 means non-periodic.
	bool execute_periodic_tick_on_application = false;

	struct ModifierData {
		StringName attribute;
		ModifierOp operation;
		float magnitude;
		bool use_custom_magnitude = false;
	};
	Vector<ModifierData> modifiers;

	// Activation requirements
	TypedArray<StringName> activation_required_tags;
	TypedArray<StringName> activation_blocked_tags;

	TypedArray<StringName> granted_tags;
	TypedArray<StringName> blocked_tags;
	TypedArray<StringName> removed_tags;
	TypedArray<AbilitySystemCue> cues;

public:
	void set_effect_name(const String &p_name);
	String get_effect_name() const { return effect_name; }

	void set_effect_tag(const StringName &p_tag);
	StringName get_effect_tag() const { return effect_tag; }

	void set_duration_policy(DurationPolicy p_policy) { duration_policy = p_policy; }
	DurationPolicy get_duration_policy() const { return duration_policy; }

	void set_duration_magnitude(float p_mag) { duration_magnitude = p_mag; }
	float get_duration_magnitude() const { return duration_magnitude; }

	void set_use_custom_duration(bool p_use) { use_custom_duration = p_use; }
	bool get_use_custom_duration() const { return use_custom_duration; }

	void set_stacking_policy(StackingPolicy p_policy) { stacking_policy = p_policy; }
	StackingPolicy get_stacking_policy() const { return stacking_policy; }

	void set_period(float p_period) { period = p_period; }
	float get_period() const { return period; }

	void set_execute_periodic_tick_on_application(bool p_exec) { execute_periodic_tick_on_application = p_exec; }
	bool get_execute_periodic_tick_on_application() const { return execute_periodic_tick_on_application; }

	void add_modifier(const StringName &p_attr, ModifierOp p_op, float p_mag, bool p_custom = false);
	int get_modifier_count() const { return modifiers.size(); }
	StringName get_modifier_attribute(int p_idx) const;
	ModifierOp get_modifier_operation(int p_idx) const;
	float get_modifier_magnitude(int p_idx) const;
	bool is_modifier_custom(int p_idx) const;

	void set_granted_tags(const TypedArray<StringName> &p_tags);
	TypedArray<StringName> get_granted_tags() const { return granted_tags; }

	void set_blocked_tags(const TypedArray<StringName> &p_tags);
	TypedArray<StringName> get_blocked_tags() const { return blocked_tags; }

	void set_removed_tags(const TypedArray<StringName> &p_tags);
	TypedArray<StringName> get_removed_tags() const { return removed_tags; }

	void set_cues(const TypedArray<AbilitySystemCue> &p_cues);
	TypedArray<AbilitySystemCue> get_cues() const { return cues; }

	void set_modifiers_count(int p_count);
	int get_modifiers_count() const;

	void set_activation_required_tags(const TypedArray<StringName> &p_tags);
	TypedArray<StringName> get_activation_required_tags() const { return activation_required_tags; }

	void set_activation_blocked_tags(const TypedArray<StringName> &p_tags);
	TypedArray<StringName> get_activation_blocked_tags() const { return activation_blocked_tags; }

	AbilitySystemEffect();
	~AbilitySystemEffect();
};

} // namespace godot

VARIANT_ENUM_CAST(godot::AbilitySystemEffect::DurationPolicy);
VARIANT_ENUM_CAST(godot::AbilitySystemEffect::StackingPolicy);
VARIANT_ENUM_CAST(godot::AbilitySystemEffect::ModifierOp);

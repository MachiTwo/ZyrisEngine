/**************************************************************************/
/*  ability_system_ability.h                                              */
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
#include "modules/ability_system/resources/ability_system_effect.h"
#elif defined(ABILITY_SYSTEM_GDEXTENSION)
#include "src/resources/ability_system_effect.h"
#endif

#ifdef ABILITY_SYSTEM_MODULE
#include "core/io/resource.h"
#include "core/object/gdvirtual.gen.inc"
#include "core/object/object.h"
#include "core/variant/typed_array.h"
#include "core/variant/variant.h"
#elif defined(ABILITY_SYSTEM_GDEXTENSION)
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/gdvirtual.gen.inc>
#include <godot_cpp/core/type_info.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <godot_cpp/variant/variant.hpp>
#endif

namespace godot {

class AbilitySystemComponent;
class AbilitySystemAbilitySpec;
class AbilitySystemCue;

class AbilitySystemAbility : public Resource {
	GDCLASS(AbilitySystemAbility, Resource);

private:
	String ability_name;
	StringName ability_tag;
	TypedArray<StringName> activation_owned_tags;
	TypedArray<StringName> activation_required_tags;
	TypedArray<StringName> activation_blocked_tags;
	TypedArray<AbilitySystemCue> cues;

	// The main gameplay effect (e.g., damage, status)
	Ref<AbilitySystemEffect> effect;

	// Native costs (Dictionary: {attribute: StringName, amount: float})
	TypedArray<Dictionary> costs;
	bool use_custom_costs = false;

	// Native cooldown
	float cooldown_duration = 0.0;
	TypedArray<StringName> cooldown_tags;
	bool use_custom_cooldown = false;

protected:
	static void _bind_methods();

	// GDScript virtuals
	GDVIRTUAL2(_on_activate_ability, Object *, Ref<RefCounted>);
	GDVIRTUAL2RC(bool, _on_can_activate_ability, Object *, Ref<RefCounted>);
	GDVIRTUAL2(_on_end_ability, Object *, Ref<RefCounted>);

public:
	bool can_activate_ability(AbilitySystemComponent *p_owner, Ref<AbilitySystemAbilitySpec> p_spec = nullptr) const;
	void activate_ability(AbilitySystemComponent *p_owner, Ref<AbilitySystemAbilitySpec> p_spec = nullptr, Object *p_target_node = nullptr);
	void end_ability(AbilitySystemComponent *p_owner, Ref<AbilitySystemAbilitySpec> p_spec = nullptr);

	void set_ability_name(const String &p_name);
	String get_ability_name() const;

	void set_ability_tag(const StringName &p_tag);
	StringName get_ability_tag() const;

	void set_activation_owned_tags(const TypedArray<StringName> &p_tags);
	TypedArray<StringName> get_activation_owned_tags() const;

	void set_activation_required_tags(const TypedArray<StringName> &p_tags);
	TypedArray<StringName> get_activation_required_tags() const;

	void set_activation_blocked_tags(const TypedArray<StringName> &p_tags);
	TypedArray<StringName> get_activation_blocked_tags() const;

	void set_cues(const TypedArray<AbilitySystemCue> &p_cues);
	TypedArray<AbilitySystemCue> get_cues() const;

	void set_effect(Ref<AbilitySystemEffect> p_effect);
	Ref<AbilitySystemEffect> get_effect() const;

	// Cooldown methods
	void set_cooldown_duration(float p_duration);
	float get_cooldown_duration() const;
	void set_cooldown_tags(const TypedArray<StringName> &p_tags);
	TypedArray<StringName> get_cooldown_tags() const;
	void set_use_custom_cooldown(bool p_use);
	bool get_use_custom_cooldown() const;

	// Cost methods
	void set_costs(const TypedArray<Dictionary> &p_costs);
	TypedArray<Dictionary> get_costs() const;
	void set_use_custom_costs(bool p_use);
	bool get_use_custom_costs() const;

	void add_cost(const StringName &p_attribute, float p_amount);
	bool remove_cost(const StringName &p_attribute);
	float get_cost_amount(const StringName &p_attribute) const;

	bool can_afford_costs(AbilitySystemComponent *p_owner, Ref<AbilitySystemAbilitySpec> p_spec = nullptr) const;
	void apply_costs(AbilitySystemComponent *p_owner, Ref<AbilitySystemAbilitySpec> p_spec = nullptr) const;

	AbilitySystemAbility();
	~AbilitySystemAbility();
};

} // namespace godot

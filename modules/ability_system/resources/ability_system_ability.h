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

#include "core/io/resource.h"
#include "core/object/gdvirtual.gen.inc"
#include "core/variant/typed_array.h"

class AbilitySystemComponent;
class AbilitySystemEffect;

class AbilitySystemAbilitySpec;

class AbilitySystemAbility : public Resource {
	GDCLASS(AbilitySystemAbility, Resource);

private:
	String ability_name;
	StringName ability_tag;
	TypedArray<StringName> activation_owned_tags;
	TypedArray<StringName> activation_required_tags;
	TypedArray<StringName> activation_blocked_tags;
	TypedArray<class AbilitySystemCue> cues;

	Ref<AbilitySystemEffect> effect;
	Ref<AbilitySystemEffect> cost_effect;
	Ref<AbilitySystemEffect> cooldown_effect;

protected:
	static void _bind_methods();

	// GDScript virtuals
	GDVIRTUAL2(_on_activate_ability, Object *, Ref<RefCounted>); // Context passed as generic RefCounted to avoid binding issues if Spec not exposed properly
	GDVIRTUAL2RC(bool, _on_can_activate_ability, Object *, Ref<RefCounted>);
	GDVIRTUAL2(_on_end_ability, Object *, Ref<RefCounted>);

public:
	// Public methods take the owner and the spec context (for level/state)
	bool can_activate_ability(AbilitySystemComponent *p_owner, Ref<AbilitySystemAbilitySpec> p_spec = nullptr) const;
	void activate_ability(AbilitySystemComponent *p_owner, Ref<AbilitySystemAbilitySpec> p_spec = nullptr);
	void end_ability(AbilitySystemComponent *p_owner, Ref<AbilitySystemAbilitySpec> p_spec = nullptr);
	void set_ability_name(const String &p_name);
	String get_ability_name() const { return ability_name; }

	void set_ability_tag(const StringName &p_tag);
	StringName get_ability_tag() const { return ability_tag; }

	void set_activation_owned_tags(const TypedArray<StringName> &p_tags);
	TypedArray<StringName> get_activation_owned_tags() const { return activation_owned_tags; }

	void set_activation_required_tags(const TypedArray<StringName> &p_tags);
	TypedArray<StringName> get_activation_required_tags() const { return activation_required_tags; }

	void set_activation_blocked_tags(const TypedArray<StringName> &p_tags);
	TypedArray<StringName> get_activation_blocked_tags() const { return activation_blocked_tags; }

	void set_cues(const TypedArray<class AbilitySystemCue> &p_cues);
	TypedArray<class AbilitySystemCue> get_cues() const { return cues; }

	void set_cost_effect(Ref<AbilitySystemEffect> p_effect);
	Ref<AbilitySystemEffect> get_cost_effect() const;

	void set_cooldown_effect(Ref<AbilitySystemEffect> p_effect);
	Ref<AbilitySystemEffect> get_cooldown_effect() const;

	void set_effect(Ref<AbilitySystemEffect> p_effect);
	Ref<AbilitySystemEffect> get_effect() const;

	AbilitySystemAbility();
	~AbilitySystemAbility();
};

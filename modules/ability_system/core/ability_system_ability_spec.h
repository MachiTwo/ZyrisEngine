/**************************************************************************/
/*  ability_system_ability_spec.h                                         */
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
#include "modules/ability_system/resources/ability_system_ability.h"
#elif defined(ABILITY_SYSTEM_GDEXTENSION)
#include "src/resources/ability_system_ability.h"
#endif

#ifdef ABILITY_SYSTEM_MODULE
#include "core/object/ref_counted.h"
#include "core/templates/vector.h"
#include "core/variant/typed_array.h"
#include "core/variant/variant.h"
#elif defined(ABILITY_SYSTEM_GDEXTENSION)
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/templates/vector.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <godot_cpp/variant/variant.hpp>
#endif

namespace godot {

class AbilitySystemComponent;
class AbilitySystemEffectSpec;

class AbilitySystemAbilitySpec : public RefCounted {
	GDCLASS(AbilitySystemAbilitySpec, RefCounted);

protected:
	static void _bind_methods();

private:
	Ref<AbilitySystemAbility> ability;
	bool is_active = false;
	int level = 1;
	ObjectID owner_id;
	Vector<Ref<AbilitySystemEffectSpec>> active_effects;

public:
	void init(Ref<AbilitySystemAbility> p_ability, int p_level = 1);
	Ref<AbilitySystemAbility> get_ability() const { return ability; }

	bool get_is_active() const { return is_active; }
	void set_is_active(bool p_active) { is_active = p_active; }

	int get_level() const { return level; }
	void set_level(int p_level) { level = p_level; }

	void set_owner(AbilitySystemComponent *p_owner);
	AbilitySystemComponent *get_owner() const;

	// Runtime query methods
	float get_cooldown_duration() const;
	float get_cooldown_remaining() const;
	bool is_on_cooldown() const;

	float get_cost_amount(const StringName &p_attribute) const;

	// Effect tracking for cascading cancellation
	void add_active_effect(Ref<AbilitySystemEffectSpec> p_spec);
	void remove_active_effect(Ref<AbilitySystemEffectSpec> p_spec);
	Vector<Ref<AbilitySystemEffectSpec>> get_active_effects() const { return active_effects; }
	void clear_active_effects();

	AbilitySystemAbilitySpec();
	~AbilitySystemAbilitySpec();
};

} // namespace godot

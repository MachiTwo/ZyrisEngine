/**************************************************************************/
/*  ability_system_attribute_set.h                                        */
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
#include "core/templates/hash_map.h"
#include "core/variant/typed_array.h"
#include "modules/ability_system/resources/ability_system_attribute.h"

class AbilitySystemAbility;

/**
 * AbilitySystemAttributeSet
 *
 * Responsible for initializing the actor. Defines:
 * - Attribute definitions
 * - Which abilities from the catalog are already granted/unlocked
 *
 * The Container stores this set; the ASC uses it to initialize the entity.
 */
class AbilitySystemAttributeSet : public Resource {
	GDCLASS(AbilitySystemAttributeSet, Resource);

public:
	struct AttributeValue {
		float base_value = 0.0f;
		float current_value = 0.0f;
	};

protected:
	static void _bind_methods();

	// Attribute definitions (configurable Resources)
	TypedArray<AbilitySystemAttribute> attribute_definitions;

	// Abilities already granted/unlocked from the catalog
	TypedArray<AbilitySystemAbility> granted_abilities;

	// Runtime values (maintained for performance)
	HashMap<StringName, AttributeValue> attributes;

public:
	// Resource-based API
	void add_attribute_definition(Ref<AbilitySystemAttribute> p_attribute);
	void remove_attribute_definition(const StringName &p_name);
	Ref<AbilitySystemAttribute> get_attribute_definition(const StringName &p_name) const;
	void set_attribute_definitions(const TypedArray<AbilitySystemAttribute> &p_definitions);
	TypedArray<AbilitySystemAttribute> get_attribute_definitions() const;

	// Granted abilities (unlocked from the catalog)
	void set_granted_abilities(const TypedArray<AbilitySystemAbility> &p_abilities) { granted_abilities = p_abilities; }
	TypedArray<AbilitySystemAbility> get_granted_abilities() const { return granted_abilities; }

	// Value API (validated via Resource)
	void set_attribute_base_value(const StringName &p_name, float p_value);
	float get_attribute_base_value(const StringName &p_name) const;

	void set_attribute_current_value(const StringName &p_name, float p_value);
	float get_attribute_current_value(const StringName &p_name) const;

	bool has_attribute(const StringName &p_name) const;
	TypedArray<StringName> get_attribute_list() const;
	void reset_current_values();

	// Signal handlers for attribute changes
	void _on_attribute_value_changed(float p_old_value, float p_new_value, const StringName &p_name);
	void _on_attribute_limits_changed(float p_min_value, float p_max_value, const StringName &p_name);

	// Helper to find attribute by name
	Ref<AbilitySystemAttribute> _find_attribute_by_name(const StringName &p_name) const;

	// Signal binding helpers
	void _bind_attribute_signals(Ref<AbilitySystemAttribute> p_attribute);
	void _unbind_attribute_signals(Ref<AbilitySystemAttribute> p_attribute);

	AbilitySystemAttributeSet();
	~AbilitySystemAttributeSet();
};

/**************************************************************************/
/*  ability_system_attribute_set.cpp                                      */
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

#include "modules/ability_system/resources/ability_system_attribute_set.h"

#include "modules/ability_system/resources/ability_system_ability.h"

void AbilitySystemAttributeSet::_bind_methods() {
	// Resource-based API
	ClassDB::bind_method(D_METHOD("add_attribute_definition", "attribute"), &AbilitySystemAttributeSet::add_attribute_definition);
	ClassDB::bind_method(D_METHOD("remove_attribute_definition", "name"), &AbilitySystemAttributeSet::remove_attribute_definition);
	ClassDB::bind_method(D_METHOD("get_attribute_definition", "name"), &AbilitySystemAttributeSet::get_attribute_definition);
	ClassDB::bind_method(D_METHOD("set_attribute_definitions", "definitions"), &AbilitySystemAttributeSet::set_attribute_definitions);
	ClassDB::bind_method(D_METHOD("get_attribute_definitions"), &AbilitySystemAttributeSet::get_attribute_definitions);

	// Granted abilities
	ClassDB::bind_method(D_METHOD("set_granted_abilities", "abilities"), &AbilitySystemAttributeSet::set_granted_abilities);
	ClassDB::bind_method(D_METHOD("get_granted_abilities"), &AbilitySystemAttributeSet::get_granted_abilities);

	// Value API (validated via Resource)
	ClassDB::bind_method(D_METHOD("set_attribute_base_value", "name", "value"), &AbilitySystemAttributeSet::set_attribute_base_value);
	ClassDB::bind_method(D_METHOD("get_attribute_base_value", "name"), &AbilitySystemAttributeSet::get_attribute_base_value);
	ClassDB::bind_method(D_METHOD("set_attribute_current_value", "name", "value"), &AbilitySystemAttributeSet::set_attribute_current_value);
	ClassDB::bind_method(D_METHOD("get_attribute_current_value", "name"), &AbilitySystemAttributeSet::get_attribute_current_value);
	ClassDB::bind_method(D_METHOD("has_attribute", "name"), &AbilitySystemAttributeSet::has_attribute);
	ClassDB::bind_method(D_METHOD("get_attribute_list"), &AbilitySystemAttributeSet::get_attribute_list);
	ClassDB::bind_method(D_METHOD("reset_current_values"), &AbilitySystemAttributeSet::reset_current_values);

	// Properties — attribute_definitions and granted_abilities at the top for editor visibility.
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "attribute_definitions", PROPERTY_HINT_TYPE_STRING, vformat("%s/%s:%s", Variant::OBJECT, PROPERTY_HINT_RESOURCE_TYPE, "AbilitySystemAttribute")), "set_attribute_definitions", "get_attribute_definitions");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "granted_abilities", PROPERTY_HINT_TYPE_STRING, vformat("%s/%s:%s", Variant::OBJECT, PROPERTY_HINT_RESOURCE_TYPE, "AbilitySystemAbility")), "set_granted_abilities", "get_granted_abilities");

	// Signals
	ADD_SIGNAL(MethodInfo("attribute_changed", PropertyInfo(Variant::STRING_NAME, "attribute_name"), PropertyInfo(Variant::FLOAT, "old_value"), PropertyInfo(Variant::FLOAT, "new_value")));
	ADD_SIGNAL(MethodInfo("attribute_pre_change", PropertyInfo(Variant::STRING_NAME, "attribute_name"), PropertyInfo(Variant::FLOAT, "old_value"), PropertyInfo(Variant::FLOAT, "new_value")));
}

// Resource-based API
void AbilitySystemAttributeSet::add_attribute_definition(Ref<AbilitySystemAttribute> p_attribute) {
	if (p_attribute.is_null() || p_attribute->get_attribute_name().is_empty()) {
		return;
	}

	StringName attr_name = p_attribute->get_attribute_name();

	// Check if already exists
	for (int i = 0; i < attribute_definitions.size(); i++) {
		Ref<AbilitySystemAttribute> existing = attribute_definitions[i];
		if (existing.is_valid() && existing->get_attribute_name() == attr_name) {
			return; // Already exists
		}
	}

	attribute_definitions.push_back(p_attribute);

	// Initialize runtime value if not exists
	if (!attributes.has(attr_name)) {
		AttributeValue av;
		av.base_value = p_attribute->get_base_value();
		av.current_value = p_attribute->get_base_value();
		attributes[attr_name] = av;
	}

	// Connect to attribute signals for value changes
	_bind_attribute_signals(p_attribute);
}

void AbilitySystemAttributeSet::remove_attribute_definition(const StringName &p_name) {
	for (int i = 0; i < attribute_definitions.size(); i++) {
		Ref<AbilitySystemAttribute> attr = attribute_definitions[i];
		if (attr.is_valid() && attr->get_attribute_name() == p_name) {
			// Disconnect signals before removing
			_unbind_attribute_signals(attr);

			attribute_definitions.remove_at(i);
			break;
		}
	}

	// Remove runtime values
	attributes.erase(p_name);
}

Ref<AbilitySystemAttribute> AbilitySystemAttributeSet::get_attribute_definition(const StringName &p_name) const {
	for (int i = 0; i < attribute_definitions.size(); i++) {
		Ref<AbilitySystemAttribute> attr = attribute_definitions[i];
		if (attr.is_valid() && attr->get_attribute_name() == p_name) {
			return attr;
		}
	}
	return nullptr;
}

void AbilitySystemAttributeSet::set_attribute_definitions(const TypedArray<AbilitySystemAttribute> &p_definitions) {
	// Unbind from old definitions
	for (int i = 0; i < attribute_definitions.size(); i++) {
		Ref<AbilitySystemAttribute> attr = attribute_definitions[i];
		if (attr.is_valid()) {
			_unbind_attribute_signals(attr);
		}
	}

	attribute_definitions = p_definitions;

	// Bind to new and initialize
	for (int i = 0; i < attribute_definitions.size(); i++) {
		Ref<AbilitySystemAttribute> attr = attribute_definitions[i];
		if (attr.is_valid()) {
			StringName attr_name = attr->get_attribute_name();
			if (!attributes.has(attr_name)) {
				AttributeValue av;
				av.base_value = attr->get_base_value();
				av.current_value = attr->get_base_value();
				attributes[attr_name] = av;
			}
			_bind_attribute_signals(attr);
		}
	}
}

TypedArray<AbilitySystemAttribute> AbilitySystemAttributeSet::get_attribute_definitions() const {
	return attribute_definitions;
}

// Value API (validated via Resource)
void AbilitySystemAttributeSet::set_attribute_base_value(const StringName &p_name, float p_value) {
	// Validate using Resource
	Ref<AbilitySystemAttribute> def = get_attribute_definition(p_name);
	ERR_FAIL_COND_MSG(def.is_null(), vformat("Attribute '%s' not found in AttributeSet", p_name));

	float validated_value = def->clamp_value(p_value);
	attributes[p_name].base_value = validated_value;

	// Update the attribute's base value to keep them in sync
	def->set_base_value(validated_value);
}

float AbilitySystemAttributeSet::get_attribute_base_value(const StringName &p_name) const {
	ERR_FAIL_COND_V_MSG(!attributes.has(p_name), 0.0f, vformat("Attribute '%s' not found in AttributeSet", p_name));
	return attributes[p_name].base_value;
}

void AbilitySystemAttributeSet::set_attribute_current_value(const StringName &p_name, float p_value) {
	// Validate using Resource
	Ref<AbilitySystemAttribute> def = get_attribute_definition(p_name);
	ERR_FAIL_COND_MSG(def.is_null(), vformat("Attribute '%s' not found in AttributeSet", p_name));

	float validated_value = def->clamp_value(p_value);

	float old_val = 0.0f;
	if (attributes.has(p_name)) {
		old_val = attributes[p_name].current_value;
		emit_signal("attribute_pre_change", p_name, old_val, validated_value);
		attributes[p_name].current_value = validated_value;
	} else {
		AttributeValue av;
		av.base_value = validated_value;
		av.current_value = validated_value;
		attributes[p_name] = av;
		emit_signal("attribute_pre_change", p_name, old_val, validated_value);
	}
	emit_signal("attribute_changed", p_name, old_val, validated_value);

	// Update the attribute's base value to keep them in sync
	def->set_base_value(validated_value);
}

float AbilitySystemAttributeSet::get_attribute_current_value(const StringName &p_name) const {
	ERR_FAIL_COND_V_MSG(!attributes.has(p_name), 0.0f, vformat("Attribute '%s' not found in AttributeSet", p_name));
	return attributes[p_name].current_value;
}

bool AbilitySystemAttributeSet::has_attribute(const StringName &p_name) const {
	return attributes.has(p_name);
}

TypedArray<StringName> AbilitySystemAttributeSet::get_attribute_list() const {
	TypedArray<StringName> res;
	for (const KeyValue<StringName, AttributeValue> &E : attributes) {
		res.push_back(E.key);
	}
	return res;
}

void AbilitySystemAttributeSet::reset_current_values() {
	for (KeyValue<StringName, AttributeValue> &E : attributes) {
		E.value.current_value = E.value.base_value;
	}
}

AbilitySystemAttributeSet::AbilitySystemAttributeSet() {
}

AbilitySystemAttributeSet::~AbilitySystemAttributeSet() {
	// Disconnect all signals when destroyed
	for (int i = 0; i < attribute_definitions.size(); i++) {
		Ref<AbilitySystemAttribute> attr = attribute_definitions[i];
		if (attr.is_valid()) {
			_unbind_attribute_signals(attr);
		}
	}
}

void AbilitySystemAttributeSet::_bind_attribute_signals(Ref<AbilitySystemAttribute> p_attribute) {
	StringName attr_name = p_attribute->get_attribute_name();

	p_attribute->connect("value_changed", callable_mp(this, &AbilitySystemAttributeSet::_on_attribute_value_changed).bind(attr_name));
	p_attribute->connect("limits_changed", callable_mp(this, &AbilitySystemAttributeSet::_on_attribute_limits_changed).bind(attr_name));
}

void AbilitySystemAttributeSet::_unbind_attribute_signals(Ref<AbilitySystemAttribute> p_attribute) {
	p_attribute->disconnect("value_changed", callable_mp(this, &AbilitySystemAttributeSet::_on_attribute_value_changed));
	p_attribute->disconnect("limits_changed", callable_mp(this, &AbilitySystemAttributeSet::_on_attribute_limits_changed));
}

// Signal handlers for attribute changes
void AbilitySystemAttributeSet::_on_attribute_value_changed(float p_old_value, float p_new_value, const StringName &p_name) {
	// Emit AttributeSet signal for attribute value change
	emit_signal("attribute_changed", p_name, p_old_value, p_new_value);
}

void AbilitySystemAttributeSet::_on_attribute_limits_changed(float p_min_value, float p_max_value, const StringName &p_name) {
	// Handle limits changes if needed
}

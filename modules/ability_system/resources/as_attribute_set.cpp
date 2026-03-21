/**************************************************************************/
/*  as_attribute_set.cpp                                                  */
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

#ifdef ABILITY_SYSTEM_GDEXTENSION
#include "src/resources/as_attribute_set.h"
#include "src/core/ability_system.h"
#include "src/resources/as_attribute.h"
#else
#include "modules/ability_system/core/ability_system.h"
#include "modules/ability_system/resources/as_attribute.h"
#include "modules/ability_system/resources/as_attribute_set.h"
#endif

#ifdef ABILITY_SYSTEM_GDEXTENSION
using namespace godot;
#endif

void ASAttributeSet::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_attribute_definitions", "definitions"), &ASAttributeSet::set_attribute_definitions);
	ClassDB::bind_method(D_METHOD("get_attribute_definitions"), &ASAttributeSet::get_attribute_definitions);
	ClassDB::bind_method(D_METHOD("add_attribute_definition", "attribute"), &ASAttributeSet::add_attribute_definition);
	ClassDB::bind_method(D_METHOD("remove_attribute_definition", "name"), &ASAttributeSet::remove_attribute_definition);
	ClassDB::bind_method(D_METHOD("get_attribute_definition", "name"), &ASAttributeSet::get_attribute_definition);

	ClassDB::bind_method(D_METHOD("set_unlocked_abilities", "abilities"), &ASAttributeSet::set_unlocked_abilities);
	ClassDB::bind_method(D_METHOD("get_unlocked_abilities"), &ASAttributeSet::get_unlocked_abilities);

	ClassDB::bind_method(D_METHOD("get_attribute_value", "name"), &ASAttributeSet::get_attribute_value);
	ClassDB::bind_method(D_METHOD("get_attribute_base_value", "name"), &ASAttributeSet::get_attribute_base_value);
	ClassDB::bind_method(D_METHOD("set_attribute_base_value", "name", "value"), &ASAttributeSet::set_attribute_base_value);
	ClassDB::bind_method(D_METHOD("has_attribute", "name"), &ASAttributeSet::has_attribute);
	ClassDB::bind_method(D_METHOD("get_attribute_list"), &ASAttributeSet::get_attribute_list);

	ClassDB::bind_method(D_METHOD("set_attribute_drivers", "drivers"), &ASAttributeSet::set_attribute_drivers);
	ClassDB::bind_method(D_METHOD("get_attribute_drivers"), &ASAttributeSet::get_attribute_drivers);

	ClassDB::bind_method(D_METHOD("add_modifier", "name", "value", "type"), &ASAttributeSet::add_modifier, DEFVAL(MODIFIER_ADD));
	ClassDB::bind_method(D_METHOD("remove_modifier", "name", "value", "type"), &ASAttributeSet::remove_modifier, DEFVAL(MODIFIER_ADD));

	ClassDB::bind_method(D_METHOD("_on_attribute_value_changed", "old_value", "new_value", "name"), &ASAttributeSet::_on_attribute_value_changed);
	ClassDB::bind_method(D_METHOD("_on_attribute_limits_changed", "min_value", "max_value", "name"), &ASAttributeSet::_on_attribute_limits_changed);

	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "attribute_definitions", PROPERTY_HINT_ARRAY_TYPE, "ASAttribute"), "set_attribute_definitions", "get_attribute_definitions");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "unlocked_abilities", PROPERTY_HINT_ARRAY_TYPE, "ASAbility"), "set_unlocked_abilities", "get_unlocked_abilities");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "attribute_drivers", PROPERTY_HINT_ARRAY_TYPE, "Dictionary"), "set_attribute_drivers", "get_attribute_drivers");

	ADD_SIGNAL(MethodInfo("attribute_changed", PropertyInfo(Variant::STRING_NAME, "attribute_name"), PropertyInfo(Variant::FLOAT, "old_value"), PropertyInfo(Variant::FLOAT, "new_value")));

	BIND_ENUM_CONSTANT(MODIFIER_ADD);
	BIND_ENUM_CONSTANT(MODIFIER_MULTIPLY);
}

void ASAttributeSet::add_attribute_definition(Ref<ASAttribute> p_attribute) {
	ERR_FAIL_COND(p_attribute.is_null());
	if (attribute_definitions.find(p_attribute) == -1) {
		attribute_definitions.push_back(p_attribute);
		_bind_attribute_signals(p_attribute);

		AttributeValue val;
		val.base_value = p_attribute->get_base_value();
		val.current_value = val.base_value;
		attributes[p_attribute->get_attribute_name()] = val;

		if (AbilitySystem::get_singleton()) {
			AbilitySystem::get_singleton()->register_tag(p_attribute->get_attribute_name(), AbilitySystem::TAG_TYPE_NAME, get_instance_id());
		}
	}
}

void ASAttributeSet::remove_attribute_definition(const StringName &p_name) {
	for (int i = 0; i < attribute_definitions.size(); i++) {
		Ref<ASAttribute> attr = attribute_definitions[i];
		if (attr.is_valid() && attr->get_attribute_name() == p_name) {
			_unbind_attribute_signals(attr);
			attribute_definitions.remove_at(i);
			attributes.erase(p_name);
			return;
		}
	}
}

Ref<ASAttribute> ASAttributeSet::get_attribute_definition(const StringName &p_name) const {
	return _find_attribute_by_name(p_name);
}

void ASAttributeSet::set_attribute_definitions(const TypedArray<ASAttribute> &p_definitions) {
	// Cleanup old
	for (int i = 0; i < attribute_definitions.size(); i++) {
		Ref<ASAttribute> attr = attribute_definitions[i];
		if (attr.is_valid()) {
			_unbind_attribute_signals(attr);
		}
	}

	attribute_definitions = p_definitions;
	attributes.clear();

	for (int i = 0; i < attribute_definitions.size(); i++) {
		Ref<ASAttribute> attr = attribute_definitions[i];
		if (attr.is_valid()) {
			_bind_attribute_signals(attr);
			AttributeValue val;
			val.base_value = attr->get_base_value();
			val.current_value = val.base_value;
			attributes[attr->get_attribute_name()] = val;

			if (AbilitySystem::get_singleton()) {
				AbilitySystem::get_singleton()->register_tag(attr->get_attribute_name(), AbilitySystem::TAG_TYPE_NAME, get_instance_id());
			}
		}
	}
}

TypedArray<ASAttribute> ASAttributeSet::get_attribute_definitions() const {
	return attribute_definitions;
}

void ASAttributeSet::set_attribute_base_value(const StringName &p_name, float p_value) {
	Ref<ASAttribute> attr = _find_attribute_by_name(p_name);
	if (attr.is_valid()) {
		attr->set_base_value(p_value); // This will trigger _on_attribute_value_changed
	}
}

float ASAttributeSet::get_attribute_base_value(const StringName &p_name) const {
	if (attributes.has(p_name)) {
		return attributes[p_name].base_value;
	}
	return 0.0f;
}

void ASAttributeSet::set_attribute_current_value(const StringName &p_name, float p_value) {
	if (attributes.has(p_name)) {
		attributes[p_name].current_value = p_value;
	}
}

float ASAttributeSet::get_attribute_current_value(const StringName &p_name) const {
	if (attributes.has(p_name)) {
		return attributes[p_name].current_value;
	}
	return 0.0f;
}

float ASAttributeSet::get_attribute_value(const StringName &p_name) const {
	return get_attribute_current_value(p_name);
}

bool ASAttributeSet::has_attribute(const StringName &p_name) const {
	return attributes.has(p_name);
}

TypedArray<StringName> ASAttributeSet::get_attribute_list() const {
	TypedArray<StringName> res;
	for (const KeyValue<StringName, AttributeValue> &E : attributes) {
		res.push_back(E.key);
	}
	return res;
}

void ASAttributeSet::reset_current_values() {
	for (KeyValue<StringName, AttributeValue> &E : attributes) {
		E.value.current_value = E.value.base_value;
	}
}

void ASAttributeSet::_on_attribute_value_changed(float p_old_value, float p_new_value, const StringName &p_name) {
	if (attributes.has(p_name)) {
		attributes[p_name].base_value = p_new_value;
		attributes[p_name].current_value += (p_new_value - p_old_value);
		_apply_drivers_for_source(p_name, p_new_value);
		emit_signal("attribute_changed", p_name, p_old_value, p_new_value);
	}
}

void ASAttributeSet::_on_attribute_limits_changed(float p_min_value, float p_max_value, const StringName &p_name) {
	// Limits changes might force a base value clamp
	Ref<ASAttribute> attr = _find_attribute_by_name(p_name);
	if (attr.is_valid()) {
		float old_base = attr->get_base_value();
		float new_base = CLAMP(old_base, p_min_value, p_max_value);
		if (old_base != new_base) {
			attr->set_base_value(new_base);
		}
	}
}

Ref<ASAttribute> ASAttributeSet::_find_attribute_by_name(const StringName &p_name) const {
	for (int i = 0; i < attribute_definitions.size(); i++) {
		Ref<ASAttribute> attr = attribute_definitions[i];
		if (attr.is_valid() && attr->get_attribute_name() == p_name) {
			return attr;
		}
	}
	return Ref<ASAttribute>();
}

void ASAttributeSet::_bind_attribute_signals(Ref<ASAttribute> p_attribute) {
	if (p_attribute.is_valid()) {
		if (!p_attribute->is_connected("value_changed", callable_mp(this, &ASAttributeSet::_on_attribute_value_changed))) {
			p_attribute->connect("value_changed", callable_mp(this, &ASAttributeSet::_on_attribute_value_changed).bind(p_attribute->get_attribute_name()));
		}
		if (!p_attribute->is_connected("limits_changed", callable_mp(this, &ASAttributeSet::_on_attribute_limits_changed))) {
			p_attribute->connect("limits_changed", callable_mp(this, &ASAttributeSet::_on_attribute_limits_changed).bind(p_attribute->get_attribute_name()));
		}
	}
}

void ASAttributeSet::_unbind_attribute_signals(Ref<ASAttribute> p_attribute) {
	if (p_attribute.is_valid()) {
		if (p_attribute->is_connected("value_changed", callable_mp(this, &ASAttributeSet::_on_attribute_value_changed))) {
			p_attribute->disconnect("value_changed", callable_mp(this, &ASAttributeSet::_on_attribute_value_changed));
		}
		if (p_attribute->is_connected("limits_changed", callable_mp(this, &ASAttributeSet::_on_attribute_limits_changed))) {
			p_attribute->disconnect("limits_changed", callable_mp(this, &ASAttributeSet::_on_attribute_limits_changed));
		}
	}
}

void ASAttributeSet::set_attribute_drivers(const TypedArray<Dictionary> &p_drivers) {
	attribute_drivers = p_drivers;
	// Recalculate all destinations based on current sources
	for (int i = 0; i < attribute_drivers.size(); i++) {
		Dictionary d = attribute_drivers[i];
		StringName src = d.get("source", StringName());
		if (src != StringName() && attributes.has(src)) {
			_apply_drivers_for_source(src, attributes[src].base_value);
		}
	}
}

void ASAttributeSet::_apply_drivers_for_source(const StringName &p_source, float p_new_base) {
	for (int i = 0; i < attribute_drivers.size(); i++) {
		Dictionary d = attribute_drivers[i];
		StringName src = d.get("source", StringName());
		StringName dst = d.get("destination", StringName());
		if (src == p_source && attributes.has(dst)) {
			// Accumulate: start from base, add contribution
			// Simple approach: set driven portion = source * ratio
			// We track by recalculating full destination value from all drivers + its own base
			attributes[dst].current_value = attributes[dst].base_value;
			for (int j = 0; j < attribute_drivers.size(); j++) {
				Dictionary d2 = attribute_drivers[j];
				StringName dst2 = d2.get("destination", StringName());
				if (dst2 == dst) {
					StringName src2 = d2.get("source", StringName());
					float ratio2 = d2.get("ratio", 1.0f);
					if (attributes.has(src2)) {
						attributes[dst].current_value += attributes[src2].base_value * ratio2;
					}
				}
			}
			break; // Destination recalculated — move on
		}
	}
}

void ASAttributeSet::add_modifier(const StringName &p_name, float p_value, ModifierType p_type) {
	if (attributes.has(p_name)) {
		if (p_type == MODIFIER_ADD) {
			attributes[p_name].current_value += p_value;
		} else if (p_type == MODIFIER_MULTIPLY) {
			attributes[p_name].current_value *= p_value;
		}
	}
}

void ASAttributeSet::remove_modifier(const StringName &p_name, float p_value, ModifierType p_type) {
	if (attributes.has(p_name)) {
		if (p_type == MODIFIER_ADD) {
			attributes[p_name].current_value -= p_value;
		} else if (p_type == MODIFIER_MULTIPLY) {
			if (p_value != 0) {
				attributes[p_name].current_value /= p_value;
			}
		}
	}
}

ASAttributeSet::ASAttributeSet() {
}

ASAttributeSet::~ASAttributeSet() {
	for (int i = 0; i < attribute_definitions.size(); i++) {
		Ref<ASAttribute> attr = attribute_definitions[i];
		if (attr.is_valid()) {
			_unbind_attribute_signals(attr);
		}
	}
	attribute_definitions.clear();
	attributes.clear();
}

/**************************************************************************/
/*  ability_system_attribute.cpp                                          */
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

#ifdef ABILITY_SYSTEM_MODULE
#include "modules/ability_system/resources/ability_system_attribute.h"
#include "modules/ability_system/core/ability_system.h"
#elif defined(ABILITY_SYSTEM_GDEXTENSION)
#include "src/core/ability_system.h"
#include "src/resources/ability_system_attribute.h"
#endif

namespace godot {

void AbilitySystemAttribute::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_attribute_name", "name"), &AbilitySystemAttribute::set_attribute_name);
	ClassDB::bind_method(D_METHOD("get_attribute_name"), &AbilitySystemAttribute::get_attribute_name);
	ClassDB::bind_method(D_METHOD("set_base_value", "value"), &AbilitySystemAttribute::set_base_value);
	ClassDB::bind_method(D_METHOD("get_base_value"), &AbilitySystemAttribute::get_base_value);
	ClassDB::bind_method(D_METHOD("set_min_value", "value"), &AbilitySystemAttribute::set_min_value);
	ClassDB::bind_method(D_METHOD("get_min_value"), &AbilitySystemAttribute::get_min_value);
	ClassDB::bind_method(D_METHOD("set_max_value", "value"), &AbilitySystemAttribute::set_max_value);
	ClassDB::bind_method(D_METHOD("get_max_value"), &AbilitySystemAttribute::get_max_value);
	ClassDB::bind_method(D_METHOD("is_valid_value", "value"), &AbilitySystemAttribute::is_valid_value);
	ClassDB::bind_method(D_METHOD("clamp_value", "value"), &AbilitySystemAttribute::clamp_value);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "attribute_name"), "set_attribute_name", "get_attribute_name");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "base_value"), "set_base_value", "get_base_value");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "min_value"), "set_min_value", "get_min_value");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "max_value"), "set_max_value", "get_max_value");

	ADD_SIGNAL(MethodInfo("value_changed", PropertyInfo(Variant::FLOAT, "old_value"), PropertyInfo(Variant::FLOAT, "new_value")));
	ADD_SIGNAL(MethodInfo("limits_changed", PropertyInfo(Variant::FLOAT, "min_value"), PropertyInfo(Variant::FLOAT, "max_value")));
}

void AbilitySystemAttribute::set_attribute_name(const String &p_name) {
	if (attribute_name == p_name) {
		return;
	}

	AbilitySystem *as = AbilitySystem::get_singleton();
	if (as) {
		if (!attribute_name.is_empty()) {
			as->unregister_resource_name(attribute_name);
			as->unregister_tag(attribute_name);
		}
		as->register_resource_name(p_name, get_instance_id());
		as->register_tag(p_name, AbilitySystem::TAG_TYPE_NAME, get_instance_id());
	}
	attribute_name = p_name;
}

String AbilitySystemAttribute::get_attribute_name() const {
	return attribute_name;
}

void AbilitySystemAttribute::set_base_value(float p_value) {
	float old_value = base_value;
	base_value = clamp_value(p_value);
	if (old_value != base_value) {
		emit_signal("value_changed", old_value, base_value);
	}
}

float AbilitySystemAttribute::get_base_value() const {
	return base_value;
}

void AbilitySystemAttribute::set_min_value(float p_value) {
	float old_min = min_value;
	min_value = p_value;
	// Re-clamp base_value if needed
	base_value = clamp_value(base_value);
	if (old_min != min_value) {
		emit_signal("limits_changed", old_min, max_value);
	}
}

float AbilitySystemAttribute::get_min_value() const {
	return min_value;
}

void AbilitySystemAttribute::set_max_value(float p_value) {
	float old_max = max_value;
	max_value = p_value;
	// Re-clamp base_value if needed
	base_value = clamp_value(base_value);
	if (old_max != max_value) {
		emit_signal("limits_changed", min_value, old_max);
	}
}

float AbilitySystemAttribute::get_max_value() const {
	return max_value;
}

bool AbilitySystemAttribute::is_valid_value(float p_value) const {
	return p_value >= min_value && p_value <= max_value;
}

float AbilitySystemAttribute::clamp_value(float p_value) const {
	return CLAMP(p_value, min_value, max_value);
}

AbilitySystemAttribute::AbilitySystemAttribute() {
}

AbilitySystemAttribute::~AbilitySystemAttribute() {
}

} // namespace godot

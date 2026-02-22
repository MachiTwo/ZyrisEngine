/**************************************************************************/
/*  ability_system_target_data.cpp                                        */
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

#include "ability_system_target_data.h"

void AbilitySystemTargetData::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_target_node", "node"), &AbilitySystemTargetData::set_target_node);
	ClassDB::bind_method(D_METHOD("get_target_node"), &AbilitySystemTargetData::get_target_node);
	ClassDB::bind_method(D_METHOD("set_hit_position", "position"), &AbilitySystemTargetData::set_hit_position);
	ClassDB::bind_method(D_METHOD("get_hit_position"), &AbilitySystemTargetData::get_hit_position);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "target_node", PROPERTY_HINT_RESOURCE_TYPE, "Node"), "set_target_node", "get_target_node");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "hit_position"), "set_hit_position", "get_hit_position");
}

AbilitySystemTargetData::AbilitySystemTargetData() {
}

AbilitySystemTargetData::~AbilitySystemTargetData() {
}

/**************************************************************************/
/*  ability_system_tag_spec.cpp                                           */
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
#include "modules/ability_system/core/ability_system_tag_spec.h"
#include "modules/ability_system/core/ability_system.h"
#elif defined(ABILITY_SYSTEM_GDEXTENSION)
#include "src/core/ability_system.h"
#include "src/core/ability_system_tag_spec.h"
#endif

namespace godot {

void AbilitySystemTagSpec::_bind_methods() {
	ClassDB::bind_method(D_METHOD("has_tag", "tag", "exact"), &AbilitySystemTagSpec::has_tag, DEFVAL(false));
	ClassDB::bind_method(D_METHOD("has_any_tags", "tags", "exact"), &AbilitySystemTagSpec::has_any_tags, DEFVAL(false));
	ClassDB::bind_method(D_METHOD("has_all_tags", "tags", "exact"), &AbilitySystemTagSpec::has_all_tags, DEFVAL(false));
	ClassDB::bind_method(D_METHOD("add_tag", "tag"), &AbilitySystemTagSpec::add_tag);
	ClassDB::bind_method(D_METHOD("remove_tag", "tag"), &AbilitySystemTagSpec::remove_tag);
	ClassDB::bind_method(D_METHOD("clear"), &AbilitySystemTagSpec::clear);
	ClassDB::bind_method(D_METHOD("get_all_tags"), &AbilitySystemTagSpec::get_all_tags);
}

bool AbilitySystemTagSpec::has_tag(const StringName &p_tag, bool p_exact) const {
	if (p_exact) {
		return tags.has(p_tag);
	}

	for (const KeyValue<StringName, int> &E : tags) {
		if (AbilitySystem::tag_matches(E.key, p_tag, false)) {
			return true;
		}
	}
	return false;
}

bool AbilitySystemTagSpec::has_any_tags(const TypedArray<StringName> &p_tags, bool p_exact) const {
	for (int i = 0; i < p_tags.size(); i++) {
		if (has_tag(p_tags[i], p_exact)) {
			return true;
		}
	}
	return false;
}

bool AbilitySystemTagSpec::has_all_tags(const TypedArray<StringName> &p_tags, bool p_exact) const {
	for (int i = 0; i < p_tags.size(); i++) {
		if (!has_tag(p_tags[i], p_exact)) {
			return false;
		}
	}
	return true;
}

bool AbilitySystemTagSpec::add_tag(const StringName &p_tag) {
	if (p_tag == StringName()) {
		return false;
	}

	// Register with global system if not already there
	AbilitySystem::get_singleton()->register_tag(p_tag, AbilitySystem::TAG_TYPE_CONDITIONAL);

	if (tags.has(p_tag)) {
		tags[p_tag]++;
		return false;
	} else {
		tags[p_tag] = 1;
		return true;
	}
}

bool AbilitySystemTagSpec::remove_tag(const StringName &p_tag) {
	if (tags.has(p_tag)) {
		tags[p_tag]--;
		if (tags[p_tag] <= 0) {
			tags.erase(p_tag);
			return true;
		}
	}
	return false;
}

void AbilitySystemTagSpec::clear() {
	tags.clear();
}

TypedArray<StringName> AbilitySystemTagSpec::get_all_tags() const {
	TypedArray<StringName> res;
	for (const KeyValue<StringName, int> &E : tags) {
		res.push_back(E.key);
	}
	return res;
}

AbilitySystemTagSpec::AbilitySystemTagSpec() {
}

AbilitySystemTagSpec::~AbilitySystemTagSpec() {
}

} // namespace godot

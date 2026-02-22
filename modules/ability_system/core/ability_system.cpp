/**************************************************************************/
/*  ability_system.cpp                                                    */
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

#include "modules/ability_system/core/ability_system.h"
#include "core/config/project_settings.h"

AbilitySystem *AbilitySystem::singleton = nullptr;

void AbilitySystem::_bind_methods() {
	ClassDB::bind_method(D_METHOD("register_tag", "tag", "owner_id"), &AbilitySystem::register_tag, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("is_tag_registered", "tag"), &AbilitySystem::is_tag_registered);
	ClassDB::bind_method(D_METHOD("unregister_tag", "tag"), &AbilitySystem::unregister_tag);
	ClassDB::bind_method(D_METHOD("get_registered_tags"), &AbilitySystem::get_registered_tags);

	ADD_SIGNAL(MethodInfo("tags_changed"));

	ClassDB::bind_method(D_METHOD("register_resource_name", "name", "owner_id"), &AbilitySystem::register_resource_name);
	ClassDB::bind_method(D_METHOD("unregister_resource_name", "name"), &AbilitySystem::unregister_resource_name);
	ClassDB::bind_method(D_METHOD("get_resource_name_owner", "name"), &AbilitySystem::get_resource_name_owner);

	ClassDB::bind_static_method("AbilitySystem", D_METHOD("tag_matches", "tag", "match_against", "exact"), &AbilitySystem::tag_matches, DEFVAL(false));
}

void AbilitySystem::_load_settings() {
	if (ProjectSettings::get_singleton()->has_setting("ability_system/common/tags")) {
		PackedStringArray tags = ProjectSettings::get_singleton()->get_setting("ability_system/common/tags");
		for (int i = 0; i < tags.size(); i++) {
			registered_tags.insert(tags[i]);
		}
	} else {
		ProjectSettings::get_singleton()->set_setting("ability_system/common/tags", PackedStringArray());
		ProjectSettings::get_singleton()->set_initial_value("ability_system/common/tags", PackedStringArray());
	}
}

void AbilitySystem::_update_settings() {
	PackedStringArray tags;
	for (const StringName &E : registered_tags) {
		tags.push_back(E);
	}
	tags.sort();
	ProjectSettings::get_singleton()->set_setting("ability_system/common/tags", tags);
	ProjectSettings::get_singleton()->save();
	emit_signal("tags_changed");
}

void AbilitySystem::register_tag(const StringName &p_tag, uint64_t p_owner_id) {
	if (p_tag == StringName()) {
		return;
	}

	if (!registered_tags.has(p_tag)) {
		registered_tags.insert(p_tag);
		if (p_owner_id != 0) {
			tag_owners[p_tag] = p_owner_id;
		}
		_update_settings();
	} else if (p_owner_id != 0 && !tag_owners.has(p_tag)) {
		// Tag was loaded from settings but doesn't have a runtime owner yet
		tag_owners[p_tag] = p_owner_id;
	}
}

bool AbilitySystem::is_tag_registered(const StringName &p_tag) const {
	return registered_tags.has(p_tag);
}

void AbilitySystem::unregister_tag(const StringName &p_tag) {
	if (registered_tags.has(p_tag)) {
		registered_tags.erase(p_tag);
		tag_owners.erase(p_tag);
		_update_settings();
	}
}

uint64_t AbilitySystem::get_tag_owner(const StringName &p_tag) const {
	if (tag_owners.has(p_tag)) {
		return tag_owners[p_tag];
	}
	return 0;
}

bool AbilitySystem::register_resource_name(const String &p_name, uint64_t p_owner_id) {
	if (p_name.is_empty()) {
		return true;
	}

	if (resource_names.has(p_name)) {
		if (resource_names[p_name] == p_owner_id) {
			return true;
		}
		return false;
	}

	resource_names[p_name] = p_owner_id;
	return true;
}

void AbilitySystem::unregister_resource_name(const String &p_name) {
	resource_names.erase(p_name);
}

uint64_t AbilitySystem::get_resource_name_owner(const String &p_name) const {
	if (resource_names.has(p_name)) {
		return resource_names[p_name];
	}
	return 0;
}

TypedArray<StringName> AbilitySystem::get_registered_tags() const {
	TypedArray<StringName> res;
	for (const StringName &E : registered_tags) {
		res.push_back(E);
	}
	return res;
}

bool AbilitySystem::tag_matches(const StringName &p_tag, const StringName &p_match_against, bool p_exact) {
	if (p_tag == p_match_against) {
		return true;
	}

	if (p_exact) {
		return false;
	}

	String s_tag = p_tag;
	String s_match = p_match_against;

	if (s_tag.begins_with(s_match + ".")) {
		return true;
	}

	return false;
}

AbilitySystem::AbilitySystem() {
	singleton = this;
	_load_settings();
}

AbilitySystem::~AbilitySystem() {
	singleton = nullptr;
}

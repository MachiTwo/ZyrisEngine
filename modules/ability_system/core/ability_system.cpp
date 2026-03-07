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

#ifdef ABILITY_SYSTEM_MODULE
#include "modules/ability_system/core/ability_system.h"
#elif defined(ABILITY_SYSTEM_GDEXTENSION)
#include "src/core/ability_system.h"
#endif

#ifdef ABILITY_SYSTEM_MODULE
#include "core/config/engine.h"
#include "core/config/project_settings.h"
#elif defined(ABILITY_SYSTEM_GDEXTENSION)
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#endif

namespace godot {

AbilitySystem *AbilitySystem::singleton = nullptr;

void AbilitySystem::_bind_methods() {
	ClassDB::bind_method(D_METHOD("register_tag", "tag", "type", "owner_id"), &AbilitySystem::register_tag, DEFVAL(TAG_TYPE_NAME), DEFVAL(0));
	ClassDB::bind_method(D_METHOD("is_tag_registered", "tag"), &AbilitySystem::is_tag_registered);
	ClassDB::bind_method(D_METHOD("unregister_tag", "tag"), &AbilitySystem::unregister_tag);
	ClassDB::bind_method(D_METHOD("get_registered_tags"), &AbilitySystem::get_registered_tags);
	ClassDB::bind_method(D_METHOD("get_registered_tags_of_type", "type"), &AbilitySystem::get_registered_tags_of_type);
	ClassDB::bind_method(D_METHOD("get_tag_type", "tag"), &AbilitySystem::get_tag_type);

	ADD_SIGNAL(MethodInfo("tags_changed"));

	ClassDB::bind_method(D_METHOD("register_resource_name", "name", "owner_id"), &AbilitySystem::register_resource_name);
	ClassDB::bind_method(D_METHOD("unregister_resource_name", "name"), &AbilitySystem::unregister_resource_name);
	ClassDB::bind_method(D_METHOD("get_resource_name_owner", "name"), &AbilitySystem::get_resource_name_owner);

	ClassDB::bind_static_method("AbilitySystem", D_METHOD("tag_matches", "tag", "match_against", "exact"), &AbilitySystem::tag_matches, DEFVAL(false));

	BIND_ENUM_CONSTANT(TAG_TYPE_NAME);
	BIND_ENUM_CONSTANT(TAG_TYPE_CONDITIONAL);
}

void AbilitySystem::_load_settings() {
	ProjectSettings *ps = ProjectSettings::get_singleton();

	// Load Name Tags
	if (ps->has_setting("ability_system/common/name_tags")) {
		PackedStringArray tags = ps->get_setting("ability_system/common/name_tags");
		for (int i = 0; i < tags.size(); i++) {
			registered_tags[tags[i]] = TAG_TYPE_NAME;
		}
	} else {
		ps->set_setting("ability_system/common/name_tags", PackedStringArray());
		ps->set_initial_value("ability_system/common/name_tags", PackedStringArray());
	}

	// Load Conditional Tags
	if (ps->has_setting("ability_system/common/conditional_tags")) {
		PackedStringArray tags = ps->get_setting("ability_system/common/conditional_tags");
		for (int i = 0; i < tags.size(); i++) {
			registered_tags[tags[i]] = TAG_TYPE_CONDITIONAL;
		}
	} else {
		ps->set_setting("ability_system/common/conditional_tags", PackedStringArray());
		ps->set_initial_value("ability_system/common/conditional_tags", PackedStringArray());
	}
}

void AbilitySystem::_update_settings() {
	PackedStringArray name_tags;
	PackedStringArray cond_tags;

	for (const KeyValue<StringName, TagType> &E : registered_tags) {
		if (E.value == TAG_TYPE_NAME) {
			name_tags.push_back(E.key);
		} else {
			cond_tags.push_back(E.key);
		}
	}

	name_tags.sort();
	cond_tags.sort();

	ProjectSettings::get_singleton()->set_setting("ability_system/common/name_tags", name_tags);
	ProjectSettings::get_singleton()->set_setting("ability_system/common/conditional_tags", cond_tags);

	if (Engine::get_singleton()->is_editor_hint()) {
		ProjectSettings::get_singleton()->save();
	}
	emit_signal("tags_changed");
}

void AbilitySystem::register_tag(const StringName &p_tag, TagType p_type, uint64_t p_owner_id) {
	if (p_tag == StringName()) {
		return;
	}

	bool changed = false;
	if (!registered_tags.has(p_tag)) {
		registered_tags[p_tag] = p_type;
		changed = true;
	} else if (p_owner_id == 0 && registered_tags[p_tag] != p_type) {
		// Re-registration from UI (no owner specified) usually means changing type
		registered_tags[p_tag] = p_type;
		changed = true;
	}

	if (p_owner_id != 0 && (!tag_owners.has(p_tag) || tag_owners[p_tag] != p_owner_id)) {
		tag_owners[p_tag] = p_owner_id;
		// Owner changes don't necessarily require settings update, but they change runtime state
	}

	if (changed) {
		_update_settings();
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

AbilitySystem::TagType AbilitySystem::get_tag_type(const StringName &p_tag) const {
	if (registered_tags.has(p_tag)) {
		return registered_tags[p_tag];
	}
	return TAG_TYPE_NAME;
}

TypedArray<StringName> AbilitySystem::get_registered_tags() const {
	TypedArray<StringName> res;
	for (const KeyValue<StringName, TagType> &E : registered_tags) {
		res.push_back(E.key);
	}
	return res;
}

TypedArray<StringName> AbilitySystem::get_registered_tags_of_type(TagType p_type) const {
	TypedArray<StringName> res;
	for (const KeyValue<StringName, TagType> &E : registered_tags) {
		if (E.value == p_type) {
			res.push_back(E.key);
		}
	}
	return res;
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

} // namespace godot

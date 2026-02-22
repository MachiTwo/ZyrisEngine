/**************************************************************************/
/*  ability_system.h                                                      */
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

#include "core/object/class_db.h"
#include "core/object/object.h"
#include "core/templates/hash_map.h"
#include "core/templates/hash_set.h"
#include "core/variant/typed_array.h"

class AbilitySystemCue;

/**
 * AbilitySystem
 * The central singleton for the Ability System module.
 * Primarily manages the Tag registry for the project.
 */
class AbilitySystem : public Object {
	GDCLASS(AbilitySystem, Object);

	static AbilitySystem *singleton;

	HashSet<StringName> registered_tags;
	HashMap<StringName, uint64_t> tag_owners;
	HashMap<String, uint64_t> resource_names;

	void _load_settings();
	void _update_settings();

protected:
	static void _bind_methods();

public:
	static AbilitySystem *get_singleton() { return singleton; }

	void register_tag(const StringName &p_tag, uint64_t p_owner_id = 0);
	bool is_tag_registered(const StringName &p_tag) const;
	void unregister_tag(const StringName &p_tag);
	uint64_t get_tag_owner(const StringName &p_tag) const;
	TypedArray<StringName> get_registered_tags() const;

	bool register_resource_name(const String &p_name, uint64_t p_owner_id);
	void unregister_resource_name(const String &p_name);
	uint64_t get_resource_name_owner(const String &p_name) const;

	// Helper to check if a tag matches another (hierarchical)
	static bool tag_matches(const StringName &p_tag, const StringName &p_match_against, bool p_exact = false);

	AbilitySystem();
	~AbilitySystem();
};

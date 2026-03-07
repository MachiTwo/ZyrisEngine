/**************************************************************************/
/*  ability_system_cue.h                                                  */
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
#include "modules/ability_system/core/ability_system_cue_spec.h"
#elif defined(ABILITY_SYSTEM_GDEXTENSION)
#include "src/core/ability_system_cue_spec.h"
#endif

#ifdef ABILITY_SYSTEM_MODULE
#include "core/object/object.h"
#elif defined(ABILITY_SYSTEM_GDEXTENSION)
#include <godot_cpp/core/type_info.hpp>
#endif

#ifdef ABILITY_SYSTEM_MODULE
#include "core/io/resource.h"
#include "core/object/gdvirtual.gen.inc"
#include "core/variant/typed_array.h"
#elif defined(ABILITY_SYSTEM_GDEXTENSION)
#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/gdvirtual.gen.inc>
#include <godot_cpp/variant/typed_array.hpp>
#endif

namespace godot {

class AbilitySystemCue : public Resource {
	GDCLASS(AbilitySystemCue, Resource);

public:
	enum CueEventType {
		ON_EXECUTE, // Instant feedback
		ON_ACTIVE, // Continuous feedback while effect/ability is active
		ON_REMOVE // Feedback when effect/ability ends
	};

protected:
	static void _bind_methods();

	// Script callbacks
	GDVIRTUAL1(_on_execute, Ref<AbilitySystemCueSpec>)
	GDVIRTUAL1(_on_active, Ref<AbilitySystemCueSpec>)
	GDVIRTUAL1(_on_remove, Ref<AbilitySystemCueSpec>)

	String cue_name;
	CueEventType event_type = ON_EXECUTE;
	StringName cue_tag;
	StringName node_name;

	// Activation requirements
	TypedArray<StringName> activation_required_tags;
	TypedArray<StringName> activation_blocked_tags;

public:
	void set_cue_name(const String &p_name);
	String get_cue_name() const { return cue_name; }

	void set_event_type(CueEventType p_type) { event_type = p_type; }
	CueEventType get_event_type() const { return event_type; }

	void set_cue_tag(const StringName &p_tag);
	StringName get_cue_tag() const { return cue_tag; }

	void set_node_name(const StringName &p_name) { node_name = p_name; }
	StringName get_node_name() const { return node_name; }

	void set_activation_required_tags(const TypedArray<StringName> &p_tags);
	TypedArray<StringName> get_activation_required_tags() const { return activation_required_tags; }

	void set_activation_blocked_tags(const TypedArray<StringName> &p_tags);
	TypedArray<StringName> get_activation_blocked_tags() const { return activation_blocked_tags; }

	// Execution logic
	virtual void execute(Ref<AbilitySystemCueSpec> p_spec);

	AbilitySystemCue();
	~AbilitySystemCue();
};

} // namespace godot

VARIANT_ENUM_CAST(godot::AbilitySystemCue::CueEventType);

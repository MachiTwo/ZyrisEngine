/**************************************************************************/
/*  ability_system_cue_spec.h                                             */
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
#include "core/object/ref_counted.h"
#include "core/variant/dictionary.h"
#include "core/variant/variant.h"
#elif defined(ABILITY_SYSTEM_GDEXTENSION)
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/variant.hpp>
#endif

namespace godot {

// Forward declarations to break circularity
class AbilitySystemCue;
class AbilitySystemComponent;
class AbilitySystemEffectSpec;

// Runtime context for a single Cue execution.
class AbilitySystemCueSpec : public RefCounted {
	GDCLASS(AbilitySystemCueSpec, RefCounted);

protected:
	static void _bind_methods();

private:
	// The Cue definition that is being executed.
	Ref<AbilitySystemCue> cue;

	// ObjectID of ASC that originated this cue.
	ObjectID source_id;

	// ObjectID of ASC that is the target / owner of this cue.
	ObjectID target_id;

	// Specific node hit (e.g. a limb or bone).
	ObjectID target_node_id;

	// Position where the hit occurred.
	Variant hit_position;

	// The EffectSpec that triggered this cue, if any.
	Ref<AbilitySystemEffectSpec> effect_spec;

	float level = 1.0f;
	float magnitude = 0.0f;
	Dictionary extra_data;

public:
	// IMPORTANT: Constructor and Destructor must be in .cpp to allow Ref<T> with forward declarations
	AbilitySystemCueSpec();
	~AbilitySystemCueSpec();

	void init_from_effect(Ref<AbilitySystemCue> p_cue,
			AbilitySystemComponent *p_source,
			AbilitySystemComponent *p_target,
			Ref<AbilitySystemEffectSpec> p_effect_spec,
			float p_magnitude);

	void init_manual(Ref<AbilitySystemCue> p_cue,
			AbilitySystemComponent *p_owner,
			const Dictionary &p_extra_data);

	// --- Cue ---
	void set_cue(Ref<AbilitySystemCue> p_cue);
	Ref<AbilitySystemCue> get_cue() const;

	// --- Source / Target ---
	void set_source_asc(AbilitySystemComponent *p_asc);
	AbilitySystemComponent *get_source_asc() const;

	void set_target_asc(AbilitySystemComponent *p_asc);
	AbilitySystemComponent *get_target_asc() const;

	void set_target_node(Object *p_node);
	Object *get_target_node() const;

	void set_hit_position(const Variant &p_pos) { hit_position = p_pos; }
	Variant get_hit_position() const { return hit_position; }

	// --- Effect Spec ---
	void set_effect_spec(Ref<AbilitySystemEffectSpec> p_spec);
	Ref<AbilitySystemEffectSpec> get_effect_spec() const;

	// --- Magnitude ---
	void set_magnitude(float p_mag) { magnitude = p_mag; }
	float get_magnitude() const { return magnitude; }

	// --- Level ---
	void set_level(float p_level) { level = p_level; }
	float get_level() const { return level; }

	// --- Extra Data ---
	void set_extra_data(const Dictionary &p_data) { extra_data = p_data; }
	Dictionary get_extra_data() const { return extra_data; }
};

} // namespace godot

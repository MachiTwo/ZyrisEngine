/**************************************************************************/
/*  ability_system_ability_container.h                                    */
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

#include "core/io/resource.h"
#include "core/variant/typed_array.h"
#include "modules/ability_system/resources/ability_system_attribute_set.h"

class AbilitySystemAbility;
class AbilitySystemEffect;
class AbilitySystemCue;
class AbilitySystemAttributeSet;

/**
 * AbilitySystemAbilityContainer
 *
 * Pure data container that defines the archetype of an actor.
 * It stores the full catalog of abilities, effects, tags,
 * the AttributeSet, and cues. It does NOT execute any logic —
 * the AttributeSet is the one that actually grants abilities
 * and initializes attribute values.
 */
class AbilitySystemAbilityContainer : public Resource {
	GDCLASS(AbilitySystemAbilityContainer, Resource);

private:
	// Full catalog of abilities this archetype CAN use.
	TypedArray<AbilitySystemAbility> abilities;

	// Effects that are always applied when this container is loaded.
	TypedArray<AbilitySystemEffect> effects;

	// Tags permanently granted by this archetype.
	TypedArray<StringName> tags;

	// The AttributeSet responsible for initializing the actor.
	Ref<AbilitySystemAttributeSet> attribute_set;

	// Cue resources for visual/audio feedback.
	TypedArray<AbilitySystemCue> cues;

protected:
	static void _bind_methods();

public:
	// Abilities (catalog)
	void set_abilities(const TypedArray<AbilitySystemAbility> &p_abilities) { abilities = p_abilities; }
	TypedArray<AbilitySystemAbility> get_abilities() const { return abilities; }

	// Effects
	void set_effects(const TypedArray<AbilitySystemEffect> &p_effects) { effects = p_effects; }
	TypedArray<AbilitySystemEffect> get_effects() const { return effects; }

	// Tags
	void set_tags(const TypedArray<StringName> &p_tags);
	TypedArray<StringName> get_tags() const { return tags; }

	// AttributeSet
	void set_attribute_set(const Ref<AbilitySystemAttributeSet> &p_set) { attribute_set = p_set; }
	Ref<AbilitySystemAttributeSet> get_attribute_set() const { return attribute_set; }

	// Cues
	void set_cues(const TypedArray<AbilitySystemCue> &p_cues) { cues = p_cues; }
	TypedArray<AbilitySystemCue> get_cues() const { return cues; }

	// Resource Verification (Archetype Contract)
	bool has_ability(const Ref<AbilitySystemAbility> &p_ability) const;
	bool has_effect(const Ref<AbilitySystemEffect> &p_effect) const;
	bool has_cue(const StringName &p_tag) const;

	AbilitySystemAbilityContainer();
	~AbilitySystemAbilityContainer();
};

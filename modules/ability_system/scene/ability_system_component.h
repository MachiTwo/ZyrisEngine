/**************************************************************************/
/*  ability_system_component.h                                            */
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
#include "core/object/gdvirtual.gen.inc"
#include "core/object/object.h"
#include "core/templates/hash_map.h"
#include "core/templates/vector.h"
#include "scene/main/node.h"
#elif defined(ABILITY_SYSTEM_GDEXTENSION)
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/gdvirtual.gen.inc>
#include <godot_cpp/core/type_info.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/templates/vector.hpp>
#endif

// Includes that introduce types used in Ref<T> or needed for direct access
#ifdef ABILITY_SYSTEM_MODULE
#include "modules/ability_system/core/ability_system_effect_spec.h"
#elif defined(ABILITY_SYSTEM_GDEXTENSION)
#include "src/core/ability_system_effect_spec.h"
#endif

#ifdef ABILITY_SYSTEM_MODULE
#include "servers/audio/audio_stream.h"
#elif defined(ABILITY_SYSTEM_GDEXTENSION)
#include <godot_cpp/classes/audio_stream.hpp>
#endif

// In MODULE mode, engine classes live in the global namespace.
// Declaring them inside namespace godot would create a ghost type
// (godot::AnimationPlayer) that conflicts with the real one.
#ifdef ABILITY_SYSTEM_MODULE
class CharacterBody2D;
class CharacterBody3D;
class AnimationPlayer;
class AnimatedSprite2D;
class AnimatedSprite3D;
class AudioStreamPlayer;
class AudioStreamPlayer2D;
class AudioStreamPlayer3D;
#endif

namespace godot {

class AbilitySystemAttributeSet;
class AbilitySystemTagSpec;
class AbilitySystemAttribute;

class AbilitySystemAbility;
class AbilitySystemAbilitySpec;
class AbilitySystemEffect;
class AbilitySystemTask;
class AbilitySystemCue;
class AbilitySystemCueSpec;
class AbilitySystemAbilityContainer;

// In GDExtension mode, all Godot classes live inside namespace godot.
#ifdef ABILITY_SYSTEM_GDEXTENSION
class CharacterBody2D;
class CharacterBody3D;
class AnimationPlayer;
class AnimatedSprite2D;
class AnimatedSprite3D;
class AudioStreamPlayer;
class AudioStreamPlayer2D;
class AudioStreamPlayer3D;
#endif

/**
 * AbilitySystemComponent (ASC)
 * The central hub for an actor's logic. Manages abilities, attributes, and effects.
 */
class AbilitySystemComponent : public Node {
	GDCLASS(AbilitySystemComponent, Node);

public:
	CharacterBody2D *character_body_2d = nullptr;
	CharacterBody3D *character_body_3d = nullptr;
	HashMap<StringName, Node *> registered_nodes;
	Node *animation_player_node = nullptr;
	Node *audio_player_node = nullptr;

	// Core Ability System Data
	Vector<Ref<AbilitySystemAttributeSet>> attribute_sets;
	Vector<Ref<AbilitySystemAbilitySpec>> granted_abilities;
	Vector<Ref<AbilitySystemEffectSpec>> active_effects;
	Vector<Ref<AbilitySystemTask>> active_tasks;
	Ref<AbilitySystemTagSpec> owned_tags;
	Vector<Ref<AbilitySystemCue>> registered_cues;
	Ref<AbilitySystemAbilityContainer> ability_container;

	// Cooldown management
	struct CooldownData {
		float remaining = 0.0f;
		TypedArray<StringName> tags;
	};
	HashMap<StringName, CooldownData> active_cooldowns;

protected:
	static void _bind_methods();
	void _notification(int p_what);

	void _on_attribute_set_attribute_changed(const StringName &p_name, float p_old_val, float p_new_val);
	void _update_attribute_current_values();

	void _process_effects(float p_delta);
	void _process_tasks(float p_delta);
	void _process_cooldowns(float p_delta);
	void _remove_effect_at_index(int p_idx);

public:
	// --- Cooldown API ---
	void start_cooldown(const StringName &p_ability_tag, float p_duration, const TypedArray<StringName> &p_cooldown_tags);
	bool is_on_cooldown(const StringName &p_ability_tag) const;
	float get_cooldown_remaining(const StringName &p_ability_tag) const;
	void tick(float p_delta);

	// --- General Initialization & Management ---
	void apply_ability_container(Ref<AbilitySystemAbilityContainer> p_container, int p_level = 1);
	void add_attribute_set(Ref<AbilitySystemAttributeSet> p_set);

	void give_ability_by_tag(const StringName &p_tag, int p_level = 1);
	void give_ability_by_resource(const Ref<AbilitySystemAbility> &p_ability, int p_level = 1);
	void remove_ability_by_tag(const StringName &p_tag);
	void remove_ability_by_resource(const Ref<AbilitySystemAbility> &p_ability);
	void remove_granted_ability_spec(Ref<AbilitySystemAbilitySpec> p_spec);

	// --- Ability Activation API ---
	bool can_activate_ability_by_tag(const StringName &p_tag);
	bool try_activate_ability_by_tag(const StringName &p_tag);
	void cancel_ability_by_tag(const StringName &p_tag);

	bool can_activate_ability_by_resource(const Ref<AbilitySystemAbility> &p_ability);
	bool try_activate_ability_by_resource(const Ref<AbilitySystemAbility> &p_ability);
	void cancel_ability_by_resource(const Ref<AbilitySystemAbility> &p_ability);

	void cancel_all_abilities();

	// --- Effect Activation API ---
	bool can_activate_effect_by_tag(const StringName &p_tag);
	bool try_activate_effect_by_tag(const StringName &p_tag, float p_level = 1.0f, Object *p_target_node = nullptr);
	void cancel_effect_by_tag(const StringName &p_tag);

	bool can_activate_effect_by_resource(const Ref<AbilitySystemEffect> &p_effect);
	bool try_activate_effect_by_resource(const Ref<AbilitySystemEffect> &p_effect, float p_level = 1.0f, Object *p_target_node = nullptr);
	void cancel_effect_by_resource(const Ref<AbilitySystemEffect> &p_effect);

	void remove_effect_by_tag(const StringName &p_tag);
	void remove_effect_by_resource(const Ref<AbilitySystemEffect> &p_effect);

	// --- Effect Execution API (Low level) ---
	void apply_effect_by_tag(const StringName &p_tag, float p_level = 1.0f, Object *p_target_node = nullptr);
	void apply_effect_by_resource(const Ref<AbilitySystemEffect> &p_effect, float p_level = 1.0f, Object *p_target_node = nullptr);

	Ref<AbilitySystemEffectSpec> make_outgoing_spec(Ref<AbilitySystemEffect> p_effect, float p_level = 1.0f, Object *p_target_node = nullptr);
	void apply_effect_spec_to_self(Ref<AbilitySystemEffectSpec> p_spec);
	void apply_effect_spec_to_target(Ref<AbilitySystemEffectSpec> p_spec, AbilitySystemComponent *p_target);
	void remove_active_effect(Ref<AbilitySystemEffectSpec> p_spec);
	bool has_active_effect_by_tag(const StringName &p_tag) const;
	bool has_active_effect_by_resource(const Ref<AbilitySystemEffect> &p_effect) const;

	// --- Attribute API ---
	float get_attribute_value_by_tag(const StringName &p_tag) const;
	float get_attribute_base_value_by_tag(const StringName &p_tag) const;
	void set_attribute_base_value_by_tag(const StringName &p_tag, float p_value);
	bool has_attribute_by_tag(const StringName &p_tag) const;

	float get_attribute_value_by_resource(const Ref<AbilitySystemAttribute> &p_attribute) const;
	float get_attribute_base_value_by_resource(const Ref<AbilitySystemAttribute> &p_attribute) const;
	void set_attribute_base_value_by_resource(const Ref<AbilitySystemAttribute> &p_attribute, float p_value);
	bool has_attribute_by_resource(const Ref<AbilitySystemAttribute> &p_attribute) const;

	// --- Tag Management ---
	void add_tag(const StringName &p_tag);
	void remove_tag(const StringName &p_tag);
	bool has_tag(const StringName &p_tag) const;
	Ref<AbilitySystemTagSpec> get_owned_tags() const;

	// --- Cue Activation API ---
	bool can_activate_cue_by_tag(const StringName &p_tag);
	bool try_activate_cue_by_tag(const StringName &p_tag, const Dictionary &p_data = Dictionary(), Object *p_target_node = nullptr);
	void cancel_cue_by_tag(const StringName &p_tag);

	bool can_activate_cue_by_resource(const Ref<AbilitySystemCue> &p_cue);
	bool try_activate_cue_by_resource(const Ref<AbilitySystemCue> &p_cue, const Dictionary &p_data = Dictionary(), Object *p_target_node = nullptr);
	void cancel_cue_by_resource(const Ref<AbilitySystemCue> &p_cue);

	void _execute_cue_with_spec(const StringName &p_tag, Ref<AbilitySystemCueSpec> p_spec);

	void register_cue_resource(Ref<AbilitySystemCue> p_cue);
	void unregister_cue_resource(const StringName &p_tag);
	Ref<AbilitySystemCue> get_cue_resource(const StringName &p_tag) const;

	// --- Task & Montage API ---
	void register_task(Ref<AbilitySystemTask> p_task);
	void play_montage(const StringName &p_name, Node *p_target = nullptr);
	bool is_montage_playing(const StringName &p_name, Node *p_target = nullptr) const;
	void play_sound(Ref<AudioStream> p_sound, Node *p_target = nullptr);

	// --- Node Registration ---
	void register_node(const StringName &p_name, Node *p_node);
	void unregister_node(const StringName &p_name);
	Node *get_node_ptr(const StringName &p_name) const;

	void set_animation_player(Node *p_node) { animation_player_node = p_node; }
	Node *get_animation_player() const { return animation_player_node; }

	void set_audio_player(Node *p_node) { audio_player_node = p_node; }
	Node *get_audio_player() const { return audio_player_node; }

	// --- Logic & Calculations ---
	float calculate_modifier_magnitude(const Ref<AbilitySystemEffectSpec> &p_spec, int p_modifier_idx) const;
	float calculate_effect_duration(const Ref<AbilitySystemEffectSpec> &p_spec) const;

	void set_ability_container(Ref<AbilitySystemAbilityContainer> p_container);
	Ref<AbilitySystemAbilityContainer> get_ability_container() const;

	// --- Debug Helpers ---
	TypedArray<AbilitySystemEffectSpec> get_active_effects_debug() const;
	TypedArray<AbilitySystemAbilitySpec> get_granted_abilities_debug() const;

	// Virtual callback for custom magnitudes from scripts
	GDVIRTUAL2RC(float, _on_calculate_custom_magnitude, Ref<AbilitySystemEffectSpec>, int);

	AbilitySystemComponent();
	~AbilitySystemComponent();
};

} // namespace godot

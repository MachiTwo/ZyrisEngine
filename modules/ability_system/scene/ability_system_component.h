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

#include "scene/main/node.h"

class AbilitySystemAttributeSet;
class AbilitySystemTagSpec;

class AbilitySystemAbility;
class AbilitySystemAbilitySpec;
class AbilitySystemEffect;
class AudioStream;
class AbilitySystemEffectSpec;
class AbilitySystemTask;
class AbilitySystemCue;
class AbilitySystemCueSpec;
class AbilitySystemAbilityContainer;

class CharacterBody2D;
class CharacterBody3D;
class AnimationPlayer;
class AnimatedSprite2D;
class AnimatedSprite3D;
class AudioStreamPlayer;
class AudioStreamPlayer2D;
class AudioStreamPlayer3D;
class Marker2D;
class Marker3D;

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

protected:
	static void _bind_methods();
	void _notification(int p_what);

	void _on_attribute_set_attribute_changed(const StringName &p_name, float p_old_val, float p_new_val);
	void _update_attribute_current_values();

	void _process_effects(float p_delta);
	void _process_tasks(float p_delta);
	void _remove_effect_at_index(int p_idx);

public:
	// API intended for GDScript usage as seen in tutorials
	void give_ability(Ref<AbilitySystemAbility> p_ability, int p_level = 1);
	void apply_ability_container(Ref<AbilitySystemAbilityContainer> p_container, int p_level = 1);
	bool try_activate_ability(Ref<AbilitySystemAbility> p_ability);

	Ref<AbilitySystemEffectSpec> make_outgoing_spec(Ref<AbilitySystemEffect> p_effect, float p_level = 1.0f);
	void apply_effect_spec_to_self(Ref<AbilitySystemEffectSpec> p_spec);
	void apply_effect_spec_to_target(Ref<AbilitySystemEffectSpec> p_spec, AbilitySystemComponent *p_target);

	void remove_granted_ability(Ref<AbilitySystemAbilitySpec> p_spec);
	void remove_active_effect(Ref<AbilitySystemEffectSpec> p_spec);

	void register_task(Ref<AbilitySystemTask> p_task);
	void play_montage(const StringName &p_name, Node *p_target = nullptr);
	bool is_montage_playing(const StringName &p_name, Node *p_target = nullptr) const;
	void play_sound(Ref<AudioStream> p_sound, Node *p_target = nullptr);
	void execute_cue(const StringName &p_tag, const Dictionary &p_data = Dictionary());
	void _execute_cue_with_spec(const StringName &p_tag, Ref<AbilitySystemCueSpec> p_spec);

	void register_node(const StringName &p_name, Node *p_node);
	void unregister_node(const StringName &p_name);
	Node *get_node_ptr(const StringName &p_name) const;

	void set_animation_player(Node *p_node) { animation_player_node = p_node; }
	Node *get_animation_player() const { return animation_player_node; }

	void set_audio_player(Node *p_node) { audio_player_node = p_node; }
	Node *get_audio_player() const { return audio_player_node; }

	void add_tag(const StringName &p_tag);
	void remove_tag(const StringName &p_tag);
	bool has_tag(const StringName &p_tag) const;

	float get_attribute_value(const StringName &p_attribute_name) const;
	void set_attribute_base_value(const StringName &p_name, float p_value);
	void add_attribute_set(Ref<AbilitySystemAttributeSet> p_set);

	void register_cue_resource(Ref<class AbilitySystemCue> p_cue);
	void unregister_cue_resource(const StringName &p_tag);
	Ref<class AbilitySystemCue> get_cue_resource(const StringName &p_tag) const;

	Ref<AbilitySystemTagSpec> get_owned_tags() const;

	TypedArray<AbilitySystemEffectSpec> get_active_effects_debug() const;
	TypedArray<AbilitySystemAbilitySpec> get_granted_abilities_debug() const;

	void set_ability_container(Ref<AbilitySystemAbilityContainer> p_container);
	Ref<AbilitySystemAbilityContainer> get_ability_container() const;

	virtual PackedStringArray get_configuration_warnings() const override;

	AbilitySystemComponent();
	~AbilitySystemComponent();
};

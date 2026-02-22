/**************************************************************************/
/*  ability_system_component.cpp                                          */
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

#include "modules/ability_system/scene/ability_system_component.h"
#include "modules/ability_system/core/ability_system.h"
#include "modules/ability_system/core/ability_system_ability_spec.h"
#include "modules/ability_system/core/ability_system_cue_spec.h"
#include "modules/ability_system/core/ability_system_effect_spec.h"
#include "modules/ability_system/core/ability_system_tag_spec.h"
#include "modules/ability_system/core/ability_system_task.h"
#include "modules/ability_system/resources/ability_system_ability_container.h"
#include "modules/ability_system/resources/ability_system_attribute_set.h"
#include "modules/ability_system/resources/ability_system_cue.h"

#include "scene/2d/animated_sprite_2d.h"
#include "scene/2d/audio_stream_player_2d.h"
#include "scene/2d/physics/character_body_2d.h"
#include "scene/3d/audio_stream_player_3d.h"
#include "scene/3d/physics/character_body_3d.h"
#include "scene/3d/sprite_3d.h"
#include "scene/animation/animation_player.h"
#include "scene/audio/audio_stream_player.h"
#include "servers/audio/audio_stream.h"

void AbilitySystemComponent::_bind_methods() {
	// Ability Container — top-level, always visible (no collapsible group).
	ClassDB::bind_method(D_METHOD("set_ability_container", "container"), &AbilitySystemComponent::set_ability_container);
	ClassDB::bind_method(D_METHOD("get_ability_container"), &AbilitySystemComponent::get_ability_container);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "ability_container", PROPERTY_HINT_RESOURCE_TYPE, "AbilitySystemAbilityContainer"), "set_ability_container", "get_ability_container");

	// API methods
	ClassDB::bind_method(D_METHOD("give_ability", "ability", "level"), &AbilitySystemComponent::give_ability, DEFVAL(1));
	ClassDB::bind_method(D_METHOD("apply_ability_container", "container", "level"), &AbilitySystemComponent::apply_ability_container, DEFVAL(1));
	ClassDB::bind_method(D_METHOD("try_activate_ability", "ability"), &AbilitySystemComponent::try_activate_ability);
	ClassDB::bind_method(D_METHOD("make_outgoing_spec", "effect", "level"), &AbilitySystemComponent::make_outgoing_spec, DEFVAL(1.0f));
	ClassDB::bind_method(D_METHOD("apply_effect_spec_to_self", "spec"), &AbilitySystemComponent::apply_effect_spec_to_self);
	ClassDB::bind_method(D_METHOD("apply_effect_spec_to_target", "spec", "target"), &AbilitySystemComponent::apply_effect_spec_to_target);
	ClassDB::bind_method(D_METHOD("remove_granted_ability", "spec"), &AbilitySystemComponent::remove_granted_ability);
	ClassDB::bind_method(D_METHOD("remove_active_effect", "spec"), &AbilitySystemComponent::remove_active_effect);

	ClassDB::bind_method(D_METHOD("register_task", "task"), &AbilitySystemComponent::register_task);
	ClassDB::bind_method(D_METHOD("play_montage", "name", "target"), &AbilitySystemComponent::play_montage, DEFVAL(Variant()));
	ClassDB::bind_method(D_METHOD("is_montage_playing", "name", "target"), &AbilitySystemComponent::is_montage_playing, DEFVAL(Variant()));
	ClassDB::bind_method(D_METHOD("play_sound", "sound", "target"), &AbilitySystemComponent::play_sound, DEFVAL(Variant()));

	ClassDB::bind_method(D_METHOD("register_node", "name", "node"), &AbilitySystemComponent::register_node);
	ClassDB::bind_method(D_METHOD("unregister_node", "name"), &AbilitySystemComponent::unregister_node);
	ClassDB::bind_method(D_METHOD("get_node_ptr", "name"), &AbilitySystemComponent::get_node_ptr);

	ClassDB::bind_method(D_METHOD("set_animation_player", "node"), &AbilitySystemComponent::set_animation_player);
	ClassDB::bind_method(D_METHOD("get_animation_player"), &AbilitySystemComponent::get_animation_player);
	ClassDB::bind_method(D_METHOD("set_audio_player", "node"), &AbilitySystemComponent::set_audio_player);
	ClassDB::bind_method(D_METHOD("get_audio_player"), &AbilitySystemComponent::get_audio_player);

	ClassDB::bind_method(D_METHOD("execute_cue", "tag", "data"), &AbilitySystemComponent::execute_cue, DEFVAL(Dictionary()));

	// Helper methods for Cue system
	ClassDB::bind_method(D_METHOD("add_tag", "tag"), &AbilitySystemComponent::add_tag);
	ClassDB::bind_method(D_METHOD("remove_tag", "tag"), &AbilitySystemComponent::remove_tag);
	ClassDB::bind_method(D_METHOD("has_tag", "tag"), &AbilitySystemComponent::has_tag);
	ClassDB::bind_method(D_METHOD("get_attribute_value", "attribute_name"), &AbilitySystemComponent::get_attribute_value);
	ClassDB::bind_method(D_METHOD("set_attribute_base_value", "name", "value"), &AbilitySystemComponent::set_attribute_base_value);
	ClassDB::bind_method(D_METHOD("add_attribute_set", "set"), &AbilitySystemComponent::add_attribute_set);
	ClassDB::bind_method(D_METHOD("register_cue_resource", "cue"), &AbilitySystemComponent::register_cue_resource);
	ClassDB::bind_method(D_METHOD("unregister_cue_resource", "tag"), &AbilitySystemComponent::unregister_cue_resource);
	ClassDB::bind_method(D_METHOD("get_cue_resource", "tag"), &AbilitySystemComponent::get_cue_resource);
	ClassDB::bind_method(D_METHOD("get_owned_tags"), &AbilitySystemComponent::get_owned_tags);
	ClassDB::bind_method(D_METHOD("get_active_effects_debug"), &AbilitySystemComponent::get_active_effects_debug);
	ClassDB::bind_method(D_METHOD("get_granted_abilities_debug"), &AbilitySystemComponent::get_granted_abilities_debug);

	ADD_SIGNAL(MethodInfo("attribute_changed", PropertyInfo(Variant::STRING_NAME, "attribute_name"), PropertyInfo(Variant::FLOAT, "old_value"), PropertyInfo(Variant::FLOAT, "new_value")));
	ADD_SIGNAL(MethodInfo("tag_changed", PropertyInfo(Variant::STRING_NAME, "tag_name"), PropertyInfo(Variant::BOOL, "is_present")));
	ADD_SIGNAL(MethodInfo("ability_activated", PropertyInfo(Variant::OBJECT, "ability_spec", PROPERTY_HINT_RESOURCE_TYPE, "AbilitySystemAbilitySpec")));
	ADD_SIGNAL(MethodInfo("ability_ended", PropertyInfo(Variant::OBJECT, "ability_spec", PROPERTY_HINT_RESOURCE_TYPE, "AbilitySystemAbilitySpec"), PropertyInfo(Variant::BOOL, "was_cancelled")));
	ADD_SIGNAL(MethodInfo("ability_failed", PropertyInfo(Variant::STRING_NAME, "ability_name"), PropertyInfo(Variant::STRING, "reason")));

	ADD_SIGNAL(MethodInfo("effect_applied", PropertyInfo(Variant::OBJECT, "effect_spec", PROPERTY_HINT_RESOURCE_TYPE, "AbilitySystemEffectSpec")));
	ADD_SIGNAL(MethodInfo("effect_removed", PropertyInfo(Variant::OBJECT, "effect_spec", PROPERTY_HINT_RESOURCE_TYPE, "AbilitySystemEffectSpec")));

	ADD_SIGNAL(MethodInfo("tag_event_received", PropertyInfo(Variant::STRING_NAME, "event_tag"), PropertyInfo(Variant::DICTIONARY, "data")));

	ADD_SIGNAL(MethodInfo("cooldown_started", PropertyInfo(Variant::STRING_NAME, "ability_tag"), PropertyInfo(Variant::FLOAT, "duration")));
	ADD_SIGNAL(MethodInfo("cooldown_ended", PropertyInfo(Variant::STRING_NAME, "ability_tag")));
}

void AbilitySystemComponent::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			// Validate parent is CharacterBody2D or CharacterBody3D
			Node *parent = get_parent();
			if (!parent || (!Object::cast_to<CharacterBody2D>(parent) && !Object::cast_to<CharacterBody3D>(parent))) {
				ERR_PRINT("AbilitySystemComponent FATAL: Can only be child of CharacterBody2D or CharacterBody3D. Disabling component.");
				set_process(false);
				return;
			}

			// Cache the validated CharacterBody parent
			character_body_2d = Object::cast_to<CharacterBody2D>(parent);
			character_body_3d = Object::cast_to<CharacterBody3D>(parent);

			if (ability_container.is_valid()) {
				apply_ability_container(ability_container);
			}
			set_process(true);
		} break;
		case NOTIFICATION_PROCESS: {
			float delta = get_process_delta_time();
			_process_effects(delta);
			_process_tasks(delta);
		} break;
	}
}

void AbilitySystemComponent::_process_effects(float p_delta) {
	bool removed_any = false;
	bool recalculate_needed = false;

	for (int i = active_effects.size() - 1; i >= 0; i--) {
		Ref<AbilitySystemEffectSpec> spec = active_effects[i];
		Ref<AbilitySystemEffect> effect = spec->get_effect();

		// --- Duration countdown ---
		if (effect->get_duration_policy() == AbilitySystemEffect::DURATION) {
			float remaining = spec->get_duration_remaining() - p_delta;
			spec->set_duration_remaining(remaining);
			if (remaining <= 0) {
				_remove_effect_at_index(i);
				removed_any = true;
				continue;
			}
		}

		// --- Periodic tick ---
		if (effect->get_period() > 0.0f) {
			float timer = spec->get_period_timer() - p_delta;
			if (timer <= 0.0f) {
				timer += effect->get_period();
				// Apply instant-style attribute modification for this tick
				for (int j = 0; j < effect->get_modifier_count(); j++) {
					StringName attr = effect->get_modifier_attribute(j);
					AbilitySystemEffect::ModifierOp op = effect->get_modifier_operation(j);
					float mag = spec->calculate_modifier_magnitude(j);
					float current = get_attribute_value(attr);
					float next = current;
					switch (op) {
						case AbilitySystemEffect::ADD:
							next += mag;
							break;
						case AbilitySystemEffect::MULTIPLY:
							next *= mag;
							break;
						case AbilitySystemEffect::DIVIDE:
							if (mag != 0) {
								next /= mag;
							}
							break;
						case AbilitySystemEffect::OVERRIDE:
							next = mag;
							break;
					}
					set_attribute_base_value(attr, next);
				}
				recalculate_needed = true;
			}
			spec->set_period_timer(timer);
		}
	}

	if (removed_any || recalculate_needed) {
		_update_attribute_current_values();
	}
}

void AbilitySystemComponent::_process_tasks(float p_delta) {
	for (int i = active_tasks.size() - 1; i >= 0; i--) {
		Ref<AbilitySystemTask> task = active_tasks[i];
		if (task->is_finished()) {
			active_tasks.remove_at(i);
			continue;
		}
		task->tick(p_delta);
		if (task->is_finished()) {
			active_tasks.remove_at(i);
		}
	}
}

void AbilitySystemComponent::register_task(Ref<AbilitySystemTask> p_task) {
	ERR_FAIL_COND(p_task.is_null());
	if (active_tasks.find(p_task) != -1) {
		return; // Already active
	}
	active_tasks.push_back(p_task);
	p_task->activate();
}

void AbilitySystemComponent::_remove_effect_at_index(int p_idx) {
	Ref<AbilitySystemEffectSpec> spec = active_effects[p_idx];
	Ref<AbilitySystemEffect> effect = spec->get_effect();

	// Remove tags
	TypedArray<StringName> granted = effect->get_granted_tags();
	for (int i = 0; i < granted.size(); i++) {
		remove_tag(granted[i]);
	}

	// Trigger Cues (Tags and Direct)
	{
		// 1. Cue Tags (Registry based)
		StringName effect_tag = effect->get_effect_tag();
		if (effect_tag != StringName()) {
			Ref<AbilitySystemCue> cue = get_cue_resource(effect_tag);
			if (cue.is_null() && spec->get_source_component()) {
				cue = spec->get_source_component()->get_cue_resource(effect_tag);
			}

			if (cue.is_valid() && cue->get_event_type() == AbilitySystemCue::ON_REMOVE) {
				Ref<AbilitySystemCueSpec> cue_spec;
				cue_spec.instantiate();
				cue_spec->init_from_effect(cue, spec->get_source_component(), this, spec, 0.0f);
				_execute_cue_with_spec(effect_tag, cue_spec);
			} else if (cue.is_null()) {
				emit_signal("tag_event_received", effect_tag, Dictionary());
			}
		}

		// 2. Direct Cues (Resource based)
		TypedArray<AbilitySystemCue> cues = effect->get_cues();
		for (int i = 0; i < cues.size(); i++) {
			Ref<AbilitySystemCue> cue = cues[i];
			if (cue.is_valid() && cue->get_event_type() == AbilitySystemCue::ON_REMOVE) {
				Ref<AbilitySystemCueSpec> cue_spec;
				cue_spec.instantiate();
				cue_spec->init_from_effect(cue, spec->get_source_component(), this, spec, 0.0f);
				cue->execute(cue_spec);
			}
		}
	}

	active_effects.remove_at(p_idx);
	emit_signal("effect_removed", spec);

	// Recalculate if duration/infinite
	if (effect->get_duration_policy() != AbilitySystemEffect::INSTANT) {
		_update_attribute_current_values();
	}
}

void AbilitySystemComponent::remove_active_effect(Ref<AbilitySystemEffectSpec> p_spec) {
	ERR_FAIL_COND(p_spec.is_null());
	int idx = active_effects.find(p_spec);
	if (idx != -1) {
		_remove_effect_at_index(idx);
		_update_attribute_current_values();
	}
}

void AbilitySystemComponent::remove_granted_ability(Ref<AbilitySystemAbilitySpec> p_spec) {
	ERR_FAIL_COND(p_spec.is_null());
	int idx = granted_abilities.find(p_spec);
	if (idx != -1) {
		granted_abilities.remove_at(idx);
	}
}

void AbilitySystemComponent::give_ability(Ref<AbilitySystemAbility> p_ability, int p_level) {
	ERR_FAIL_COND(p_ability.is_null());
	Ref<AbilitySystemAbilitySpec> spec;
	spec.instantiate();
	spec->init(p_ability, p_level);
	granted_abilities.push_back(spec);
}

void AbilitySystemComponent::apply_ability_container(Ref<AbilitySystemAbilityContainer> p_container, int p_level) {
	ERR_FAIL_COND(p_container.is_null());

	// 1. Initialize from AttributeSet (grants abilities and sets initial attributes)
	Ref<AbilitySystemAttributeSet> attr_set = p_container->get_attribute_set();
	if (attr_set.is_valid()) {
		// Register the AttributeSet
		add_attribute_set(attr_set);

		// Grant abilities defined by the AttributeSet (already unlocked)
		TypedArray<AbilitySystemAbility> granted = attr_set->get_granted_abilities();
		for (int i = 0; i < granted.size(); i++) {
			Ref<AbilitySystemAbility> ability = granted[i];
			if (ability.is_valid()) {
				give_ability(ability, p_level);
			}
		}
	}

	// 2. Apply Effects
	TypedArray<AbilitySystemEffect> effects = p_container->get_effects();
	for (int i = 0; i < effects.size(); i++) {
		Ref<AbilitySystemEffect> effect = effects[i];
		if (effect.is_valid()) {
			Ref<AbilitySystemEffectSpec> spec = make_outgoing_spec(effect, p_level);
			apply_effect_spec_to_self(spec);
		}
	}

	// 3. Apply Tags
	TypedArray<StringName> tags = p_container->get_tags();
	for (int i = 0; i < tags.size(); i++) {
		add_tag(tags[i]);
	}

	// 4. Register Cues
	TypedArray<AbilitySystemCue> cues = p_container->get_cues();
	for (int i = 0; i < cues.size(); i++) {
		Ref<AbilitySystemCue> cue = cues[i];
		if (cue.is_valid()) {
			register_cue_resource(cue);
		}
	}

	// Force update of current values after setting base values
	_update_attribute_current_values();
}

bool AbilitySystemComponent::try_activate_ability(Ref<AbilitySystemAbility> p_ability) {
	ERR_FAIL_COND_V(p_ability.is_null(), false);
	for (int i = 0; i < granted_abilities.size(); i++) {
		Ref<AbilitySystemAbilitySpec> spec = granted_abilities[i];
		if (spec->get_ability() == p_ability) {
			if (p_ability->can_activate_ability(this, spec)) {
				p_ability->activate_ability(this, spec);
				emit_signal("ability_activated", spec);
				return true;
			}
		}
	}
	return false;
}

Ref<AbilitySystemEffectSpec> AbilitySystemComponent::make_outgoing_spec(Ref<AbilitySystemEffect> p_effect, float p_level) {
	ERR_FAIL_COND_V(p_effect.is_null(), Ref<AbilitySystemEffectSpec>());

	// Validation: Outgoing effects must be registered in the source's container (Archetype Blueprint).
	if (ability_container.is_valid()) {
		if (!ability_container->has_effect(p_effect)) {
			// Also check currently granted/unlocked abilities (cooldowns/costs)
			bool found = false;
			for (int i = 0; i < granted_abilities.size(); i++) {
				Ref<AbilitySystemAbility> ability = granted_abilities[i]->get_ability();
				if (ability.is_valid() && (ability->get_cost_effect() == p_effect || ability->get_cooldown_effect() == p_effect)) {
					found = true;
					break;
				}
			}

			if (!found) {
				ERR_PRINT(vformat("AbilitySystemComponent Error: Effect '%s' is NOT registered in the source's AbilityContainer. Outgoing effects (attacks/buffs) must be part of the archetype contract.", p_effect->get_path()));
			}
		}
	}

	Ref<AbilitySystemEffectSpec> spec;
	spec.instantiate();
	spec->init(p_effect, p_level);
	spec->set_source_component(this);
	return spec;
}

void AbilitySystemComponent::apply_effect_spec_to_target(Ref<AbilitySystemEffectSpec> p_spec, AbilitySystemComponent *p_target) {
	ERR_FAIL_COND(p_spec.is_null());
	ERR_FAIL_COND(p_target == nullptr);

	p_target->apply_effect_spec_to_self(p_spec);
}

void AbilitySystemComponent::apply_effect_spec_to_self(Ref<AbilitySystemEffectSpec> p_spec) {
	ERR_FAIL_COND(p_spec.is_null());
	Ref<AbilitySystemEffect> effect = p_spec->get_effect();
	ERR_FAIL_COND(effect.is_null());

	p_spec->set_target_component(this);

	if (effect->get_duration_policy() == AbilitySystemEffect::INSTANT) {
		// Instant effects modify Base Value permanently
		for (int i = 0; i < effect->get_modifier_count(); i++) {
			StringName attr = effect->get_modifier_attribute(i);
			AbilitySystemEffect::ModifierOp op = effect->get_modifier_operation(i);
			float mag = p_spec->calculate_modifier_magnitude(i);

			float current = get_attribute_value(attr);
			float next = current;

			switch (op) {
				case AbilitySystemEffect::ADD:
					next += mag;
					break;
				case AbilitySystemEffect::MULTIPLY:
					next *= mag;
					break;
				case AbilitySystemEffect::DIVIDE:
					if (mag != 0) {
						next /= mag;
					}
					break;
				case AbilitySystemEffect::OVERRIDE:
					next = mag;
					break;
			}

			set_attribute_base_value(attr, next);
		}
	} else {
		// Duration/Infinite effects: handle stacking before adding to the active list.
		AbilitySystemEffect::StackingPolicy stacking = effect->get_stacking_policy();

		if (stacking != AbilitySystemEffect::STACK_NEW_INSTANCE) {
			// Search for an existing active effect from the same resource.
			for (int i = 0; i < active_effects.size(); i++) {
				if (active_effects[i]->get_effect() == effect) {
					Ref<AbilitySystemEffectSpec> existing = active_effects[i];

					switch (stacking) {
						case AbilitySystemEffect::STACK_OVERRIDE:
							// Reset duration to new spec's full duration.
							existing->set_duration_remaining(p_spec->get_total_duration());
							if (effect->get_period() > 0.0f) {
								existing->set_period_timer(effect->get_period());
							}
							break;
						case AbilitySystemEffect::STACK_INTENSITY:
							existing->set_stack_count(existing->get_stack_count() + 1);
							break;
						case AbilitySystemEffect::STACK_DURATION:
							existing->set_duration_remaining(existing->get_duration_remaining() + p_spec->get_total_duration());
							break;
						default:
							break;
					}
					// Stacking applied — do NOT add a new spec entry.
					_update_attribute_current_values();
					goto finish_cues;
				}
			}
		}

		// No existing stack found (or policy is NewInstance): add as new.
		active_effects.push_back(p_spec);

		// Apply immediate periodic tick on application if configured.
		if (effect->get_period() > 0.0f) {
			p_spec->set_period_timer(effect->get_period());
		}

		// Apply tags.
		TypedArray<StringName> granted = effect->get_granted_tags();
		for (int i = 0; i < granted.size(); i++) {
			add_tag(granted[i]);
		}

		_update_attribute_current_values();
	}

finish_cues:
	// Trigger Cues (ON_EXECUTE / ON_ACTIVE)
	{
		float representative_magnitude = 0.0f;
		for (int j = 0; j < effect->get_modifier_count(); j++) {
			representative_magnitude += p_spec->calculate_modifier_magnitude(j);
		}

		// 1. Cue Tags (Registry based)
		StringName effect_tag = effect->get_effect_tag();
		if (effect_tag != StringName()) {
			Ref<AbilitySystemCue> cue = get_cue_resource(effect_tag);
			if (cue.is_null() && p_spec->get_source_component()) {
				cue = p_spec->get_source_component()->get_cue_resource(effect_tag);
			}

			if (cue.is_valid() && cue->get_event_type() != AbilitySystemCue::ON_REMOVE) {
				Ref<AbilitySystemCueSpec> cue_spec;
				cue_spec.instantiate();
				cue_spec->init_from_effect(cue, p_spec->get_source_component(), this, p_spec, representative_magnitude);
				_execute_cue_with_spec(effect_tag, cue_spec);
			} else if (cue.is_null()) {
				emit_signal("tag_event_received", effect_tag, Dictionary());
			}
		}

		// 2. Direct Cues (Resource based)
		TypedArray<AbilitySystemCue> cues = effect->get_cues();
		for (int i = 0; i < cues.size(); i++) {
			Ref<AbilitySystemCue> cue = cues[i];
			if (cue.is_valid() && cue->get_event_type() != AbilitySystemCue::ON_REMOVE) {
				Ref<AbilitySystemCueSpec> cue_spec;
				cue_spec.instantiate();
				cue_spec->init_from_effect(cue, p_spec->get_source_component(), this, p_spec, representative_magnitude);
				cue->execute(cue_spec);
			}
		}
	}

	emit_signal("effect_applied", p_spec);
}

void AbilitySystemComponent::play_montage(const StringName &p_name, Node *p_target) {
	if (!p_target) {
		p_target = animation_player_node;
	}
	if (!p_target) {
		return;
	}

	if (AnimationPlayer *ap = Object::cast_to<AnimationPlayer>(p_target)) {
		ap->play(p_name);
	} else if (AnimatedSprite2D *as2 = Object::cast_to<AnimatedSprite2D>(p_target)) {
		as2->play(p_name);
	} else if (AnimatedSprite3D *as3 = Object::cast_to<AnimatedSprite3D>(p_target)) {
		as3->play(p_name);
	}
}

bool AbilitySystemComponent::is_montage_playing(const StringName &p_name, Node *p_target) const {
	if (!p_target) {
		p_target = animation_player_node;
	}
	if (!p_target) {
		return false;
	}

	if (AnimationPlayer *ap = Object::cast_to<AnimationPlayer>(p_target)) {
		return ap->is_playing() && ap->get_current_animation() == p_name;
	} else if (AnimatedSprite2D *as2 = Object::cast_to<AnimatedSprite2D>(p_target)) {
		return as2->is_playing() && as2->get_animation() == p_name;
	} else if (AnimatedSprite3D *as3 = Object::cast_to<AnimatedSprite3D>(p_target)) {
		return as3->is_playing() && as3->get_animation() == p_name;
	}
	return false;
}

void AbilitySystemComponent::play_sound(Ref<AudioStream> p_sound, Node *p_target) {
	ERR_FAIL_COND(p_sound.is_null());

	if (!p_target) {
		p_target = audio_player_node;
	}
	if (!p_target) {
		return;
	}

	if (AudioStreamPlayer *ap = Object::cast_to<AudioStreamPlayer>(p_target)) {
		ap->set_stream(p_sound);
		ap->play();
	} else if (AudioStreamPlayer2D *ap2 = Object::cast_to<AudioStreamPlayer2D>(p_target)) {
		ap2->set_stream(p_sound);
		ap2->play();
	} else if (AudioStreamPlayer3D *ap3 = Object::cast_to<AudioStreamPlayer3D>(p_target)) {
		ap3->set_stream(p_sound);
		ap3->play();
	}
}

void AbilitySystemComponent::register_node(const StringName &p_name, Node *p_node) {
	registered_nodes[p_name] = p_node;
}

void AbilitySystemComponent::unregister_node(const StringName &p_name) {
	registered_nodes.erase(p_name);
}

Node *AbilitySystemComponent::get_node_ptr(const StringName &p_name) const {
	if (registered_nodes.has(p_name)) {
		return registered_nodes[p_name];
	}
	return nullptr;
}

void AbilitySystemComponent::execute_cue(const StringName &p_tag, const Dictionary &p_data) {
	Ref<AbilitySystemCue> cue = get_cue_resource(p_tag);
	if (cue.is_null()) {
		// Emit raw event even with no Cue resource registered — lets external
		// managers (AnimationTree, VFX systems) react via the signal.
		emit_signal("tag_event_received", p_tag, p_data);
		return;
	}

	Ref<AbilitySystemCueSpec> spec;
	spec.instantiate();
	spec->init_manual(cue, this, p_data);

	_execute_cue_with_spec(p_tag, spec);
}

void AbilitySystemComponent::_execute_cue_with_spec(const StringName &p_tag, Ref<AbilitySystemCueSpec> p_spec) {
	// Emit the generic event first so external managers always get notified.
	emit_signal("tag_event_received", p_tag, p_spec->get_extra_data());

	// Dispatch to the typed Cue resource virtual.
	Ref<AbilitySystemCue> cue = p_spec->get_cue();
	if (cue.is_valid()) {
		cue->execute(p_spec);
	}
}

void AbilitySystemComponent::register_cue_resource(Ref<AbilitySystemCue> p_cue) {
	ERR_FAIL_COND(p_cue.is_null());
	StringName tag = p_cue->get_cue_tag();
	if (tag != StringName()) {
		// Replace if already exists or just add
		for (int i = 0; i < registered_cues.size(); i++) {
			if (registered_cues[i]->get_cue_tag() == tag) {
				registered_cues.write[i] = p_cue;
				return;
			}
		}
		registered_cues.push_back(p_cue);
	}
}

void AbilitySystemComponent::unregister_cue_resource(const StringName &p_tag) {
	for (int i = 0; i < registered_cues.size(); i++) {
		if (registered_cues[i]->get_cue_tag() == p_tag) {
			registered_cues.remove_at(i);
			return;
		}
	}
}

Ref<AbilitySystemCue> AbilitySystemComponent::get_cue_resource(const StringName &p_tag) const {
	for (int i = 0; i < registered_cues.size(); i++) {
		if (registered_cues[i]->get_cue_tag() == p_tag) {
			return registered_cues[i];
		}
	}
	return Ref<AbilitySystemCue>();
}

void AbilitySystemComponent::add_tag(const StringName &p_tag) {
	if (owned_tags->add_tag(p_tag)) {
		emit_signal("tag_changed", p_tag, true);
	}
}

void AbilitySystemComponent::remove_tag(const StringName &p_tag) {
	if (owned_tags->remove_tag(p_tag)) {
		emit_signal("tag_changed", p_tag, false);
	}
}

bool AbilitySystemComponent::has_tag(const StringName &p_tag) const {
	return owned_tags->has_tag(p_tag);
}

float AbilitySystemComponent::get_attribute_value(const StringName &p_attribute_name) const {
	for (int i = 0; i < attribute_sets.size(); i++) {
		if (attribute_sets[i]->has_attribute(p_attribute_name)) {
			return attribute_sets[i]->get_attribute_current_value(p_attribute_name);
		}
	}
	return 0.0f;
}

void AbilitySystemComponent::set_attribute_base_value(const StringName &p_name, float p_value) {
	for (int i = 0; i < attribute_sets.size(); i++) {
		if (attribute_sets[i]->has_attribute(p_name)) {
			attribute_sets[i]->set_attribute_base_value(p_name, p_value);
			return;
		}
	}
}

void AbilitySystemComponent::add_attribute_set(Ref<AbilitySystemAttributeSet> p_set) {
	ERR_FAIL_COND(p_set.is_null());
	attribute_sets.push_back(p_set);
	// Connect to changed signals
	if (p_set->is_connected("attribute_changed", callable_mp(this, &AbilitySystemComponent::_on_attribute_set_attribute_changed))) {
		return; // Already connected
	}
	p_set->connect("attribute_changed", callable_mp(this, &AbilitySystemComponent::_on_attribute_set_attribute_changed));
}

void AbilitySystemComponent::_on_attribute_set_attribute_changed(const StringName &p_name, float p_old_val, float p_new_val) {
	emit_signal("attribute_changed", p_name, p_old_val, p_new_val);
}

void AbilitySystemComponent::_update_attribute_current_values() {
	// 1. Reset all current values to base values.
	for (int k = 0; k < attribute_sets.size(); k++) {
		attribute_sets[k]->reset_current_values();
	}

	// 2. Apply modifiers in a DETERMINISTIC ORDER: ADD -> MULTIPLY -> DIVIDE -> OVERRIDE.
	// This guarantees the same final attribute value regardless of effect application order.
	const AbilitySystemEffect::ModifierOp OP_ORDER[] = {
		AbilitySystemEffect::ADD,
		AbilitySystemEffect::MULTIPLY,
		AbilitySystemEffect::DIVIDE,
		AbilitySystemEffect::OVERRIDE
	};

	for (int pass = 0; pass < 4; pass++) {
		AbilitySystemEffect::ModifierOp current_op = OP_ORDER[pass];

		for (int i = 0; i < active_effects.size(); i++) {
			Ref<AbilitySystemEffectSpec> spec = active_effects[i];
			Ref<AbilitySystemEffect> effect = spec->get_effect();

			const float stack_multiplier = (effect->get_stacking_policy() == AbilitySystemEffect::STACK_INTENSITY)
					? (float)spec->get_stack_count()
					: 1.0f;

			for (int j = 0; j < effect->get_modifier_count(); j++) {
				if (effect->get_modifier_operation(j) != current_op) {
					continue;
				}

				// Skip periodic effects: they apply to base value directly via tick, not here.
				if (effect->get_period() > 0.0f) {
					continue;
				}

				StringName attr_name = effect->get_modifier_attribute(j);
				for (int k = 0; k < attribute_sets.size(); k++) {
					if (!attribute_sets[k]->has_attribute(attr_name)) {
						continue;
					}
					float current = attribute_sets[k]->get_attribute_current_value(attr_name);
					float mag = spec->calculate_modifier_magnitude(j) * stack_multiplier;
					float next = current;
					switch (current_op) {
						case AbilitySystemEffect::ADD:
							next += mag;
							break;
						case AbilitySystemEffect::MULTIPLY:
							next *= mag;
							break;
						case AbilitySystemEffect::DIVIDE:
							if (mag != 0) {
								next /= mag;
							}
							break;
						case AbilitySystemEffect::OVERRIDE:
							next = mag;
							break;
					}
					attribute_sets[k]->set_attribute_current_value(attr_name, next);
					break;
				}
			}
		}
	}
}

Ref<AbilitySystemTagSpec> AbilitySystemComponent::get_owned_tags() const {
	return owned_tags;
}

void AbilitySystemComponent::set_ability_container(Ref<AbilitySystemAbilityContainer> p_container) {
	ability_container = p_container;
	update_configuration_warnings();
}

Ref<AbilitySystemAbilityContainer> AbilitySystemComponent::get_ability_container() const {
	return ability_container;
}

TypedArray<AbilitySystemEffectSpec> AbilitySystemComponent::get_active_effects_debug() const {
	TypedArray<AbilitySystemEffectSpec> ret;
	for (int i = 0; i < active_effects.size(); i++) {
		ret.push_back(active_effects[i]);
	}
	return ret;
}

TypedArray<AbilitySystemAbilitySpec> AbilitySystemComponent::get_granted_abilities_debug() const {
	TypedArray<AbilitySystemAbilitySpec> ret;
	for (int i = 0; i < granted_abilities.size(); i++) {
		ret.push_back(granted_abilities[i]);
	}
	return ret;
}

PackedStringArray AbilitySystemComponent::get_configuration_warnings() const {
	PackedStringArray warnings = Node::get_configuration_warnings();

	Node *parent = get_parent();
	if (parent) {
		bool is_valid_parent = Object::cast_to<CharacterBody2D>(parent) || Object::cast_to<CharacterBody3D>(parent);
		if (!is_valid_parent) {
			warnings.push_back("ERROR: AbilitySystemComponent can ONLY be child of CharacterBody2D or CharacterBody3D. Current parent: " + parent->get_class());
		}
	} else {
		warnings.push_back("ERROR: AbilitySystemComponent requires CharacterBody2D or CharacterBody3D as parent.");
	}

	if (ability_container.is_null()) {
		warnings.push_back("AbilitySystemComponent requires an AbilitySystemAbilityContainer to be valid.");
	}

	return warnings;
}

AbilitySystemComponent::AbilitySystemComponent() {
	owned_tags.instantiate();
}

AbilitySystemComponent::~AbilitySystemComponent() {
}

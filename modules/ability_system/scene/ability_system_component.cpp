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

#ifdef ABILITY_SYSTEM_MODULE
#include "modules/ability_system/scene/ability_system_component.h"
#include "modules/ability_system/core/ability_system.h"
#include "modules/ability_system/core/ability_system_ability_spec.h"
#include "modules/ability_system/core/ability_system_cue_spec.h"
#include "modules/ability_system/core/ability_system_effect_spec.h"
#include "modules/ability_system/core/ability_system_tag_spec.h"
#include "modules/ability_system/core/ability_system_task.h"
#include "modules/ability_system/resources/ability_system_ability.h"
#include "modules/ability_system/resources/ability_system_ability_container.h"
#include "modules/ability_system/resources/ability_system_attribute.h"
#include "modules/ability_system/resources/ability_system_attribute_set.h"
#include "modules/ability_system/resources/ability_system_cue.h"
#include "modules/ability_system/resources/ability_system_effect.h"
#elif defined(ABILITY_SYSTEM_GDEXTENSION)
#include "src/core/ability_system.h"
#include "src/core/ability_system_ability_spec.h"
#include "src/core/ability_system_cue_spec.h"
#include "src/core/ability_system_effect_spec.h"
#include "src/core/ability_system_tag_spec.h"
#include "src/core/ability_system_task.h"
#include "src/resources/ability_system_ability.h"
#include "src/resources/ability_system_ability_container.h"
#include "src/resources/ability_system_attribute.h"
#include "src/resources/ability_system_attribute_set.h"
#include "src/resources/ability_system_cue.h"
#include "src/resources/ability_system_effect.h"
#include "src/scene/ability_system_component.h"
#endif

#ifdef ABILITY_SYSTEM_MODULE
#include "core/config/project_settings.h"
#include "scene/2d/animated_sprite_2d.h"
#include "scene/2d/audio_stream_player_2d.h"
#include "scene/2d/physics/character_body_2d.h"
#include "scene/3d/audio_stream_player_3d.h"
#include "scene/3d/physics/character_body_3d.h"
#include "scene/3d/sprite_3d.h"
#include "scene/animation/animation_player.h"
#include "scene/audio/audio_stream_player.h"
#include "servers/audio/audio_stream.h"
#elif defined(ABILITY_SYSTEM_GDEXTENSION)
#include <godot_cpp/classes/animated_sprite2d.hpp>
#include <godot_cpp/classes/animated_sprite3d.hpp>
#include <godot_cpp/classes/animation_player.hpp>
#include <godot_cpp/classes/audio_stream.hpp>
#include <godot_cpp/classes/audio_stream_player.hpp>
#include <godot_cpp/classes/audio_stream_player2d.hpp>
#include <godot_cpp/classes/audio_stream_player3d.hpp>
#include <godot_cpp/classes/character_body2d.hpp>
#include <godot_cpp/classes/character_body3d.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/sprite3d.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#endif

namespace godot {

void AbilitySystemComponent::_bind_methods() {
	// --- Ability Container ---
	ClassDB::bind_method(D_METHOD("set_ability_container", "container"), &AbilitySystemComponent::set_ability_container);
	ClassDB::bind_method(D_METHOD("get_ability_container"), &AbilitySystemComponent::get_ability_container);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "ability_container", PROPERTY_HINT_RESOURCE_TYPE, "AbilitySystemAbilityContainer"), "set_ability_container", "get_ability_container");

	// --- Initialization API ---
	ClassDB::bind_method(D_METHOD("apply_ability_container", "container", "level"), &AbilitySystemComponent::apply_ability_container, DEFVAL(1));
	ClassDB::bind_method(D_METHOD("add_attribute_set", "set"), &AbilitySystemComponent::add_attribute_set);

	// --- Ability Management API ---
	ClassDB::bind_method(D_METHOD("give_ability_by_tag", "tag", "level"), &AbilitySystemComponent::give_ability_by_tag, DEFVAL(1));
	ClassDB::bind_method(D_METHOD("give_ability_by_resource", "ability", "level"), &AbilitySystemComponent::give_ability_by_resource, DEFVAL(1));
	ClassDB::bind_method(D_METHOD("remove_ability_by_tag", "tag"), &AbilitySystemComponent::remove_ability_by_tag);
	ClassDB::bind_method(D_METHOD("remove_ability_by_resource", "ability"), &AbilitySystemComponent::remove_ability_by_resource);
	ClassDB::bind_method(D_METHOD("remove_granted_ability_spec", "spec"), &AbilitySystemComponent::remove_granted_ability_spec);

	// --- Ability Activation API (By Tag) ---
	ClassDB::bind_method(D_METHOD("can_activate_ability_by_tag", "tag"), &AbilitySystemComponent::can_activate_ability_by_tag);
	ClassDB::bind_method(D_METHOD("try_activate_ability_by_tag", "tag"), &AbilitySystemComponent::try_activate_ability_by_tag);
	ClassDB::bind_method(D_METHOD("cancel_ability_by_tag", "tag"), &AbilitySystemComponent::cancel_ability_by_tag);

	// --- Ability Activation API (By Resource) ---
	ClassDB::bind_method(D_METHOD("can_activate_ability_by_resource", "ability"), &AbilitySystemComponent::can_activate_ability_by_resource);
	ClassDB::bind_method(D_METHOD("try_activate_ability_by_resource", "ability"), &AbilitySystemComponent::try_activate_ability_by_resource);
	ClassDB::bind_method(D_METHOD("cancel_ability_by_resource", "ability"), &AbilitySystemComponent::cancel_ability_by_resource);

	ClassDB::bind_method(D_METHOD("cancel_all_abilities"), &AbilitySystemComponent::cancel_all_abilities);

	// --- Effect Activation API (By Tag) ---
	ClassDB::bind_method(D_METHOD("can_activate_effect_by_tag", "tag"), &AbilitySystemComponent::can_activate_effect_by_tag);
	ClassDB::bind_method(D_METHOD("try_activate_effect_by_tag", "tag", "level", "target_node"), &AbilitySystemComponent::try_activate_effect_by_tag, DEFVAL(1.0f), DEFVAL(Variant()));
	ClassDB::bind_method(D_METHOD("cancel_effect_by_tag", "tag"), &AbilitySystemComponent::cancel_effect_by_tag);

	// --- Effect Activation API (By Resource) ---
	ClassDB::bind_method(D_METHOD("can_activate_effect_by_resource", "effect"), &AbilitySystemComponent::can_activate_effect_by_resource);
	ClassDB::bind_method(D_METHOD("try_activate_effect_by_resource", "effect", "level", "target_node"), &AbilitySystemComponent::try_activate_effect_by_resource, DEFVAL(1.0f), DEFVAL(Variant()));
	ClassDB::bind_method(D_METHOD("cancel_effect_by_resource", "effect"), &AbilitySystemComponent::cancel_effect_by_resource);

	// --- Effect Specification API (Low level) ---
	ClassDB::bind_method(D_METHOD("make_outgoing_spec", "effect", "level", "target_node"), &AbilitySystemComponent::make_outgoing_spec, DEFVAL(1.0f), DEFVAL(Variant()));
	ClassDB::bind_method(D_METHOD("apply_effect_spec_to_self", "spec"), &AbilitySystemComponent::apply_effect_spec_to_self);
	ClassDB::bind_method(D_METHOD("apply_effect_spec_to_target", "spec", "target"), &AbilitySystemComponent::apply_effect_spec_to_target);
	ClassDB::bind_method(D_METHOD("remove_active_effect", "spec"), &AbilitySystemComponent::remove_active_effect);
	ClassDB::bind_method(D_METHOD("remove_effect_by_tag", "tag"), &AbilitySystemComponent::remove_effect_by_tag);
	ClassDB::bind_method(D_METHOD("remove_effect_by_resource", "effect"), &AbilitySystemComponent::remove_effect_by_resource);
	ClassDB::bind_method(D_METHOD("apply_effect_by_tag", "tag", "level", "target_node"), &AbilitySystemComponent::apply_effect_by_tag, DEFVAL(1.0f), DEFVAL(Variant()));
	ClassDB::bind_method(D_METHOD("apply_effect_by_resource", "effect", "level", "target_node"), &AbilitySystemComponent::apply_effect_by_resource, DEFVAL(1.0f), DEFVAL(Variant()));
	ClassDB::bind_method(D_METHOD("has_active_effect_by_tag", "tag"), &AbilitySystemComponent::has_active_effect_by_tag);
	ClassDB::bind_method(D_METHOD("has_active_effect_by_resource", "effect"), &AbilitySystemComponent::has_active_effect_by_resource);

	// --- Attribute API (By Tag) ---
	ClassDB::bind_method(D_METHOD("get_attribute_value_by_tag", "tag"), &AbilitySystemComponent::get_attribute_value_by_tag);
	ClassDB::bind_method(D_METHOD("get_attribute_base_value_by_tag", "tag"), &AbilitySystemComponent::get_attribute_base_value_by_tag);
	ClassDB::bind_method(D_METHOD("set_attribute_base_value_by_tag", "tag", "value"), &AbilitySystemComponent::set_attribute_base_value_by_tag);
	ClassDB::bind_method(D_METHOD("has_attribute_by_tag", "tag"), &AbilitySystemComponent::has_attribute_by_tag);

	// --- Attribute API (By Resource) ---
	ClassDB::bind_method(D_METHOD("get_attribute_value_by_resource", "attribute"), &AbilitySystemComponent::get_attribute_value_by_resource);
	ClassDB::bind_method(D_METHOD("get_attribute_base_value_by_resource", "attribute"), &AbilitySystemComponent::get_attribute_base_value_by_resource);
	ClassDB::bind_method(D_METHOD("set_attribute_base_value_by_resource", "attribute", "value"), &AbilitySystemComponent::set_attribute_base_value_by_resource);
	ClassDB::bind_method(D_METHOD("has_attribute_by_resource", "attribute"), &AbilitySystemComponent::has_attribute_by_resource);

	// --- Cooldown API ---
	ClassDB::bind_method(D_METHOD("start_cooldown", "ability_tag", "duration", "tags"), &AbilitySystemComponent::start_cooldown);
	ClassDB::bind_method(D_METHOD("is_on_cooldown", "ability_tag"), &AbilitySystemComponent::is_on_cooldown);
	ClassDB::bind_method(D_METHOD("get_cooldown_remaining", "ability_tag"), &AbilitySystemComponent::get_cooldown_remaining);
	ClassDB::bind_method(D_METHOD("tick", "delta"), &AbilitySystemComponent::tick);

	// --- Tag Management ---
	ClassDB::bind_method(D_METHOD("add_tag", "tag"), &AbilitySystemComponent::add_tag);
	ClassDB::bind_method(D_METHOD("remove_tag", "tag"), &AbilitySystemComponent::remove_tag);
	ClassDB::bind_method(D_METHOD("has_tag", "tag"), &AbilitySystemComponent::has_tag);
	ClassDB::bind_method(D_METHOD("get_owned_tags"), &AbilitySystemComponent::get_owned_tags);

	// --- Cue Activation API (By Tag) ---
	ClassDB::bind_method(D_METHOD("can_activate_cue_by_tag", "tag"), &AbilitySystemComponent::can_activate_cue_by_tag);
	ClassDB::bind_method(D_METHOD("try_activate_cue_by_tag", "tag", "data", "target_node"), &AbilitySystemComponent::try_activate_cue_by_tag, DEFVAL(Dictionary()), DEFVAL(Variant()));
	ClassDB::bind_method(D_METHOD("cancel_cue_by_tag", "tag"), &AbilitySystemComponent::cancel_cue_by_tag);

	// --- Cue Activation API (By Resource) ---
	ClassDB::bind_method(D_METHOD("can_activate_cue_by_resource", "cue"), &AbilitySystemComponent::can_activate_cue_by_resource);
	ClassDB::bind_method(D_METHOD("try_activate_cue_by_resource", "cue", "data", "target_node"), &AbilitySystemComponent::try_activate_cue_by_resource, DEFVAL(Dictionary()), DEFVAL(Variant()));
	ClassDB::bind_method(D_METHOD("cancel_cue_by_resource", "cue"), &AbilitySystemComponent::cancel_cue_by_resource);

	ClassDB::bind_method(D_METHOD("register_cue_resource", "cue"), &AbilitySystemComponent::register_cue_resource);
	ClassDB::bind_method(D_METHOD("unregister_cue_resource", "tag"), &AbilitySystemComponent::unregister_cue_resource);
	ClassDB::bind_method(D_METHOD("get_cue_resource", "tag"), &AbilitySystemComponent::get_cue_resource);

	// --- Task & Montage API ---
	ClassDB::bind_method(D_METHOD("register_task", "task"), &AbilitySystemComponent::register_task);
	ClassDB::bind_method(D_METHOD("play_montage", "name", "target"), &AbilitySystemComponent::play_montage, DEFVAL(Variant()));
	ClassDB::bind_method(D_METHOD("is_montage_playing", "name", "target"), &AbilitySystemComponent::is_montage_playing, DEFVAL(Variant()));
	ClassDB::bind_method(D_METHOD("play_sound", "sound", "target"), &AbilitySystemComponent::play_sound, DEFVAL(Variant()));

	// --- Node Registration ---
	ClassDB::bind_method(D_METHOD("register_node", "name", "node"), &AbilitySystemComponent::register_node);
	ClassDB::bind_method(D_METHOD("unregister_node", "name"), &AbilitySystemComponent::unregister_node);
	ClassDB::bind_method(D_METHOD("get_node_ptr", "name"), &AbilitySystemComponent::get_node_ptr);

	ClassDB::bind_method(D_METHOD("set_animation_player", "node"), &AbilitySystemComponent::set_animation_player);
	ClassDB::bind_method(D_METHOD("get_animation_player"), &AbilitySystemComponent::get_animation_player);
	ClassDB::bind_method(D_METHOD("set_audio_player", "node"), &AbilitySystemComponent::set_audio_player);
	ClassDB::bind_method(D_METHOD("get_audio_player"), &AbilitySystemComponent::get_audio_player);

	// --- Calculation Engine ---
	ClassDB::bind_method(D_METHOD("calculate_modifier_magnitude", "spec", "modifier_index"), &AbilitySystemComponent::calculate_modifier_magnitude);

	// --- Debug Helpers ---
	ClassDB::bind_method(D_METHOD("get_active_effects_debug"), &AbilitySystemComponent::get_active_effects_debug);
	ClassDB::bind_method(D_METHOD("get_granted_abilities_debug"), &AbilitySystemComponent::get_granted_abilities_debug);

	GDVIRTUAL_BIND(_on_calculate_custom_magnitude, "effect_spec", "modifier_index");

	// --- Signals ---
	ADD_SIGNAL(MethodInfo("attribute_changed", PropertyInfo(Variant::STRING_NAME, "attribute_name"), PropertyInfo(Variant::FLOAT, "old_value"), PropertyInfo(Variant::FLOAT, "new_value")));
	ADD_SIGNAL(MethodInfo("tag_changed", PropertyInfo(Variant::STRING_NAME, "tag_name"), PropertyInfo(Variant::BOOL, "is_present")));
	ADD_SIGNAL(MethodInfo("ability_activated", PropertyInfo(Variant::OBJECT, "ability_spec", PROPERTY_HINT_NONE, "AbilitySystemAbilitySpec")));
	ADD_SIGNAL(MethodInfo("ability_ended", PropertyInfo(Variant::OBJECT, "ability_spec", PROPERTY_HINT_NONE, "AbilitySystemAbilitySpec"), PropertyInfo(Variant::BOOL, "was_cancelled")));
	ADD_SIGNAL(MethodInfo("ability_failed", PropertyInfo(Variant::STRING_NAME, "ability_name"), PropertyInfo(Variant::STRING, "reason")));

	ADD_SIGNAL(MethodInfo("effect_failed", PropertyInfo(Variant::STRING_NAME, "effect_name"), PropertyInfo(Variant::STRING, "reason")));
	ADD_SIGNAL(MethodInfo("cue_failed", PropertyInfo(Variant::STRING_NAME, "cue_name"), PropertyInfo(Variant::STRING, "reason")));

	ADD_SIGNAL(MethodInfo("effect_applied", PropertyInfo(Variant::OBJECT, "effect_spec", PROPERTY_HINT_NONE, "AbilitySystemEffectSpec")));
	ADD_SIGNAL(MethodInfo("effect_removed", PropertyInfo(Variant::OBJECT, "effect_spec", PROPERTY_HINT_NONE, "AbilitySystemEffectSpec")));
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
			_process_cooldowns(delta);
		} break;
	}
}

void AbilitySystemComponent::_process_cooldowns(float p_delta) {
	Vector<StringName> to_remove;
	for (KeyValue<StringName, CooldownData> &E : active_cooldowns) {
		E.value.remaining -= p_delta;
		if (E.value.remaining <= 0) {
			// Cleanup tags
			for (int i = 0; i < E.value.tags.size(); i++) {
				remove_tag(E.value.tags[i]);
			}
			to_remove.push_back(E.key);
		}
	}

	for (int i = 0; i < to_remove.size(); i++) {
		active_cooldowns.erase(to_remove[i]);
		emit_signal("cooldown_ended", to_remove[i]);
	}
}

void AbilitySystemComponent::start_cooldown(const StringName &p_ability_tag, float p_duration, const TypedArray<StringName> &p_cooldown_tags) {
	if (p_duration <= 0) {
		return;
	}

	CooldownData cd;
	cd.remaining = p_duration;
	cd.tags = p_cooldown_tags;
	active_cooldowns[p_ability_tag] = cd;

	for (int i = 0; i < p_cooldown_tags.size(); i++) {
		add_tag(p_cooldown_tags[i]);
	}

	emit_signal("cooldown_started", p_ability_tag, p_duration);
}

bool AbilitySystemComponent::is_on_cooldown(const StringName &p_ability_tag) const {
	return active_cooldowns.has(p_ability_tag);
}

float AbilitySystemComponent::get_cooldown_remaining(const StringName &p_ability_tag) const {
	if (active_cooldowns.has(p_ability_tag)) {
		return active_cooldowns[p_ability_tag].remaining;
	}
	return 0.0f;
}

void AbilitySystemComponent::tick(float p_delta) {
	_process_effects(p_delta);
	_process_tasks(p_delta);
	_process_cooldowns(p_delta);
}

void AbilitySystemComponent::_process_effects(float p_delta) {
	bool removed_any = false;
	bool recalculate_needed = false;

	for (int i = active_effects.size() - 1; i >= 0; i--) {
		Ref<AbilitySystemEffectSpec> spec = active_effects[i];
		Ref<AbilitySystemEffect> effect = spec->get_effect();

		// --- Duration countdown ---
		if (effect->get_duration_policy() == AbilitySystemEffect::POLICY_DURATION) {
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
					float mag = calculate_modifier_magnitude(spec, j);
					float current = get_attribute_value_by_tag(attr);
					float next = current;
					switch (op) {
						case AbilitySystemEffect::OP_ADD:
							next += mag;
							break;
						case AbilitySystemEffect::OP_MULTIPLY:
							next *= mag;
							break;
						case AbilitySystemEffect::OP_DIVIDE:
							if (mag != 0) {
								next /= mag;
							}
							break;
						case AbilitySystemEffect::OP_OVERRIDE:
							next = mag;
							break;
					}
					set_attribute_base_value_by_tag(attr, next);
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
	if (effect->get_duration_policy() != AbilitySystemEffect::POLICY_INSTANT) {
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

void AbilitySystemComponent::remove_effect_by_tag(const StringName &p_tag) {
	Vector<Ref<AbilitySystemEffectSpec>> to_remove;
	for (int i = 0; i < active_effects.size(); i++) {
		Ref<AbilitySystemEffect> effect = active_effects[i]->get_effect();
		if (effect.is_valid() && effect->get_effect_tag() == p_tag) {
			to_remove.push_back(active_effects[i]);
		}
	}

	for (int i = 0; i < to_remove.size(); i++) {
		remove_active_effect(to_remove[i]);
	}
}

void AbilitySystemComponent::remove_effect_by_resource(const Ref<AbilitySystemEffect> &p_effect) {
	ERR_FAIL_COND(p_effect.is_null());
	Vector<Ref<AbilitySystemEffectSpec>> to_remove;
	for (int i = 0; i < active_effects.size(); i++) {
		if (active_effects[i]->get_effect() == p_effect) {
			to_remove.push_back(active_effects[i]);
		}
	}

	for (int i = 0; i < to_remove.size(); i++) {
		remove_active_effect(to_remove[i]);
	}
}

bool AbilitySystemComponent::has_active_effect_by_tag(const StringName &p_tag) const {
	for (int i = 0; i < active_effects.size(); i++) {
		Ref<AbilitySystemEffect> effect = active_effects[i]->get_effect();
		if (effect.is_valid() && effect->get_effect_tag() == p_tag) {
			return true;
		}
	}
	return false;
}

bool AbilitySystemComponent::has_active_effect_by_resource(const Ref<AbilitySystemEffect> &p_effect) const {
	for (int i = 0; i < active_effects.size(); i++) {
		if (active_effects[i]->get_effect() == p_effect) {
			return true;
		}
	}
	return false;
}

void AbilitySystemComponent::cancel_all_abilities() {
	for (int i = 0; i < granted_abilities.size(); i++) {
		Ref<AbilitySystemAbilitySpec> spec = granted_abilities[i];
		if (spec.is_valid() && spec->get_is_active()) {
			Ref<AbilitySystemAbility> ability = spec->get_ability();
			if (ability.is_valid()) {
				ability->end_ability(this, spec);
				spec->set_is_active(false);
				emit_signal("ability_ended", spec, true);
			}
		}
	}
}

void AbilitySystemComponent::remove_granted_ability_spec(Ref<AbilitySystemAbilitySpec> p_spec) {
	ERR_FAIL_COND(p_spec.is_null());
	int idx = granted_abilities.find(p_spec);
	if (idx != -1) {
		granted_abilities.remove_at(idx);
	}
}

void AbilitySystemComponent::give_ability_by_resource(const Ref<AbilitySystemAbility> &p_ability, int p_level) {
	ERR_FAIL_COND(p_ability.is_null());
	Ref<AbilitySystemAbilitySpec> spec;
	spec.instantiate();
	spec->init(p_ability, p_level);
	spec->set_owner(this);
	granted_abilities.push_back(spec);
}

void AbilitySystemComponent::give_ability_by_tag(const StringName &p_tag, int p_level) {
	if (ability_container.is_valid()) {
		TypedArray<AbilitySystemAbility> abilities = ability_container->get_abilities();
		for (int i = 0; i < abilities.size(); i++) {
			Ref<AbilitySystemAbility> ability = abilities[i];
			if (ability.is_valid() && ability->get_ability_tag() == p_tag) {
				give_ability_by_resource(ability, p_level);
				return;
			}
		}
	}
	ERR_PRINT(vformat("AbilitySystemComponent: give_ability_by_tag failed - Ability tag '%s' not found in container catalog.", p_tag));
}

void AbilitySystemComponent::remove_ability_by_tag(const StringName &p_tag) {
	for (int i = granted_abilities.size() - 1; i >= 0; i--) {
		if (granted_abilities[i]->get_ability()->get_ability_tag() == p_tag) {
			remove_granted_ability_spec(granted_abilities[i]);
		}
	}
}

void AbilitySystemComponent::remove_ability_by_resource(const Ref<AbilitySystemAbility> &p_ability) {
	for (int i = granted_abilities.size() - 1; i >= 0; i--) {
		if (granted_abilities[i]->get_ability() == p_ability) {
			remove_granted_ability_spec(granted_abilities[i]);
		}
	}
}

void AbilitySystemComponent::apply_ability_container(Ref<AbilitySystemAbilityContainer> p_container, int p_level) {
	ERR_FAIL_COND(p_container.is_null());

	// 1. Duplicate container and initialize from AttributeSet (ensures uniqueness per actor instance)
	Ref<AbilitySystemAbilityContainer> local_container = p_container->duplicate(true);
	Ref<AbilitySystemAttributeSet> attr_set = local_container->get_attribute_set();

	if (attr_set.is_valid()) {
		// Register the AttributeSet
		add_attribute_set(attr_set);

		// Grant abilities defined by the AttributeSet (already unlocked)
		TypedArray<AbilitySystemAbility> granted = attr_set->get_granted_abilities();
		for (int i = 0; i < granted.size(); i++) {
			Ref<AbilitySystemAbility> ability = granted[i];
			if (ability.is_valid()) {
				give_ability_by_resource(ability, p_level);
			}
		}
	}

	// 2. Apply Effects
	TypedArray<AbilitySystemEffect> effects = local_container->get_effects();
	for (int i = 0; i < effects.size(); i++) {
		Ref<AbilitySystemEffect> effect = effects[i];
		if (effect.is_valid()) {
			Ref<AbilitySystemEffectSpec> spec = make_outgoing_spec(effect, (float)p_level);
			apply_effect_spec_to_self(spec);
		}
	}

	// 3. Register Cues
	TypedArray<AbilitySystemCue> cues = local_container->get_cues();
	for (int i = 0; i < cues.size(); i++) {
		Ref<AbilitySystemCue> cue = cues[i];
		if (cue.is_valid()) {
			register_cue_resource(cue);
		}
	}

	// Force update of current values after setting base values
	_update_attribute_current_values();
}

// --- Ability API (By Tag) ---

bool AbilitySystemComponent::can_activate_ability_by_tag(const StringName &p_tag) {
	for (int i = 0; i < granted_abilities.size(); i++) {
		Ref<AbilitySystemAbilitySpec> spec = granted_abilities[i];
		if (spec->get_ability()->get_ability_tag() == p_tag) {
			if (spec->get_ability()->can_activate_ability(this, spec)) {
				return true;
			} else {
				emit_signal("ability_failed", p_tag, "Tag/Cost/Cooldown requirements not met");
				return false;
			}
		}
	}
	emit_signal("ability_failed", p_tag, "Ability not granted to actor");
	return false;
}

bool AbilitySystemComponent::try_activate_ability_by_tag(const StringName &p_tag) {
	for (int i = 0; i < granted_abilities.size(); i++) {
		Ref<AbilitySystemAbilitySpec> spec = granted_abilities[i];
		if (spec->get_ability()->get_ability_tag() == p_tag) {
			if (spec->get_ability()->can_activate_ability(this, spec)) {
				spec->set_is_active(true);
				spec->get_ability()->activate_ability(this, spec);
				emit_signal("ability_activated", spec);
				return true;
			} else {
				emit_signal("ability_failed", p_tag, "Tag/Cost/Cooldown requirements not met");
				return false;
			}
		}
	}
	emit_signal("ability_failed", p_tag, "Ability not granted to actor");
	return false;
}

void AbilitySystemComponent::cancel_ability_by_tag(const StringName &p_tag) {
	for (int i = 0; i < granted_abilities.size(); i++) {
		Ref<AbilitySystemAbilitySpec> spec = granted_abilities[i];
		if (spec->get_ability()->get_ability_tag() == p_tag && spec->get_is_active()) {
			spec->get_ability()->end_ability(this, spec);
			spec->set_is_active(false);
			emit_signal("ability_ended", spec, true);
		}
	}
}

// --- Ability API (By Resource) ---

bool AbilitySystemComponent::can_activate_ability_by_resource(const Ref<AbilitySystemAbility> &p_ability) {
	ERR_FAIL_COND_V(p_ability.is_null(), false);
	for (int i = 0; i < granted_abilities.size(); i++) {
		Ref<AbilitySystemAbilitySpec> spec = granted_abilities[i];
		if (spec->get_ability() == p_ability) {
			if (p_ability->can_activate_ability(this, spec)) {
				return true;
			} else {
				emit_signal("ability_failed", p_ability->get_ability_tag(), "Tag/Cost/Cooldown requirements not met");
				return false;
			}
		}
	}
	emit_signal("ability_failed", p_ability->get_ability_tag(), "Ability not granted to actor");
	return false;
}

bool AbilitySystemComponent::try_activate_ability_by_resource(const Ref<AbilitySystemAbility> &p_ability) {
	ERR_FAIL_COND_V(p_ability.is_null(), false);
	for (int i = 0; i < granted_abilities.size(); i++) {
		Ref<AbilitySystemAbilitySpec> spec = granted_abilities[i];
		if (spec->get_ability() == p_ability) {
			if (p_ability->can_activate_ability(this, spec)) {
				spec->set_is_active(true);
				p_ability->activate_ability(this, spec);
				emit_signal("ability_activated", spec);
				return true;
			} else {
				emit_signal("ability_failed", p_ability->get_ability_tag(), "Tag/Cost/Cooldown requirements not met");
				return false;
			}
		}
	}
	emit_signal("ability_failed", p_ability->get_ability_tag(), "Ability not granted to actor");
	return false;
}

void AbilitySystemComponent::cancel_ability_by_resource(const Ref<AbilitySystemAbility> &p_ability) {
	ERR_FAIL_COND(p_ability.is_null());
	for (int i = 0; i < granted_abilities.size(); i++) {
		Ref<AbilitySystemAbilitySpec> spec = granted_abilities[i];
		if (spec->get_ability() == p_ability && spec->get_is_active()) {
			p_ability->end_ability(this, spec);
			spec->set_is_active(false);
			emit_signal("ability_ended", spec, true);
		}
	}
}

// --- Effect Activation API ---

bool AbilitySystemComponent::can_activate_effect_by_resource(const Ref<AbilitySystemEffect> &p_effect) {
	ERR_FAIL_COND_V(p_effect.is_null(), false);

	if (!owned_tags->has_all_tags(p_effect->get_activation_required_tags())) {
		emit_signal("effect_failed", p_effect->get_effect_tag(), "Required tags missing");
		return false;
	}

	if (owned_tags->has_any_tags(p_effect->get_activation_blocked_tags())) {
		emit_signal("effect_failed", p_effect->get_effect_tag(), "Blocked by tags");
		return false;
	}

	return true;
}

bool AbilitySystemComponent::can_activate_effect_by_tag(const StringName &p_tag) {
	if (ability_container.is_valid()) {
		TypedArray<AbilitySystemEffect> effects = ability_container->get_effects();
		for (int i = 0; i < effects.size(); i++) {
			Ref<AbilitySystemEffect> effect = effects[i];
			if (effect.is_valid() && effect->get_effect_tag() == p_tag) {
				return can_activate_effect_by_resource(effect);
			}
		}
	}
	emit_signal("effect_failed", p_tag, "Effect tag not found in container catalog");
	return false;
}

bool AbilitySystemComponent::try_activate_effect_by_resource(const Ref<AbilitySystemEffect> &p_effect, float p_level, Object *p_target_node) {
	if (can_activate_effect_by_resource(p_effect)) {
		apply_effect_by_resource(p_effect, p_level, p_target_node);
		return true;
	}
	return false;
}

bool AbilitySystemComponent::try_activate_effect_by_tag(const StringName &p_tag, float p_level, Object *p_target_node) {
	if (can_activate_effect_by_tag(p_tag)) {
		apply_effect_by_tag(p_tag, p_level, p_target_node);
		return true;
	}
	return false;
}

void AbilitySystemComponent::cancel_effect_by_tag(const StringName &p_tag) {
	remove_effect_by_tag(p_tag);
}

void AbilitySystemComponent::cancel_effect_by_resource(const Ref<AbilitySystemEffect> &p_effect) {
	remove_effect_by_resource(p_effect);
}

// --- Effect Execution API (Low level) ---

void AbilitySystemComponent::apply_effect_by_tag(const StringName &p_tag, float p_level, Object *p_target_node) {
	if (ability_container.is_valid()) {
		TypedArray<AbilitySystemEffect> effects = ability_container->get_effects();
		for (int i = 0; i < effects.size(); i++) {
			Ref<AbilitySystemEffect> effect = effects[i];
			if (effect.is_valid() && effect->get_effect_tag() == p_tag) {
				apply_effect_by_resource(effect, p_level, p_target_node);
				return;
			}
		}
	}
	ERR_PRINT(vformat("AbilitySystemComponent: apply_effect_by_tag failed - Effect tag '%s' not found in container catalog.", p_tag));
}

void AbilitySystemComponent::apply_effect_by_resource(const Ref<AbilitySystemEffect> &p_effect, float p_level, Object *p_target_node) {
	Ref<AbilitySystemEffectSpec> spec = make_outgoing_spec(p_effect, p_level, p_target_node);
	apply_effect_spec_to_self(spec);
}

Ref<AbilitySystemEffectSpec> AbilitySystemComponent::make_outgoing_spec(Ref<AbilitySystemEffect> p_effect, float p_level, Object *p_target_node) {
	ERR_FAIL_COND_V(p_effect.is_null(), Ref<AbilitySystemEffectSpec>());

	// Validation: Outgoing effects must be part of the archetype contract.
	if (ability_container.is_valid()) {
		if (!ability_container->has_effect(p_effect)) {
			bool found = false;
			for (int i = 0; i < granted_abilities.size(); i++) {
				Ref<AbilitySystemAbility> ability = granted_abilities[i]->get_ability();
				if (ability.is_valid() && (ability->get_costs().has(p_effect) || ability->get_cooldown_tags().has(p_effect))) {
					found = true;
					break;
				}
			}

			if (!found) {
				ERR_PRINT(vformat("AbilitySystemComponent Error: Effect '%s' is NOT registered in the source's AbilityContainer.", p_effect->get_path()));
			}
		}
	}

	Ref<AbilitySystemEffectSpec> spec;
	spec.instantiate();
	spec->init(p_effect, p_level);
	spec->set_source_component(this);
	spec->set_target_node(p_target_node);
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

	if (effect->get_duration_policy() == AbilitySystemEffect::POLICY_DURATION) {
		float duration = calculate_effect_duration(p_spec);
		p_spec->set_duration_remaining(duration);
		p_spec->set_total_duration(duration);
	}

	if (effect->get_duration_policy() == AbilitySystemEffect::POLICY_INSTANT) {
		for (int i = 0; i < effect->get_modifier_count(); i++) {
			StringName attr = effect->get_modifier_attribute(i);
			AbilitySystemEffect::ModifierOp op = effect->get_modifier_operation(i);
			float mag = calculate_modifier_magnitude(p_spec, i);
			float current = get_attribute_value_by_tag(attr);
			float next = current;
			switch (op) {
				case AbilitySystemEffect::OP_ADD:
					next += mag;
					break;
				case AbilitySystemEffect::OP_MULTIPLY:
					next *= mag;
					break;
				case AbilitySystemEffect::OP_DIVIDE:
					if (mag != 0) {
						next /= mag;
					}
					break;
				case AbilitySystemEffect::OP_OVERRIDE:
					next = mag;
					break;
			}
			set_attribute_base_value_by_tag(attr, next);
		}
		_update_attribute_current_values();
	} else {
		AbilitySystemEffect::StackingPolicy stacking = effect->get_stacking_policy();
		if (stacking != AbilitySystemEffect::STACK_NEW_INSTANCE) {
			for (int i = 0; i < active_effects.size(); i++) {
				if (active_effects[i]->get_effect() == effect) {
					Ref<AbilitySystemEffectSpec> existing = active_effects[i];
					switch (stacking) {
						case AbilitySystemEffect::STACK_OVERRIDE:
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
					_update_attribute_current_values();
					goto finish_cues;
				}
			}
		}

		active_effects.push_back(p_spec);
		if (effect->get_period() > 0.0f) {
			p_spec->set_period_timer(effect->get_period());
		}
		TypedArray<StringName> granted = effect->get_granted_tags();
		for (int i = 0; i < granted.size(); i++) {
			add_tag(granted[i]);
		}
		_update_attribute_current_values();
	}

finish_cues: {
	float rep_mag = 0.0f;
	for (int j = 0; j < effect->get_modifier_count(); j++) {
		rep_mag += calculate_modifier_magnitude(p_spec, j);
	}

	StringName effect_tag = effect->get_effect_tag();
	if (effect_tag != StringName()) {
		Ref<AbilitySystemCue> cue = get_cue_resource(effect_tag);
		if (cue.is_null() && p_spec->get_source_component()) {
			cue = p_spec->get_source_component()->get_cue_resource(effect_tag);
		}
		if (cue.is_valid() && cue->get_event_type() != AbilitySystemCue::ON_REMOVE) {
			Ref<AbilitySystemCueSpec> cue_spec;
			cue_spec.instantiate();
			cue_spec->init_from_effect(cue, p_spec->get_source_component(), this, p_spec, rep_mag);
			_execute_cue_with_spec(effect_tag, cue_spec);
		} else if (cue.is_null()) {
			emit_signal("tag_event_received", effect_tag, Dictionary());
		}
	}

	TypedArray<AbilitySystemCue> cues = effect->get_cues();
	for (int i = 0; i < cues.size(); i++) {
		Ref<AbilitySystemCue> cue = cues[i];
		if (cue.is_valid() && cue->get_event_type() != AbilitySystemCue::ON_REMOVE) {
			Ref<AbilitySystemCueSpec> cue_spec;
			cue_spec.instantiate();
			cue_spec->init_from_effect(cue, p_spec->get_source_component(), this, p_spec, rep_mag);
			cue->execute(cue_spec);
		}
	}
}
	emit_signal("effect_applied", p_spec);
}

// --- Cue Activation API ---

bool AbilitySystemComponent::can_activate_cue_by_resource(const Ref<AbilitySystemCue> &p_cue) {
	ERR_FAIL_COND_V(p_cue.is_null(), false);
	if (!owned_tags->has_all_tags(p_cue->get_activation_required_tags())) {
		emit_signal("cue_failed", p_cue->get_cue_tag(), "Required tags missing");
		return false;
	}
	if (owned_tags->has_any_tags(p_cue->get_activation_blocked_tags())) {
		emit_signal("cue_failed", p_cue->get_cue_tag(), "Blocked by tags");
		return false;
	}
	return true;
}

bool AbilitySystemComponent::can_activate_cue_by_tag(const StringName &p_tag) {
	Ref<AbilitySystemCue> cue = get_cue_resource(p_tag);
	if (cue.is_valid()) {
		return can_activate_cue_by_resource(cue);
	}
	emit_signal("cue_failed", p_tag, "Cue tag not found in registered cues");
	return false;
}

bool AbilitySystemComponent::try_activate_cue_by_resource(const Ref<AbilitySystemCue> &p_cue, const Dictionary &p_data, Object *p_target_node) {
	if (can_activate_cue_by_resource(p_cue)) {
		Ref<AbilitySystemCueSpec> spec;
		spec.instantiate();
		spec->init_manual(p_cue, this, p_data);
		spec->set_target_node(p_target_node);
		_execute_cue_with_spec(p_cue->get_cue_tag(), spec);
		return true;
	}
	return false;
}

bool AbilitySystemComponent::try_activate_cue_by_tag(const StringName &p_tag, const Dictionary &p_data, Object *p_target_node) {
	if (can_activate_cue_by_tag(p_tag)) {
		Ref<AbilitySystemCue> cue = get_cue_resource(p_tag);
		return try_activate_cue_by_resource(cue, p_data, p_target_node);
	}
	return false;
}

void AbilitySystemComponent::cancel_cue_by_tag(const StringName &p_tag) {
	Ref<AbilitySystemCue> cue = get_cue_resource(p_tag);
	if (cue.is_valid()) {
		cancel_cue_by_resource(cue);
	}
}

void AbilitySystemComponent::cancel_cue_by_resource(const Ref<AbilitySystemCue> &p_cue) {
	ERR_FAIL_COND(p_cue.is_null());
	Ref<AbilitySystemCueSpec> spec;
	spec.instantiate();
	spec->init_manual(p_cue, this, Dictionary());
	// Manually trigger REMOVE event
	if (p_cue->get_event_type() == AbilitySystemCue::ON_ACTIVE || p_cue->get_event_type() == AbilitySystemCue::ON_REMOVE) {
		p_cue->set_event_type(AbilitySystemCue::ON_REMOVE);
		p_cue->execute(spec);
	}
}

void AbilitySystemComponent::_execute_cue_with_spec(const StringName &p_tag, Ref<AbilitySystemCueSpec> p_spec) {
	emit_signal("tag_event_received", p_tag, p_spec->get_extra_data());
	Ref<AbilitySystemCue> cue = p_spec->get_cue();
	if (cue.is_valid()) {
		cue->execute(p_spec);
	}
}

void AbilitySystemComponent::register_cue_resource(Ref<AbilitySystemCue> p_cue) {
	ERR_FAIL_COND(p_cue.is_null());
	StringName tag = p_cue->get_cue_tag();
	if (tag != StringName()) {
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

// --- Tag Management ---

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

Ref<AbilitySystemTagSpec> AbilitySystemComponent::get_owned_tags() const {
	return owned_tags;
}

// --- Attribute API ---

float AbilitySystemComponent::get_attribute_value_by_tag(const StringName &p_tag) const {
	for (int i = 0; i < attribute_sets.size(); i++) {
		if (attribute_sets[i]->has_attribute(p_tag)) {
			return attribute_sets[i]->get_attribute_current_value(p_tag);
		}
	}
	return 0.0f;
}

float AbilitySystemComponent::get_attribute_base_value_by_tag(const StringName &p_tag) const {
	for (int i = 0; i < attribute_sets.size(); i++) {
		if (attribute_sets[i]->has_attribute(p_tag)) {
			return attribute_sets[i]->get_attribute_base_value(p_tag);
		}
	}
	return 0.0f;
}

float AbilitySystemComponent::get_attribute_value_by_resource(const Ref<AbilitySystemAttribute> &p_attribute) const {
	ERR_FAIL_COND_V(p_attribute.is_null(), 0.0f);
	return get_attribute_value_by_tag(p_attribute->get_attribute_name());
}

float AbilitySystemComponent::get_attribute_base_value_by_resource(const Ref<AbilitySystemAttribute> &p_attribute) const {
	ERR_FAIL_COND_V(p_attribute.is_null(), 0.0f);
	return get_attribute_base_value_by_tag(p_attribute->get_attribute_name());
}

void AbilitySystemComponent::set_attribute_base_value_by_tag(const StringName &p_tag, float p_value) {
	for (int i = 0; i < attribute_sets.size(); i++) {
		if (attribute_sets[i]->has_attribute(p_tag)) {
			attribute_sets[i]->set_attribute_base_value(p_tag, p_value);
			_update_attribute_current_values();
			return;
		}
	}
}

void AbilitySystemComponent::set_attribute_base_value_by_resource(const Ref<AbilitySystemAttribute> &p_attribute, float p_value) {
	ERR_FAIL_COND(p_attribute.is_null());
	set_attribute_base_value_by_tag(p_attribute->get_attribute_name(), p_value);
}

bool AbilitySystemComponent::has_attribute_by_tag(const StringName &p_tag) const {
	for (int i = 0; i < attribute_sets.size(); i++) {
		if (attribute_sets[i]->has_attribute(p_tag)) {
			return true;
		}
	}
	return false;
}

bool AbilitySystemComponent::has_attribute_by_resource(const Ref<AbilitySystemAttribute> &p_attribute) const {
	ERR_FAIL_COND_V(p_attribute.is_null(), false);
	return has_attribute_by_tag(p_attribute->get_attribute_name());
}

// --- Logic & Calculations ---

float AbilitySystemComponent::calculate_modifier_magnitude(const Ref<AbilitySystemEffectSpec> &p_spec, int p_modifier_idx) const {
	ERR_FAIL_COND_V(p_spec.is_null(), 0.0f);
	Ref<AbilitySystemEffect> effect = p_spec->get_effect();
	ERR_FAIL_COND_V(effect.is_null(), 0.0f);
	if (effect->is_modifier_custom(p_modifier_idx)) {
		float result = 0.0f;
		if (const_cast<AbilitySystemComponent *>(this)->GDVIRTUAL_CALL(_on_calculate_custom_magnitude, p_spec, p_modifier_idx, result)) {
			return result;
		}
	}
	return effect->get_modifier_magnitude(p_modifier_idx);
}

float AbilitySystemComponent::calculate_effect_duration(const Ref<AbilitySystemEffectSpec> &p_spec) const {
	ERR_FAIL_COND_V(p_spec.is_null(), 0.0f);
	Ref<AbilitySystemEffect> effect = p_spec->get_effect();
	ERR_FAIL_COND_V(effect.is_null(), 0.0f);
	if (effect->get_use_custom_duration()) {
		float result = 0.0f;
		if (const_cast<AbilitySystemComponent *>(this)->GDVIRTUAL_CALL(_on_calculate_custom_magnitude, p_spec, -1, result)) {
			return result;
		}
	}
	return effect->get_duration_magnitude();
}

void AbilitySystemComponent::add_attribute_set(Ref<AbilitySystemAttributeSet> p_set) {
	ERR_FAIL_COND(p_set.is_null());
	Ref<AbilitySystemAttributeSet> local_set = p_set->duplicate(true);
	local_set->set_attribute_definitions(local_set->get_attribute_definitions());
	attribute_sets.push_back(local_set);
	if (!local_set->is_connected("attribute_changed", callable_mp(this, &AbilitySystemComponent::_on_attribute_set_attribute_changed))) {
		local_set->connect("attribute_changed", callable_mp(this, &AbilitySystemComponent::_on_attribute_set_attribute_changed));
	}
}

void AbilitySystemComponent::_on_attribute_set_attribute_changed(const StringName &p_name, float p_old_val, float p_new_val) {
	emit_signal("attribute_changed", p_name, p_old_val, p_new_val);
}

void AbilitySystemComponent::_update_attribute_current_values() {
	for (int k = 0; k < attribute_sets.size(); k++) {
		attribute_sets[k]->reset_current_values();
	}
	const AbilitySystemEffect::ModifierOp OP_ORDER[] = { AbilitySystemEffect::OP_ADD, AbilitySystemEffect::OP_MULTIPLY, AbilitySystemEffect::OP_DIVIDE, AbilitySystemEffect::OP_OVERRIDE };
	for (int pass = 0; pass < 4; pass++) {
		AbilitySystemEffect::ModifierOp current_op = OP_ORDER[pass];
		for (int i = 0; i < active_effects.size(); i++) {
			Ref<AbilitySystemEffectSpec> spec = active_effects[i];
			Ref<AbilitySystemEffect> effect = spec->get_effect();
			const float stack_multiplier = (effect->get_stacking_policy() == AbilitySystemEffect::STACK_INTENSITY) ? (float)spec->get_stack_count() : 1.0f;
			for (int j = 0; j < effect->get_modifier_count(); j++) {
				if (effect->get_modifier_operation(j) != current_op || effect->get_period() > 0.0f) {
					continue;
				}
				StringName attr_name = effect->get_modifier_attribute(j);
				for (int k = 0; k < attribute_sets.size(); k++) {
					if (!attribute_sets[k]->has_attribute(attr_name)) {
						continue;
					}
					float current = attribute_sets[k]->get_attribute_current_value(attr_name);
					float mag = calculate_modifier_magnitude(spec, j) * stack_multiplier;
					float next = current;
					switch (current_op) {
						case AbilitySystemEffect::OP_ADD:
							next += mag;
							break;
						case AbilitySystemEffect::OP_MULTIPLY:
							next *= mag;
							break;
						case AbilitySystemEffect::OP_DIVIDE:
							if (mag != 0) {
								next /= mag;
							}
							break;
						case AbilitySystemEffect::OP_OVERRIDE:
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
	if (p_sound.is_null()) {
		return;
	}
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
	return registered_nodes.has(p_name) ? registered_nodes[p_name] : nullptr;
}

void AbilitySystemComponent::set_ability_container(Ref<AbilitySystemAbilityContainer> p_container) {
	ability_container = p_container;
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

AbilitySystemComponent::AbilitySystemComponent() {
	owned_tags.instantiate();
}
AbilitySystemComponent::~AbilitySystemComponent() {}

} // namespace godot

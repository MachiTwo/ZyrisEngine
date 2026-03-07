/**************************************************************************/
/*  ability_system_task.cpp                                               */
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
#include "modules/ability_system/core/ability_system_task.h"
#include "modules/ability_system/scene/ability_system_component.h"
#elif defined(ABILITY_SYSTEM_GDEXTENSION)
#include "src/core/ability_system_task.h"
#include "src/scene/ability_system_component.h"
#endif

namespace godot {

void AbilitySystemTask::_bind_methods() {
	ADD_SIGNAL(MethodInfo("completed"));

	ClassDB::bind_method(D_METHOD("activate"), &AbilitySystemTask::activate);
	ClassDB::bind_method(D_METHOD("end_task"), &AbilitySystemTask::end_task);
	ClassDB::bind_method(D_METHOD("is_finished"), &AbilitySystemTask::is_finished);
	ClassDB::bind_method(D_METHOD("set_owner", "owner"), &AbilitySystemTask::set_owner);

	ClassDB::bind_static_method("AbilitySystemTask", D_METHOD("wait_delay", "owner", "delay"), &AbilitySystemTask::wait_delay);
	ClassDB::bind_static_method("AbilitySystemTask", D_METHOD("play_montage", "owner", "animation_name"), &AbilitySystemTask::play_montage);

	BIND_ENUM_CONSTANT(TASK_GENERIC);
	BIND_ENUM_CONSTANT(TASK_WAIT_DELAY);
	BIND_ENUM_CONSTANT(TASK_PLAY_MONTAGE);
	BIND_ENUM_CONSTANT(TASK_WAIT_EVENT);
}

Ref<AbilitySystemTask> AbilitySystemTask::wait_delay(AbilitySystemComponent *p_owner, float p_delay) {
	Ref<AbilitySystemTask> task;
	task.instantiate();
	task->set_owner(p_owner);
	task->task_type = TASK_WAIT_DELAY;
	task->delay_remaining = p_delay;

	if (p_owner) {
		p_owner->register_task(task);
	}
	return task;
}

Ref<AbilitySystemTask> AbilitySystemTask::play_montage(AbilitySystemComponent *p_owner, const StringName &p_anim) {
	Ref<AbilitySystemTask> task;
	task.instantiate();
	task->set_owner(p_owner);
	task->task_type = TASK_PLAY_MONTAGE;
	task->animation_name = p_anim;

	if (p_owner) {
		p_owner->register_task(task);
	}
	return task;
}

AbilitySystemComponent *AbilitySystemTask::get_owner() const {
	if (owner_id.is_null()) {
		return nullptr;
	}
	return Object::cast_to<AbilitySystemComponent>(ObjectDB::get_instance(owner_id));
}

void AbilitySystemTask::set_owner(AbilitySystemComponent *p_owner) {
	if (p_owner) {
		owner_id = p_owner->get_instance_id();
	} else {
		owner_id = ObjectID();
	}
}

void AbilitySystemTask::activate() {
	AbilitySystemComponent *owner = get_owner();
	if (task_type == TASK_PLAY_MONTAGE && owner) {
		owner->play_montage(animation_name);
		started = true;
	} else if (task_type == TASK_WAIT_DELAY && delay_remaining <= 0) {
		end_task();
	}
}

void AbilitySystemTask::tick(float p_delta) {
	if (finished) {
		return;
	}

	switch (task_type) {
		case TASK_WAIT_DELAY: {
			delay_remaining -= p_delta;
			if (delay_remaining <= 0) {
				end_task();
			}
		} break;

		case TASK_PLAY_MONTAGE: {
			if (!started) {
				break;
			}
			AbilitySystemComponent *owner = get_owner();
			if (!owner) {
				end_task();
				break;
			}

			// Verify if any montage-capable node exists in slot
			if (owner->get_animation_player() == nullptr || !owner->is_montage_playing(animation_name)) {
				end_task();
			}
		} break;

		default:
			break;
	}
}

void AbilitySystemTask::end_task() {
	if (finished) {
		return;
	}
	finished = true;
	emit_signal("completed");
}

AbilitySystemTask::AbilitySystemTask() {
}

AbilitySystemTask::~AbilitySystemTask() {
}

} // namespace godot

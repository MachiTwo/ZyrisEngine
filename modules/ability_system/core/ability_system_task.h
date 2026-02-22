/**************************************************************************/
/*  ability_system_task.h                                                 */
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

#include "core/object/ref_counted.h"

class AbilitySystemComponent;

/**
 * AbilitySystemTask
 * Class for asynchronous tasks within abilities (e.g., WaitDelay, PlayMontage).
 */
class AbilitySystemTask : public RefCounted {
	GDCLASS(AbilitySystemTask, RefCounted);

public:
	enum TaskType {
		TASK_GENERIC,
		TASK_WAIT_DELAY,
		TASK_PLAY_MONTAGE,
		TASK_WAIT_EVENT
	};

protected:
	static void _bind_methods();

	AbilitySystemComponent *get_owner() const;
	ObjectID owner_id;
	bool finished = false;
	TaskType task_type = TASK_GENERIC;

	// Task data
	float delay_remaining = 0.0f;
	StringName animation_name;
	bool started = false;

public:
	static Ref<AbilitySystemTask> wait_delay(AbilitySystemComponent *p_owner, float p_delay);
	static Ref<AbilitySystemTask> play_montage(AbilitySystemComponent *p_owner, const StringName &p_anim);

	void activate();
	void tick(float p_delta);
	void end_task();

	bool is_finished() const { return finished; }
	void set_owner(AbilitySystemComponent *p_owner);

	AbilitySystemTask();
	~AbilitySystemTask();
};

VARIANT_ENUM_CAST(AbilitySystemTask::TaskType);

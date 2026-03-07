/**************************************************************************/
/*  ability_system_cue_animation.cpp                                      */
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
#include "modules/ability_system/resources/ability_system_cue_animation.h"
#include "modules/ability_system/core/ability_system_cue_spec.h"
#include "modules/ability_system/scene/ability_system_component.h"
#elif defined(ABILITY_SYSTEM_GDEXTENSION)
#include "src/core/ability_system_cue_spec.h"
#include "src/resources/ability_system_cue_animation.h"
#include "src/scene/ability_system_component.h"
#endif

namespace godot {

void AbilitySystemCueAnimation::execute(Ref<AbilitySystemCueSpec> p_spec) {
	AbilitySystemComponent *asc = p_spec->get_target_asc();
	if (!asc) {
		return;
	}

	// Play animation using the component's play_montage method
	if (!animation_name.is_empty()) {
		Node *target = asc->get_node_ptr(get_node_name());
		asc->play_montage(animation_name, target);
	}

	// Call parent for GDVirtual support
	AbilitySystemCue::execute(p_spec);
}

void AbilitySystemCueAnimation::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_animation_name", "name"), &AbilitySystemCueAnimation::set_animation_name);
	ClassDB::bind_method(D_METHOD("get_animation_name"), &AbilitySystemCueAnimation::get_animation_name);

	ADD_PROPERTY(PropertyInfo(Variant::STRING, "animation_name"), "set_animation_name", "get_animation_name");
}

AbilitySystemCueAnimation::AbilitySystemCueAnimation() {
}

AbilitySystemCueAnimation::~AbilitySystemCueAnimation() {
}

} // namespace godot

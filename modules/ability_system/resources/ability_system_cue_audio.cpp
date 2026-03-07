/**************************************************************************/
/*  ability_system_cue_audio.cpp                                          */
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
#include "modules/ability_system/resources/ability_system_cue_audio.h"
#include "modules/ability_system/core/ability_system_cue_spec.h"
#include "modules/ability_system/scene/ability_system_component.h"
#elif defined(ABILITY_SYSTEM_GDEXTENSION)
#include "src/core/ability_system_cue_spec.h"
#include "src/resources/ability_system_cue_audio.h"
#include "src/scene/ability_system_component.h"
#endif

#ifdef ABILITY_SYSTEM_MODULE
#include "scene/2d/audio_stream_player_2d.h"
#include "scene/3d/audio_stream_player_3d.h"
#include "scene/audio/audio_stream_player.h"
#include "servers/audio/audio_stream.h"
#elif defined(ABILITY_SYSTEM_GDEXTENSION)
#include <godot_cpp/classes/audio_stream.hpp>
#include <godot_cpp/classes/audio_stream_player.hpp>
#include <godot_cpp/classes/audio_stream_player2d.hpp>
#include <godot_cpp/classes/audio_stream_player3d.hpp>
#endif

namespace godot {

void AbilitySystemCueAudio::set_audio_stream(const Ref<AudioStream> &p_stream) {
	audio_stream = p_stream;
}

Ref<AudioStream> AbilitySystemCueAudio::get_audio_stream() const {
	return audio_stream;
}

void AbilitySystemCueAudio::execute(Ref<AbilitySystemCueSpec> p_spec) {
	AbilitySystemComponent *asc = p_spec->get_target_asc();
	if (!asc) {
		return;
	}

	// Play audio stream
	if (audio_stream.is_valid()) {
		Node *target = asc->get_node_ptr(get_node_name());
		asc->play_sound(audio_stream, target);
	}

	// Call parent for GDVirtual support
	AbilitySystemCue::execute(p_spec);
}

void AbilitySystemCueAudio::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_audio_stream", "stream"), &AbilitySystemCueAudio::set_audio_stream);
	ClassDB::bind_method(D_METHOD("get_audio_stream"), &AbilitySystemCueAudio::get_audio_stream);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "audio_stream", PROPERTY_HINT_RESOURCE_TYPE, "AudioStream"), "set_audio_stream", "get_audio_stream");
}

AbilitySystemCueAudio::AbilitySystemCueAudio() {
}

AbilitySystemCueAudio::~AbilitySystemCueAudio() {
}

} // namespace godot

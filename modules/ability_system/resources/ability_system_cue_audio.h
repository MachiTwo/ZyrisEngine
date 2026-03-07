/**************************************************************************/
/*  ability_system_cue_audio.h                                            */
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
#include "modules/ability_system/resources/ability_system_cue.h"
#elif defined(ABILITY_SYSTEM_GDEXTENSION)
#include "src/resources/ability_system_cue.h"
#endif

#ifdef ABILITY_SYSTEM_MODULE
#include "servers/audio/audio_stream.h"
#elif defined(ABILITY_SYSTEM_GDEXTENSION)
#include <godot_cpp/classes/audio_stream.hpp>
#endif

namespace godot {

class AbilitySystemCueAudio : public AbilitySystemCue {
	GDCLASS(AbilitySystemCueAudio, AbilitySystemCue);

private:
	Ref<AudioStream> audio_stream;

public:
	void set_audio_stream(const Ref<AudioStream> &p_stream);
	Ref<AudioStream> get_audio_stream() const;

	// Override execute to play audio
	virtual void execute(Ref<AbilitySystemCueSpec> p_spec) override;

protected:
	static void _bind_methods();

	AbilitySystemCueAudio();
	~AbilitySystemCueAudio();
};

} // namespace godot

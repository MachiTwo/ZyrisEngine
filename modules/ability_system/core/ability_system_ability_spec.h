/**************************************************************************/
/*  ability_system_ability_spec.h                                         */
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
#include "modules/ability_system/resources/ability_system_ability.h"

class AbilitySystemAbilitySpec : public Resource {
	GDCLASS(AbilitySystemAbilitySpec, Resource);

protected:
	static void _bind_methods();

private:
	Ref<AbilitySystemAbility> ability;
	bool is_active = false;
	int level = 1;

public:
	void init(Ref<AbilitySystemAbility> p_ability, int p_level = 1);
	Ref<AbilitySystemAbility> get_ability() const { return ability; }

	bool get_is_active() const { return is_active; }
	void set_is_active(bool p_active) { is_active = p_active; }

	int get_level() const { return level; }
	void set_level(int p_level) { level = p_level; }

	AbilitySystemAbilitySpec();
	~AbilitySystemAbilitySpec();
};

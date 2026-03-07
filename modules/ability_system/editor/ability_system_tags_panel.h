/**************************************************************************/
/*  ability_system_tags_panel.h                                           */
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
#include "modules/ability_system/core/ability_system.h"
#elif defined(ABILITY_SYSTEM_GDEXTENSION)
#include "src/core/ability_system.h"
#endif

#ifdef ABILITY_SYSTEM_MODULE
#include "scene/gui/button.h"
#include "scene/gui/line_edit.h"
#else
#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/line_edit.hpp>
#endif
#ifdef ABILITY_SYSTEM_MODULE
#include "scene/gui/tab_container.h"
#else
#include <godot_cpp/classes/tab_container.hpp>
#endif
#ifdef ABILITY_SYSTEM_MODULE
#include "scene/gui/box_container.h"
#include "scene/gui/tree.h"
#else
#include <godot_cpp/classes/tree.hpp>
#include <godot_cpp/classes/v_box_container.hpp>
#endif

namespace godot {

class AbilitySystemTagsPanel : public VBoxContainer {
	GDCLASS(AbilitySystemTagsPanel, VBoxContainer);

	LineEdit *add_tag_edit = nullptr;
	Button *add_tag_button = nullptr;
	LineEdit *search_edit = nullptr;
	TabContainer *tabs = nullptr;
	Tree *name_tags_tree = nullptr;
	Tree *cond_tags_tree = nullptr;

	void _add_tag();
	void _add_tag_text(const String &p_tag);
	void _on_search_changed(const String &p_text);
	void _tag_removed(Object *p_item, int p_column, int p_id, MouseButton p_button);
	void _add_sub_tag(Object *p_item, int p_column, int p_id, MouseButton p_button);

	void _update_tree(Tree *p_tree, AbilitySystem::TagType p_type, const String &p_search);
	void _create_tree_items(Tree *p_tree, TreeItem *p_parent, const String &p_prefix, const TypedArray<StringName> &p_tags, AbilitySystem::TagType p_type, const String &p_search);

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	void update_tags();

	AbilitySystemTagsPanel();
};

} // namespace godot

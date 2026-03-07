/**************************************************************************/
/*  ability_system_editor_property.h                                      */
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
#include "scene/gui/dialogs.h"
#else
#include <godot_cpp/classes/accept_dialog.hpp>
#endif
#ifdef ABILITY_SYSTEM_MODULE
#include "scene/gui/button.h"
#else
#include <godot_cpp/classes/button.hpp>
#endif
#ifdef ABILITY_SYSTEM_MODULE
#include "editor/inspector/editor_inspector.h"
#else
#include <godot_cpp/classes/editor_property.hpp>
#endif
#ifdef ABILITY_SYSTEM_MODULE
#include "scene/gui/label.h"
#include "scene/gui/line_edit.h"
#include "scene/gui/option_button.h"
#include "scene/gui/tree.h"
#else
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/line_edit.hpp>
#include <godot_cpp/classes/option_button.hpp>
#include <godot_cpp/classes/tree.hpp>
#endif

namespace godot {

class AbilitySystemEditorPropertySelector : public EditorProperty {
	GDCLASS(AbilitySystemEditorPropertySelector, EditorProperty);

	Button *edit_button = nullptr;
	AcceptDialog *dialog = nullptr;
	LineEdit *search_edit = nullptr;
	Tree *tags_tree = nullptr;
	bool updating = false;

	void _edit_pressed();
	void _update_tree();
	void _tag_toggled();
	void _update_button_text();
	void _search_changed(const String &p_text);

protected:
	static void _bind_methods();

public:
#ifdef ABILITY_SYSTEM_MODULE
	virtual void update_property() override;
#else
	virtual void _update_property() override;
#endif

	AbilitySystemEditorPropertySelector();
};

class AbilitySystemEditorPropertyName : public EditorProperty {
	GDCLASS(AbilitySystemEditorPropertyName, EditorProperty);

	LineEdit *edit = nullptr;
	Label *warning_label = nullptr;
	bool updating = false;

	void _text_changed(const String &p_text);
	void _check_uniqueness(const String &p_text);

protected:
	static void _bind_methods();

public:
#ifdef ABILITY_SYSTEM_MODULE
	virtual void update_property() override;
#else
	virtual void _update_property() override;
#endif

	AbilitySystemEditorPropertyName();
};

class AbilitySystemEditorPropertyTagSelector : public EditorProperty {
	GDCLASS(AbilitySystemEditorPropertyTagSelector, EditorProperty);

	OptionButton *options = nullptr;
	bool updating = false;

	void _option_selected(int p_index);

protected:
	static void _bind_methods();

public:
#ifdef ABILITY_SYSTEM_MODULE
	virtual void update_property() override;
#else
	virtual void _update_property() override;
#endif

	AbilitySystemEditorPropertyTagSelector();
};

} // namespace godot

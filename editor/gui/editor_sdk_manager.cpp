/**************************************************************************/
/*  editor_sdk_manager.cpp                                                */
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

#include "editor_sdk_manager.h"

#include "core/io/dir_access.h"
#include "core/io/file_access.h"
#include "editor/editor_node.h"
#include "editor/editor_string_names.h"
#include "editor/settings/editor_settings.h"
#include "editor/themes/editor_scale.h"
#include "scene/gui/box_container.h"
#include "scene/gui/file_dialog.h"
#include "scene/gui/label.h"
#include "scene/gui/tree.h"

void EditorSDKManager::_update_sdk_list() {
	sdk_list->clear();
	TreeItem *root = sdk_list->create_item();

	for (int i = 0; i < sdks.size(); i++) {
		const SDKInfo &sdk = sdks[i];
		TreeItem *item = sdk_list->create_item(root);
		item->set_text(0, sdk.name);

		String path = EDITOR_GET(sdk.setting_path);
		bool valid = _is_sdk_valid(sdk, path);

		if (path.is_empty()) {
			item->set_text(1, TTR("Not Configured"));
			item->set_custom_color(1, get_theme_color(SNAME("error_color"), EditorStringName(Editor)));
		} else if (valid) {
			item->set_text(1, path);
			item->set_custom_color(1, get_theme_color(SNAME("success_color"), EditorStringName(Editor)));
		} else {
			item->set_text(1, TTR("Invalid Path (Missing files)"));
			item->set_custom_color(1, get_theme_color(SNAME("warning_color"), EditorStringName(Editor)));
		}

		item->add_button(2, get_theme_icon(SNAME("Folder"), EditorStringName(Editor)), 0, false, TTR("Browse..."));
		if (!sdk.help_url.is_empty()) {
			item->add_button(2, get_theme_icon(SNAME("Help"), EditorStringName(Editor)), 1, false, TTR("Download SDK / Documentation"));
		}

		item->set_metadata(0, sdk.setting_path);
	}
}

void EditorSDKManager::_sdk_list_button_pressed(Object *p_item, int p_column, int p_id, MouseButton p_button) {
	TreeItem *ti = Object::cast_to<TreeItem>(p_item);
	String setting_path = ti->get_metadata(0);

	if (p_id == 0) { // Browse
		current_editing_sdk = setting_path;
		browse_dialog->set_current_dir(EDITOR_GET(setting_path));
		browse_dialog->popup_centered_ratio();
	} else if (p_id == 1) { // Help
		for (const SDKInfo &sdk : sdks) {
			if (sdk.setting_path == setting_path) {
				OS::get_singleton()->shell_open(sdk.help_url);
				break;
			}
		}
	}
}

void EditorSDKManager::_sdk_path_selected(const String &p_path) {
	EditorSettings::get_singleton()->set_setting(current_editing_sdk, p_path);
	EditorSettings::get_singleton()->save();
	_update_sdk_list();
}

bool EditorSDKManager::_is_sdk_valid(const SDKInfo &p_sdk, const String &p_path) const {
	if (p_path.is_empty()) return false;

	Ref<DirAccess> da = DirAccess::open(p_path);
	if (da.is_null()) return false;

	for (const String &f : p_sdk.required_files) {
		if (!da->file_exists(f)) return false;
	}

	return true;
}

void EditorSDKManager::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			_update_sdk_list();
		} break;
		case NOTIFICATION_THEME_CHANGED: {
			_update_sdk_list();
		} break;
	}
}

void EditorSDKManager::popup_sdk_manager() {
	_update_sdk_list();
	popup_centered(Size2(800, 400) * EDSCALE);
}

void EditorSDKManager::_bind_methods() {
	ClassDB::bind_method(D_METHOD("_sdk_list_button_pressed"), &EditorSDKManager::_sdk_list_button_pressed);
	ClassDB::bind_method(D_METHOD("_sdk_path_selected"), &EditorSDKManager::_sdk_path_selected);
}

EditorSDKManager::EditorSDKManager() {
	set_title(TTR("Third-party SDK Manager"));
	set_hide_on_ok(true);

	VBoxContainer *main_vb = memnew(VBoxContainer);
	add_child(main_vb);

	Label *help_label = memnew(Label);
	help_label->set_text(TTR("Configure the paths to external SDKs required for platform-specific features (Steam Cloud, Google Play Games, etc)."));
	help_label->set_autowrap_mode(TextServer::AUTOWRAP_WORD_SMART);
	main_vb->add_child(help_label);

	sdk_list = memnew(Tree);
	sdk_list->set_columns(3);
	sdk_list->set_column_titles_visible(true);
	sdk_list->set_column_title(0, TTR("SDK Name"));
	sdk_list->set_column_title(1, TTR("Path"));
	sdk_list->set_column_title(2, TTR("Actions"));
	sdk_list->set_column_expand(0, true);
	sdk_list->set_column_expand(1, true);
	sdk_list->set_column_expand(2, false);
	sdk_list->set_column_custom_minimum_width(2, 80 * EDSCALE);
	sdk_list->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	sdk_list->connect("button_clicked", callable_mp(this, &EditorSDKManager::_sdk_list_button_pressed));
	main_vb->add_child(sdk_list);

	browse_dialog = memnew(FileDialog);
	browse_dialog->set_file_mode(FileDialog::FILE_MODE_OPEN_DIR);
	browse_dialog->set_access(FileDialog::ACCESS_FILESYSTEM);
	browse_dialog->connect("dir_selected", callable_mp(this, &EditorSDKManager::_sdk_path_selected));
	add_child(browse_dialog);

	// Setup SDKs
	{
		SDKInfo steam;
		steam.name = "Steamworks SDK";
		steam.setting_path = "export/sdks/steamworks_sdk_path";
		steam.help_url = "https://partner.steamgames.com/";
		steam.required_files.push_back("public/steam/steam_api.h");
		sdks.push_back(steam);

		SDKInfo gplay;
		gplay.name = "Google Play Games C++ SDK";
		gplay.setting_path = "export/sdks/gpg_sdk_path";
		gplay.help_url = "https://developers.google.com/games/services/cpp/downloads";
		gplay.required_files.push_back("include/gpg/gpg.h");
		sdks.push_back(gplay);

		SDKInfo xbox;
		xbox.name = "Microsoft GDK (Xbox)";
		xbox.setting_path = "export/sdks/xbox_gdk_path";
		xbox.help_url = "https://github.com/microsoft/GDK";
		xbox.required_files.push_back("Microsoft.GDK.PC/Include/GDK.h");
		sdks.push_back(xbox);

		SDKInfo psn;
		psn.name = "PlayStation SDK";
		psn.setting_path = "export/sdks/playstation_sdk_path";
		psn.help_url = "https://partners.playstation.net/";
		// PSN files are under NDA, but we check for a known header as a marker
		psn.required_files.push_back("target/include/sdk_common.h");
		sdks.push_back(psn);
	}
}

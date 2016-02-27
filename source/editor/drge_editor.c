// Public domain. See "unlicense" statement at the end of dr_ge.h.


// Opens all of the files specified on the command line as "--editor <file>".
bool drge_editor__open_files_on_cmdline_callback(const char* key, const char* value, void* pUserData)
{
    drge_editor* pEditor = pUserData;

    if (strcmp(key, "editor") == 0) {
        drge_editor_open_file(pEditor, value);  // Doesn't matter if this fails - we just silently ignore it.
    }

    return true;
}

void drge_editor__open_files_on_cmdline(drge_editor* pEditor)
{
    if (pEditor == NULL) {
        return;
    }

    dr_parse_cmdline(&pEditor->pContext->cmdline, drge_editor__open_files_on_cmdline_callback, pEditor);
}


// Called when the main window is wanting to close.
void drge_editor__on_main_window_close(ak_window* pMainWindow)
{
    drge_editor* pEditor = *(drge_editor**)ak_get_application_extra_data(ak_get_window_application(pMainWindow));
    assert(pEditor != NULL);

    drge_editor_close(pEditor);
}

// Called from dr_appkit when a log message has been received.
void drge_editor__on_log_from_appkit(ak_application* pAKApp, const char* message)
{
    drge_editor* pEditor = *(drge_editor**)ak_get_application_extra_data(pAKApp);
    assert(pEditor != NULL);

    drge_log(pEditor->pContext, message);
}

// Called from dr_appkit when the default config needs to be retrieved.
const char* drge_editor__on_get_default_config(ak_application* pAKApp)
{
    assert(pAKApp != NULL);

    return
        "InitialLayout \"Default\"\n"
        "Layout \"Default\"\n"
        "Window application 0 0 1280 720 true \"dr_ge Editor\" MainWindow\n"
        "    Panel hsplit 22\n"
        "        Panel\n"
        "            Tool \"DRGE.Editor.MainMenu\"\n"
        "            /Tool\n"
        "        /Panel\n"
        "\n"
        "        Panel hsplit-bottom 0 \"MainContainer\"\n"
        "            Panel \"MainContent\"\n"
        "            /Panel\n"
        "\n"
        "            Panel \"CommandBarContainer\"\n"
        "            /Panel\n"
        "        /Panel\n"
        "    /Panel\n"
        "/Window\n"
        "/Layout";
}

// Called from dr_appkit when the application has started running.
bool drge_editor__on_run(ak_application* pAKApp)
{
    drge_editor* pEditor = *(drge_editor**)ak_get_application_extra_data(pAKApp);
    assert(pEditor != NULL);

    // The main window needs to be configured.
    pEditor->pMainWindow = ak_get_window_by_name(pAKApp, "MainWindow");
    if (pEditor->pMainWindow == NULL) {
        ak_error(pAKApp, "[DEVELOPER NOTE] Could not find MainWindow in config.");
        return false;
    }

    // The title of the main window should be updated to something more application-specific.
    char title[256];
    strncpy_s(title, sizeof(title), pEditor->pContext->name, _TRUNCATE);
    strncat_s(title, sizeof(title), " Editor", _TRUNCATE);
    ak_set_window_title(pEditor->pMainWindow, title);

    // When the main window is closed we want to close the editor completely.
    ak_window_set_on_close(pEditor->pMainWindow, drge_editor__on_main_window_close);


    // We need to open every file specified on the command line as "--editor <file>".
    drge_editor__open_files_on_cmdline(pEditor);

    return true;
}

void drge_editor__on_key_down(ak_application* pAKApp, ak_window* pWindow, drgui_key key, int stateFlags)
{
    (void)pWindow;

    drge_editor* pEditor = *(drge_editor**)ak_get_application_extra_data(pAKApp);
    assert(pEditor != NULL);

    drgui_key shortcutKey = toupper((int)key);

    if (shortcutKey == 'S' && (stateFlags & AK_KEY_STATE_CTRL_DOWN) != 0) {
        if ((stateFlags & AK_KEY_STATE_SHIFT_DOWN) != 0) {
            ak_handle_action(pAKApp, "File.SaveAll");
        } else {
            ak_handle_action(pAKApp, "File.Save");
        }

        return;
    }

    if (shortcutKey == 'A' && (stateFlags & AK_KEY_STATE_CTRL_DOWN) != 0) {
        ak_handle_action(pAKApp, "Edit.SelectAll");
    }

    if (shortcutKey == 'Z' && (stateFlags & AK_KEY_STATE_CTRL_DOWN) != 0) {
        ak_handle_action(pAKApp, "Edit.Undo");
    }
    if (shortcutKey == 'Y' && (stateFlags & AK_KEY_STATE_CTRL_DOWN) != 0) {
        ak_handle_action(pAKApp, "Edit.Redo");
    }

    if (shortcutKey == 'C' && (stateFlags & AK_KEY_STATE_CTRL_DOWN) != 0) {
        ak_handle_action(pAKApp, "Edit.Copy");
    }
    if (shortcutKey == 'X' && (stateFlags & AK_KEY_STATE_CTRL_DOWN) != 0) {
        ak_handle_action(pAKApp, "Edit.Cut");
    }
    if (shortcutKey == 'V' && (stateFlags & AK_KEY_STATE_CTRL_DOWN) != 0) {
        ak_handle_action(pAKApp, "Edit.Paste");
    }
}

void drge_editor__on_handle_action(ak_application* pAKApp, const char* pActionName)
{
    drge_editor* pEditor = *(drge_editor**)ak_get_application_extra_data(pAKApp);
    assert(pEditor != NULL);

    // Basic editing actions.
    if (strcmp(pActionName, "Edit.SelectAll") == 0 ||
        strcmp(pActionName, "Edit.Undo") == 0 ||
        strcmp(pActionName, "Edit.Redo") == 0 ||
        strcmp(pActionName, "Edit.Copy") == 0 ||
        strcmp(pActionName, "Edit.Cut") == 0 ||
        strcmp(pActionName, "Edit.Paste") == 0 ||
        strcmp(pActionName, "Edit.Delete") == 0)
    {
        ak_tool_handle_action(drge_editor_get_focused_subeditor(pEditor), pActionName);
        return;
    }


    if (strcmp(pActionName, "Global.Exit") == 0) {
        drge_editor_close(pEditor);
        return;
    }
}

static int drge_editor__on_exec(ak_application* pApplication, const char* cmd)
{
    (void)pApplication;

    char func[256];
    cmd = dr_next_token(cmd, func, sizeof(func));
    if (cmd == NULL) {
        return 0;
    }

    int result = 0;
    return result;
}


drge_subeditor* drge_editor__create_sub_editor_by_type(drge_editor* pEditor, const char* filePath, drge_asset_type type)
{
    // Sub-editors expect the file path to be absolute. We'll need to handle that.
    char absolutePath[DRVFS_MAX_PATH];
    if (!drvfs_find_absolute_path(pEditor->pContext->pVFS, filePath, absolutePath, sizeof(absolutePath))) {
        drge_errorf(pEditor->pContext, "Failed to find absolute path of \"%s\". Check that the file exists.", filePath);
        return NULL;
    }

    // We'll treat unknown types as text files.
    if (type == drge_asset_type_text || type == drge_asset_type_unknown) {
        return drge_editor_create_text_editor(pEditor, absolutePath);
    }

    if (type == drge_asset_type_image) {
        return drge_editor_create_image_editor(pEditor, absolutePath);
    }


    // Unsupported type.
    return NULL;
}

drge_subeditor* drge_editor__find_subeditor_by_absolute_path(drge_editor* pEditor, const char* absolutePath)
{
    if (pEditor == NULL || absolutePath == NULL) {
        return NULL;
    }

    for (drgui_element* pTabGroup = ak_find_first_panel_by_type(pEditor->pAKApp, "DRGE.Editor.TabGroup"); pTabGroup != NULL; pTabGroup = ak_find_next_panel_by_type(pEditor->pAKApp, pTabGroup, "DRGE.Editor.TabGroup")) {
        for (drgui_element* pSubEditor = ak_panel_get_first_tool(pTabGroup); pSubEditor != NULL; pSubEditor = ak_panel_get_next_tool(pTabGroup, pSubEditor)) {
            if (ak_is_tool_of_type(pSubEditor, DRGE_EDITOR_TOOL_TYPE_SUB_EDITOR)) {
                if (drpath_equal(drge_subeditor_get_absolute_path((drge_subeditor*)pSubEditor), absolutePath)) {
                    return (drge_subeditor*)pSubEditor;
                }
            }
        }
    }

    return NULL;
}

drgui_element* drge_editor__get_or_create_focused_tab_group(drge_editor* pEditor)
{
    if (pEditor == NULL) {
        return NULL;
    }

    if (pEditor->pFocusedTabGroup != NULL) {
        return pEditor->pFocusedTabGroup;
    }


    // If we get here it means there is no focused tab group and we need to create one. The initial one needs to be contained within the MainContent panel.
    drgui_element* pMainContent = ak_find_first_panel_by_type(pEditor->pAKApp, "MainContent");
    if (pMainContent == NULL) {
        drge_error(pEditor->pContext, "Invalid editor config. Required panels are not defined.");
        return NULL;
    }

    // The only way to create a child panel is to split the parent. We'll place the initial editor group in the first group.
    ak_panel_split(pMainContent, ak_panel_split_axis_vertical, drgui_get_width(pMainContent));
    ak_panel_enable_ratio_split(pMainContent);

    pEditor->pFocusedTabGroup = ak_panel_get_split_panel_1(pMainContent);
    if (pEditor->pFocusedTabGroup == NULL) {
        return NULL;
    }

    ak_panel_set_type(pEditor->pFocusedTabGroup, "DRGE.Editor.TabGroup");
    ak_panel_set_tab_options(pEditor->pFocusedTabGroup, AK_PANEL_OPTION_SHOW_TOOL_TABS | AK_PANEL_OPTION_SHOW_CLOSE_BUTTON_ON_TABS);

    // TODO: Add a close button graphic to tabs.
    //ak_panel_set_tab_close_button_image(pEditorGroup, ide_get_red_cross_image(ide_get_image_manager(pApplication)));

    return pEditor->pFocusedTabGroup;
}


drge_editor* drge_create_editor(drge_context* pContext)
{
    if (pContext == NULL) {
        return NULL;
    }

    drge_editor* pEditor = malloc(sizeof(*pEditor));
    if (pEditor == NULL) {
        return NULL;
    }

    pEditor->pContext         = pContext;
    pEditor->pMainWindow      = NULL;
    pEditor->pFocusedTabGroup = NULL;

    // ak_application configuration.
    pEditor->pAKApp = ak_create_application(pContext->name, sizeof(&pEditor), &pEditor);
    if (pEditor->pAKApp == NULL) {
        free(pEditor);
        return NULL;
    }

    ak_set_log_callback(pEditor->pAKApp, drge_editor__on_log_from_appkit);
    ak_set_on_default_config(pEditor->pAKApp, drge_editor__on_get_default_config);
    ak_set_on_run(pEditor->pAKApp, drge_editor__on_run);
    ak_set_on_key_down(pEditor->pAKApp, drge_editor__on_key_down);
    ak_set_on_handle_action(pEditor->pAKApp, drge_editor__on_handle_action);
    ak_set_on_exec(pEditor->pAKApp, drge_editor__on_exec);

    return pEditor;
}

void drge_delete_editor(drge_editor* pEditor)
{
    if (pEditor == NULL) {
        return;
    }

    ak_delete_application(pEditor->pAKApp);
    free(pEditor);
}


int drge_editor_run(drge_editor* pEditor)
{
    if (pEditor == NULL) {
        return -1;
    }

    return ak_run_application(pEditor->pAKApp);
}

void drge_editor_close(drge_editor* pEditor)
{
    ak_post_quit_message(pEditor->pAKApp, 0);
}


bool drge_editor_open_file(drge_editor* pEditor, const char* filePath)
{
    if (pEditor == NULL || filePath == NULL) {
        return false;
    }

    if (drge_editor_try_focus_file_by_path(pEditor, filePath)) {
        return true;    // The file is already open.
    }

    // If we get here it means the file is not already open and we need to open it. Upon opening the file we'll need to add it to a tab group. Thus,
    // the first thing we want to do is ensure we actually have one.
    drgui_element* pFocusedTabGroup = drge_editor__get_or_create_focused_tab_group(pEditor);
    if (pFocusedTabGroup == NULL) {
        drge_errorf(pEditor->pContext, "Failed to open \"%s\" because there is no tab group to place it in.", filePath);
        return false;
    }


    drge_subeditor* pSubEditor = drge_editor__create_sub_editor_by_type(pEditor, filePath, drge_get_asset_type_from_path(filePath));
    if (pSubEditor == NULL) {
        return false;
    }

    ak_panel_attach_tool(pFocusedTabGroup, (drgui_element*)pSubEditor);


    return true;
}

bool drge_editor_try_focus_file_by_path(drge_editor* pEditor, const char* filePath)
{
    if (pEditor == NULL || filePath == NULL) {
        return false;
    }

    char absolutePath[DRVFS_MAX_PATH];
    if (!drvfs_find_absolute_path(pEditor->pContext->pVFS, filePath, absolutePath, sizeof(absolutePath))) {
        return false;
    }

    drge_subeditor* pSubEditor = drge_editor__find_subeditor_by_absolute_path(pEditor, absolutePath);
    if (pSubEditor == NULL) {
        return false;
    }

    // At this point we have the sub editor and we just need to focus it and make sure it's tab is in view.
    ak_panel_activate_tool(ak_get_tool_panel((drgui_element*)pSubEditor), (drgui_element*)pSubEditor);
    drgui_tab_move_into_view(ak_get_tool_tab((drgui_element*)pSubEditor));

    return true;
}

drge_subeditor* drge_editor_get_focused_subeditor(drge_editor* pEditor)
{
    // The focused tool is the active tool on the focused panel. Note that this returns NULL if the tool is not a sub-editor.
    drgui_element* pFocusedPanel = drge_editor__get_or_create_focused_tab_group(pEditor);
    if (pFocusedPanel == NULL) {
        return NULL;
    }

    drgui_element* pActiveTool = ak_panel_get_active_tool(pFocusedPanel);
    if (ak_is_tool_of_type(pActiveTool, DRGE_EDITOR_TOOL_TYPE_SUB_EDITOR)) {
        return pActiveTool;
    }

    return NULL;
}

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
        return drge_editor_create_text_editor(pEditor, filePath);
    }

    if (type == drge_asset_type_image) {
        return drge_editor_create_image_editor(pEditor, filePath);
    }


    // Unsupported type.
    return NULL;
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

    pEditor->pContext    = pContext;
    pEditor->pMainWindow = NULL;

    // ak_application configuration.
    pEditor->pAKApp = ak_create_application(pContext->name, sizeof(&pEditor), &pEditor);
    if (pEditor->pAKApp == NULL) {
        free(pEditor);
        return NULL;
    }

    ak_set_log_callback(pEditor->pAKApp, drge_editor__on_log_from_appkit);
    ak_set_on_default_config(pEditor->pAKApp, drge_editor__on_get_default_config);
    ak_set_on_run(pEditor->pAKApp, drge_editor__on_run);


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

    // If we get here it means the file is not already open and we need to open it.
    drge_subeditor* pSubEditor = drge_editor__create_sub_editor_by_type(pEditor, filePath, drge_get_asset_type_from_path(filePath));
    if (pSubEditor == NULL) {
        return false;
    }

    return true;
}

bool drge_editor_try_focus_file_by_path(drge_editor* pEditor, const char* filePath)
{
    // TODO: Implement Me.
    return false;
}

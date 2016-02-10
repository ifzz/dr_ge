// Public domain. See "unlicense" statement at the end of dr_ge.h.

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
        "            Tool \"DRGE.Editor.Menu.Main\"\n"
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


    // TODO: Parse the command line and look for every occurance of "--editor" and open files for those that specify a file name with it.

    return true;
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
    if (drge_editor_try_focus_file_by_path(pEditor, filePath)) {
        return true;    // The file is already open.
    }

    // The file is not already open so we'll need to create a tool of the appropriate type and link it to the file.

    return false;
}

bool drge_editor_try_focus_file_by_path(drge_editor* pEditor, const char* filePath)
{
    // TODO: Implement Me.
    return false;
}

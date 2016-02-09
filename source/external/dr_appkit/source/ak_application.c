// Public domain. See "unlicense" statement at the end of this file.

struct ak_application
{
    /// The name of the application.
    char name[AK_MAX_APPLICATION_NAME_LENGTH];

    /// The virtual file system context. This is mainly used for opening log, theme and configuration files.
    drvfs_context* pVFS;

    /// The log file.
    drvfs_file* pLogFile;

    /// The log callback.
    ak_log_proc onLog;


    /// The drawing context.
    dr2d_context* pDrawingContext;

    /// A pointer to the GUI context.
    drgui_context* pGUI;

    /// The application's theme.
    ak_theme theme;


    /// The function to call just before the application enters into it's main loop.
    ak_run_proc onRun;

    /// A pointer to the function to call when the default layout config is required. This will be called when
    /// layout config file could not be found.
    ak_layout_config_proc onGetDefaultConfig;

    /// A pointer to the function to call when a custom tool needs to be instantiated.
    ak_create_tool_proc onCreateTool;

    /// A pointer to the function to call when a custom tool needs to be deleted.
    ak_delete_tool_proc onDeleteTool;

    /// A pointer to the function to call when a key down event needs to be handled.
    ak_application_on_key_down_proc onKeyDown;

    /// A pointer to the function to call when a key up event needs to be handled.
    ak_application_on_key_up_proc onKeyUp;

    /// A pointer to the function to call when a panel is activated.
    ak_application_on_panel_activated_proc onPanelActivated;

    /// A pointer to the function to call when a panel is deactivated.
    ak_application_on_panel_deactivated_proc onPanelDeactivated;

    /// A pointer to the function to call when a tool is activated.
    ak_application_on_tool_activated_proc onToolActivated;

    /// A pointer to the function to call when a tool is deactivated.
    ak_application_on_tool_deactivated_proc onToolDeactivated;

    /// A pointer to the function to call when an action needs to be handled.
    ak_application_on_handle_action_proc onHandleAction;

    /// A pointer to the function to call when a command needs to be handled.
    ak_application_on_exec_proc onExec;


    /// We need to keep track of every existing window that is owned by the application. We implement this as a linked list, with this item being the first. The
    /// first window is considered to be the primary window.
    ak_window* pFirstWindow;

    /// The window that previously had the keyboard focus. This is only used for auto-hiding management of popup windows.
    ak_window* pPrevFocusedWindow;


    // Platform Specific.
#ifdef AK_USE_WIN32
    /// The window to associate timers with.
    HWND hTimerWnd;
#endif


    /// The size of the extra data, in bytes.
    size_t extraDataSize;

    /// A pointer to the extra data associated with the application.
    char pExtraData[1];
};


/// Enter's into the main application loop.
static int ak_main_loop(ak_application* pApplication);

/// Loads and apply's the main config.
static bool ak_load_and_apply_config(ak_application* pApplication);

/// Applies the given config to the given application object.
static bool ak_apply_config(ak_application* pApplication, ak_config* pConfig);

/// Applies the given layout object to the given application object.
static bool ak_apply_layout(ak_application* pApplication, ak_layout* pLayout, drgui_element* pElement);

/// Recursively deletes the tools that are within the given panel.
static void ak_delete_tools_recursive(ak_application* pApplication, drgui_element* pPanel);


#ifdef AK_USE_WIN32
static LRESULT TimerWindowProcWin32(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    return DefWindowProc(hWnd, msg, wParam, lParam);
}
#endif

ak_application* ak_create_application(const char* pName, size_t extraDataSize, const void* pExtraData)
{
    ak_application* pApplication = malloc(sizeof(ak_application) + extraDataSize - sizeof(pApplication->pExtraData));
    if (pApplication != NULL)
    {
        // Name.
        if (pName != NULL) {
            strcpy_s(pApplication->name, sizeof(pApplication->name), pName);
        } else {
            strcpy_s(pApplication->name, sizeof(pApplication->name), "easy_appkit");
        }


        // File system.
        pApplication->pVFS = drvfs_create_context();
        if (pApplication->pVFS == NULL) {
            free(pApplication);
            return NULL;
        }


        // Logging
        pApplication->onLog = NULL;

        char logDirPath[DRVFS_MAX_PATH];
        if (ak_get_log_file_folder_path(pApplication, logDirPath, sizeof(logDirPath)))
        {
            const unsigned int maxAttempts = 10;

            char path[DRVFS_MAX_PATH];
            for (unsigned int iAttempt = 0; iAttempt < maxAttempts; ++iAttempt)
            {
                char istr[16];
                snprintf(istr, 16, "%d", iAttempt);

                strcpy_s(path, sizeof(path), logDirPath);
                drpath_append(path, sizeof(path), drpath_file_name(pApplication->name));
                strcat_s(path, sizeof(path), istr);
                strcat_s(path, sizeof(path), ".log");

                pApplication->pLogFile = drvfs_open(pApplication->pVFS, path, DRVFS_WRITE | DRVFS_TRUNCATE, 0);
                if (pApplication->pLogFile != NULL)
                {
                    // We were able to open the log file, so break here.
                    break;
                }
            }
        }
        else
        {
            pApplication->pLogFile = NULL;
        }



        // GUI.
#ifdef AK_USE_WIN32
        pApplication->pDrawingContext = dr2d_create_context_gdi();
#endif
#ifdef AK_USE_GTK
        pApplication->pDrawingContext = dr2d_create_context_cairo();
#endif
        if (pApplication->pDrawingContext == NULL) {
            free(pApplication);
            return NULL;
        }

        pApplication->pGUI = drgui_create_context_dr_2d(pApplication->pDrawingContext);
        if (pApplication->pGUI == NULL) {
            dr2d_delete_context(pApplication->pDrawingContext);
            free(pApplication);
            return NULL;
        }


        // Theme.
        memset(&pApplication->theme, 0, sizeof(pApplication->theme));


        // Callbacks.
        pApplication->onRun              = NULL;
        pApplication->onGetDefaultConfig = NULL;
        pApplication->onCreateTool       = NULL;
        pApplication->onDeleteTool       = NULL;
        pApplication->onKeyDown          = NULL;
        pApplication->onKeyUp            = NULL;
        pApplication->onToolActivated    = NULL;
        pApplication->onToolDeactivated  = NULL;
        pApplication->onHandleAction     = NULL;
        pApplication->onExec             = NULL;


        // Windows.
        pApplication->pFirstWindow       = NULL;
        pApplication->pPrevFocusedWindow = NULL;


        // Platform Specific
#ifdef AK_USE_WIN32
        pApplication->hTimerWnd = NULL;

        WNDCLASSEXW timerWC;
        memset(&timerWC, 0, sizeof(timerWC));
        timerWC.cbSize        = sizeof(timerWC);
        timerWC.lpfnWndProc   = (WNDPROC)TimerWindowProcWin32;
        timerWC.lpszClassName = L"AK_TimerHWND";
        timerWC.style         = CS_OWNDC;
        if (RegisterClassExW(&timerWC)) {
            pApplication->hTimerWnd = CreateWindowExW(0, L"AK_TimerHWND", L"", 0, 0, 0, 0, 0, NULL, NULL, GetModuleHandle(NULL), NULL);
        }
#endif


        // Extra data.
        pApplication->extraDataSize = extraDataSize;

        if (extraDataSize > 0 && pExtraData != NULL) {
            memcpy(pApplication->pExtraData, pExtraData, extraDataSize);
        }


        // Everything looks good at this point, so now is a good time to setup the window system. This is the part that makes
        // ak_create_application() not thread safe.
        ak_init_platform();


        // The GUI needs to be connected to the window system.
        ak_connect_gui_to_window_system(pApplication->pGUI);
    }

    return pApplication;
}

void ak_delete_application(ak_application* pApplication)
{
    if (pApplication == NULL) {
        return;
    }

    // Windows need to be deleted.
    ak_delete_all_application_windows(pApplication);

    // Theme.
    ak_theme_unload(&pApplication->theme);

    // GUI.
    drgui_delete_context(pApplication->pGUI);
    dr2d_delete_context(pApplication->pDrawingContext);

    // Logs.
    drvfs_close(pApplication->pLogFile);

    // File system.
    drvfs_delete_context(pApplication->pVFS);


#ifdef AK_USE_WIN32
    // The timer window.
    DestroyWindow(pApplication->hTimerWnd);
#endif

    // The platform layer needs to be uninitialized at the end just before freeing the application. This ensures all window
    // and whatnot are destroyed first.
    ak_uninit_platform();


    // Free the application object last.
    free(pApplication);
}


int ak_run_application(ak_application* pApplication)
{
    if (pApplication == NULL) {
        return -1;
    }

    // The first thing to do when running the application is to load the default config and apply it. If a config
    // file cannot be found, a default config will be requested. If the default config fails, the config will fail
    // and an error code will be returned.
    if (!ak_load_and_apply_config(pApplication)) {
        return -2;
    }

    // After we've loaded and applied the config, but before entering the main loop, we need to let the host application
    // do some custom initialization which we'll achieve via a callback function.
    if (pApplication->onRun) {
        pApplication->onRun(pApplication);
    }

    // At this point the config should be loaded. Now we just enter the main loop.
    return ak_main_loop(pApplication);
}

void ak_post_quit_message(ak_application* pApplication, int exitCode)
{
    if (pApplication == NULL) {
        return;
    }

#ifdef AK_USE_WIN32
    ak_win32_post_quit_message(exitCode);
#endif

#ifdef AK_USE_GTK
    ak_gtk_post_quit_message(exitCode);
#endif
}


void ak_delete_all_application_windows(ak_application* pApplication)
{
    if (pApplication == NULL) {
        return;
    }

    // All tools need to be deleted first before any windows.
    for (ak_window* pWindow = ak_get_application_first_window(pApplication); pWindow != NULL; pWindow = ak_get_application_next_window(pApplication, pWindow))
    {
        ak_delete_tools_recursive(ak_get_window_application(pWindow), ak_get_window_panel(pWindow));
    }


    while (pApplication->pFirstWindow != NULL)
    {
        // Delete the window after the tools.
        ak_delete_window(pApplication->pFirstWindow);
    }
}


const char* ak_get_application_name(ak_application* pApplication)
{
    if (pApplication == NULL) {
        return NULL;
    }

    return pApplication->name;
}

drvfs_context* ak_get_application_vfs(ak_application* pApplication)
{
    if (pApplication == NULL) {
        return NULL;
    }

    return pApplication->pVFS;
}

size_t ak_get_application_extra_data_size(ak_application* pApplication)
{
    if (pApplication == NULL) {
        return 0;
    }

    return pApplication->extraDataSize;
}

void* ak_get_application_extra_data(ak_application* pApplication)
{
    if (pApplication == NULL) {
        return NULL;
    }

    return pApplication->pExtraData;
}

dr2d_context* ak_get_application_drawing_context(ak_application* pApplication)
{
    if (pApplication == NULL) {
        return NULL;
    }

    return pApplication->pDrawingContext;
}

drgui_context* ak_get_application_gui(ak_application* pApplication)
{
    if (pApplication == NULL) {
        return NULL;
    }

    return pApplication->pGUI;
}

ak_theme* ak_get_application_theme(ak_application* pApplication)
{
    if (pApplication == NULL) {
        return NULL;
    }

    return &pApplication->theme;
}


void ak_log(ak_application* pApplication, const char* message)
{
    if (pApplication == NULL) {
        return;
    }


    // Log file.
    if (pApplication->pLogFile != NULL) {
        char dateTime[64];
        dr_datetime_short(dr_now(), dateTime, sizeof(dateTime));

        drvfs_write_string(pApplication->pLogFile, "[");
        drvfs_write_string(pApplication->pLogFile, dateTime);
        drvfs_write_string(pApplication->pLogFile, "]");
        drvfs_write_line  (pApplication->pLogFile, message);
        drvfs_flush(pApplication->pLogFile);
    }


    // Log callback.
    if (pApplication->onLog) {
        pApplication->onLog(pApplication, message);
    }
}

void ak_logf(ak_application* pApplication, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    {
        char msg[4096];
        vsnprintf(msg, sizeof(msg), format, args);

        ak_log(pApplication, msg);
    }
    va_end(args);
}

void ak_warning(ak_application* pApplication, const char* message)
{
    ak_logf(pApplication, "[WARNING] %s", message);
}

void ak_warningf(ak_application* pApplication, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    {
        char msg[4096];
        vsnprintf(msg, sizeof(msg), format, args);

        ak_warning(pApplication, msg);
    }
    va_end(args);
}

void ak_error(ak_application* pApplication, const char* message)
{
    ak_logf(pApplication, "[ERROR] %s", message);
}

void ak_errorf(ak_application* pApplication, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    {
        char msg[4096];
        vsnprintf(msg, sizeof(msg), format, args);

        ak_error(pApplication, msg);
    }
    va_end(args);
}

void ak_set_log_callback(ak_application* pApplication, ak_log_proc proc)
{
    if (pApplication == NULL) {
        return;
    }

    pApplication->onLog = proc;
}

ak_log_proc ak_get_log_callback(ak_application* pApplication)
{
    if (pApplication == NULL) {
        return NULL;
    }

    return pApplication->onLog;
}

bool ak_get_log_file_folder_path(ak_application* pApplication, char* pathOut, size_t pathOutSize)
{
    if (pApplication == NULL) {
        return false;
    }

    if (pathOut == NULL || pathOutSize == 0) {
        return false;
    }


    if (!dr_get_log_folder_path(pathOut, pathOutSize)) {
        return false;
    }

    return drpath_append(pathOut, (unsigned int)pathOutSize, ak_get_application_name(pApplication));
}

bool ak_get_config_file_folder_path(ak_application* pApplication, char* pathOut, size_t pathOutSize)
{
    if (pApplication == NULL) {
        return false;
    }

    if (pathOut == NULL || pathOutSize == 0) {
        return false;
    }

    if (!dr_get_config_folder_path(pathOut, pathOutSize)) {
        return false;
    }

    return drpath_append(pathOut, (unsigned int)pathOutSize, ak_get_application_name(pApplication));
}

bool ak_get_config_file_path(ak_application* pApplication, char* pathOut, size_t pathOutSize)
{
    if (ak_get_config_file_folder_path(pApplication, pathOut, pathOutSize))
    {
        return drpath_append(pathOut, (unsigned int)pathOutSize, drpath_file_name(ak_get_application_name(pApplication))) && strcat_s(pathOut, pathOutSize, ".cfg") == 0;
    }

    return false;
}

bool ak_get_theme_file_path(ak_application* pApplication, char* pathOut, size_t pathOutSize)
{
    if (pApplication == NULL) {
        return false;
    }

    if (pathOut == NULL || pathOutSize == 0) {
        return false;
    }

    // Use the config path as the basis.
    if (!dr_get_config_folder_path(pathOut, pathOutSize)) {
        return false;
    }

    // Append the sub-directory.
    if (!drpath_append(pathOut, (unsigned int)pathOutSize, ak_get_application_name(pApplication))) {
        return false;
    }

    // Append the file name.
    return drpath_append(pathOut, (unsigned int)pathOutSize, drpath_file_name(ak_get_application_name(pApplication))) && strcat_s(pathOut, pathOutSize, ".theme") == 0;
}


void ak_set_on_default_config(ak_application* pApplication, ak_layout_config_proc proc)
{
    if (pApplication == NULL) {
        return;
    }

    pApplication->onGetDefaultConfig = proc;
}

ak_layout_config_proc ak_get_on_default_config(ak_application* pApplication)
{
    if (pApplication == NULL) {
        return NULL;
    }

    return pApplication->onGetDefaultConfig;
}


ak_window* ak_get_element_window(drgui_element* pElement)
{
    if (pElement == NULL) {
        return NULL;
    }

    return ak_get_panel_window(drgui_find_top_level_element(pElement));
}

ak_window* ak_get_window_by_name(ak_application* pApplication, const char* pName)
{
    if (pApplication == NULL) {
        return NULL;
    }

    for (ak_window* pWindow = ak_get_first_window(pApplication); pWindow != NULL; pWindow = ak_get_next_window(pApplication, pWindow))
    {
        if (strcmp(ak_get_window_name(pWindow), pName) == 0) {
            return pWindow;
        }
    }

    return NULL;
}


ak_window* ak_get_first_window(ak_application* pApplication)
{
    if (pApplication == NULL) {
        return NULL;
    }

    return pApplication->pFirstWindow;
}

static ak_window* ak_get_next_non_child_window(ak_application* pApplication, ak_window* pWindow)
{
    if (pApplication == NULL || pWindow == NULL) {
        return NULL;
    }

    // If we have a next sibling we just return that, otherwise we recursively traverse upwards.
    if (ak_get_next_sibling_window(pWindow) != NULL) {
        return ak_get_next_sibling_window(pWindow);
    }

    // At this point we don't have a sibling so we just check our parent.
    return ak_get_next_non_child_window(pApplication, ak_get_parent_window(pWindow));
}

ak_window* ak_get_next_window(ak_application* pApplication, ak_window* pWindow)
{
    if (pApplication == NULL || pWindow == NULL) {
        return NULL;
    }

    // If we have children, just return the next child.
    if (ak_get_first_child_window(pWindow) != NULL) {
        return ak_get_first_child_window(pWindow);
    }

    // At this point we don't have any children, so return the next sibling if we have one.
    return ak_get_next_non_child_window(pApplication, pWindow);

    //return ak_get_next_sibling_window(pWindow);
}

ak_window* ak_get_primary_window(ak_application* pApplication)
{
    for (ak_window* pWindow = ak_get_first_window(pApplication); pWindow != NULL; pWindow = ak_get_next_window(pApplication, pWindow))
    {
        if (ak_get_window_type(pWindow) == ak_window_type_application) {
            return pWindow;
        }
    }

    return NULL;
}


drgui_element* ak_get_first_panel(ak_application* pApplication)
{
    // The first panel is the first panel of the first window.
    ak_window* pFirstWindow = ak_get_first_window(pApplication);
    if (pFirstWindow == NULL) {
        return NULL;
    }

    return ak_get_window_panel(pFirstWindow);
}

static drgui_element* ak_get_next_non_child_panel(ak_application* pApplication, drgui_element* pPanel)
{
    if (pPanel == NULL) {
        return NULL;
    }

    if (pPanel == ak_get_window_panel(ak_get_panel_window(pPanel)))
    {
        // It's the root panel on the window so we just return the first panel of the next window.
        return ak_get_window_panel(ak_get_next_window(pApplication, ak_get_panel_window(pPanel)));
    }
    else
    {
        // It's not a root level window. If <pPanel> is the first split panel of it's parent, we just return the second split panel.
        if (pPanel == ak_panel_get_split_panel_1(ak_panel_get_parent(pPanel)))
        {
            // <pPanel> is the first split panel, so we just return the second.
            return ak_panel_get_split_panel_2(ak_panel_get_parent(pPanel));
        }
        else
        {
            // <pPanel> is not the first split panel (probably the second). In this case we need to get the next panel that is not
            // a child of the parent.
            return ak_get_next_non_child_panel(pApplication, ak_panel_get_parent(pPanel));
        }
    }
}

drgui_element* ak_get_next_panel(ak_application* pApplication, drgui_element* pPanel)
{
    // The next panel depends on where <pPanel> is in relation to it's parent, and whether or not it is split. If it's a split panel we just return the first of
    // the split children. If it's not split, it's a bit more complex.

    if (ak_panel_is_split(pPanel))
    {
        // It's a split panel - just return the first split child.
        return ak_panel_get_split_panel_1(pPanel);
    }
    else
    {
        // It's not a split panel so we just get the next non-child.
        return ak_get_next_non_child_panel(pApplication, pPanel);
    }
}


drgui_element* ak_find_first_panel_by_type(ak_application* pApplication, const char* pPanelType)
{
    for (drgui_element* pPanel = ak_get_first_panel(pApplication); pPanel != NULL; pPanel = ak_get_next_panel(pApplication, pPanel))
    {
        if (ak_panel_is_of_type(pPanel, pPanelType)) {
            return pPanel;
        }
    }

    return NULL;
}

drgui_element* ak_find_next_panel_by_type(ak_application* pApplication, drgui_element* pPanel, const char* pPanelType)
{
    for (drgui_element* pNextPanel = ak_get_next_panel(pApplication, pPanel); pNextPanel != NULL; pNextPanel = ak_get_next_panel(pApplication, pNextPanel))
    {
        if (ak_panel_is_of_type(pNextPanel, pPanelType)) {
            return pNextPanel;
        }
    }

    return NULL;
}


void ak_set_on_run(ak_application* pApplication, ak_run_proc proc)
{
    if (pApplication == NULL) {
        return;
    }

    pApplication->onRun = proc;
}

ak_run_proc ak_get_on_run(ak_application* pApplication)
{
    if (pApplication == NULL) {
        return NULL;
    }

    return pApplication->onRun;
}


void ak_set_on_create_tool(ak_application* pApplication, ak_create_tool_proc proc)
{
    if (pApplication == NULL) {
        return;
    }

    pApplication->onCreateTool = proc;
}

ak_create_tool_proc ak_get_on_create_tool(ak_application* pApplication)
{
    if (pApplication == NULL) {
        return NULL;
    }

    return pApplication->onCreateTool;
}


void ak_set_on_delete_tool(ak_application* pApplication, ak_delete_tool_proc proc)
{
    if (pApplication == NULL) {
        return;
    }

    pApplication->onDeleteTool = proc;
}

ak_delete_tool_proc ak_get_on_delete_tool(ak_application* pApplication)
{
    if (pApplication == NULL) {
        return NULL;
    }

    return pApplication->onDeleteTool;
}


drgui_element* ak_create_tool_by_type_and_attributes(ak_application* pApplication, ak_window* pWindow, const char* type, const char* attributes)
{
    if (pApplication == NULL || type == NULL) {
        return false;
    }

    // First check for built-in tools.


    // At this point we know the tool type is not a built-in so we need to give the host application a chance to
    // instantiate it in case it's a custom tool type.
    if (pApplication->onCreateTool) {
        return pApplication->onCreateTool(pApplication, pWindow, type, attributes);
    }

    return NULL;
}

bool ak_application_delete_tool(ak_application* pApplication, drgui_element* pTool, bool force)
{
    if (pApplication == NULL || pTool == NULL) {
        return false;
    }

    // First check for built-in tools.


    // At this point we know the tool type is not a built-in so we need to give the host application a chance to
    // delete it in case it's a custom tool type.
    if (pApplication->onDeleteTool) {
        return pApplication->onDeleteTool(pApplication, pTool, force);
    }

    return false;
}


void ak_set_on_key_down(ak_application* pApplication, ak_application_on_key_down_proc proc)
{
    if (pApplication == NULL) {
        return;
    }

    pApplication->onKeyDown = proc;
}

void ak_set_on_key_up(ak_application* pApplication, ak_application_on_key_up_proc proc)
{
    if (pApplication == NULL) {
        return;
    }

    pApplication->onKeyUp = proc;
}


void ak_set_on_panel_activated(ak_application* pApplication, ak_application_on_panel_activated_proc proc)
{
    if (pApplication == NULL) {
        return;
    }

    pApplication->onPanelActivated = proc;
}

void ak_set_on_panel_deactivated(ak_application* pApplication, ak_application_on_panel_deactivated_proc proc)
{
    if (pApplication == NULL) {
        return;
    }

    pApplication->onPanelDeactivated = proc;
}


void ak_set_on_tool_activated(ak_application* pApplication, ak_application_on_tool_activated_proc proc)
{
    if (pApplication == NULL) {
        return;
    }

    pApplication->onToolActivated = proc;
}

void ak_set_on_tool_deactivated(ak_application* pApplication, ak_application_on_tool_deactivated_proc proc)
{
    if (pApplication == NULL) {
        return;
    }

    pApplication->onToolDeactivated = proc;
}


void ak_set_on_handle_action(ak_application* pApplication, ak_application_on_handle_action_proc proc)
{
    if (pApplication == NULL) {
        return;
    }

    pApplication->onHandleAction = proc;
}

void ak_handle_action(ak_application* pApplication, const char* pActionName)
{
    if (pApplication == NULL || pActionName == NULL) {
        return;
    }

    if (pApplication->onHandleAction) {
        pApplication->onHandleAction(pApplication, pActionName);
    }
}


void ak_set_on_exec(ak_application* pApplication, ak_application_on_exec_proc proc)
{
    if (pApplication == NULL) {
        return;
    }

    pApplication->onExec = proc;
}

int ak_exec(ak_application* pApplication, const char* cmd)
{
    if (pApplication == NULL || cmd == NULL) {
        return 0;
    }

    if (strcmp(cmd, "quit") == 0 || strcmp(cmd, "exit") == 0) {
        ak_post_quit_message(pApplication, 0);
        return 0;
    }

    if (pApplication->onExec) {
        return pApplication->onExec(pApplication, cmd);
    }

    return 0;
}


//// Timers ////
#ifdef AK_USE_WIN32
struct ak_timer
{
    /// The value returned by SetTimer().
    UINT_PTR tagWin32;

    /// The window associated with the timer.
    HWND hWnd;

    /// The timeout in milliseconds.
    unsigned int timeoutInMilliseconds;

    /// The callback function.
    ak_timer_proc callback;

    /// The user data passed to ak_create_timer().
    void* pUserData;
};

static VOID CALLBACK ak_timer_proc_win32(HWND hWnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    (void)hWnd;
    (void)uMsg;
    (void)dwTime;

    ak_timer* pTimer = (ak_timer*)idEvent;
    if (pTimer == NULL) {
        assert(false);
    }

    if (pTimer->callback != NULL) {
        pTimer->callback(pTimer, pTimer->pUserData);
    }
}


ak_timer* ak_create_timer(ak_application* pApplication, unsigned int timeoutInMilliseconds, ak_timer_proc callback, void* pUserData)
{
    if (pApplication == NULL) {
        return NULL;
    }

    ak_timer* pTimer = malloc(sizeof(*pTimer));
    if (pTimer == NULL) {
        return NULL;
    }

    // On Win32 we need to associate the timer with a window.
    pTimer->tagWin32 = SetTimer(pApplication->hTimerWnd, (UINT_PTR)pTimer, timeoutInMilliseconds, ak_timer_proc_win32);
    if (pTimer->tagWin32 == 0) {
        free(pTimer);
        return NULL;
    }

    pTimer->hWnd                  = pApplication->hTimerWnd;
    pTimer->timeoutInMilliseconds = timeoutInMilliseconds;
    pTimer->callback              = callback;
    pTimer->pUserData             = pUserData;

    return pTimer;
}

void ak_delete_timer(ak_timer* pTimer)
{
    if (pTimer == NULL) {
        return;
    }

    KillTimer(pTimer->hWnd, pTimer->tagWin32);
    free(pTimer);
}
#endif

#ifdef AK_USE_GTK
#include <gtk/gtk.h>

struct ak_timer
{
    /// The GTK timer ID.
    guint timerID;

    /// The timeout in milliseconds.
    unsigned int timeoutInMilliseconds;

    /// The callback function.
    ak_timer_proc callback;

    /// The user data passed to ak_create_timer().
    void* pUserData;
};

static gboolean ak_timer_proc_gtk(gpointer data)
{
    ak_timer* pTimer = (ak_timer*)data;
    if (pTimer == NULL) {
        assert(false);
        return 0;
    }

    if (pTimer->callback != NULL) {
        pTimer->callback(pTimer, pTimer->pUserData);
    }

    return true;
}


ak_timer* ak_create_timer(ak_application* pApplication, unsigned int timeoutInMilliseconds, ak_timer_proc callback, void* pUserData)
{
    if (pApplication == NULL) {
        return NULL;
    }

    ak_timer* pTimer = malloc(sizeof(*pTimer));
    if (pTimer == NULL) {
        return NULL;
    }

    pTimer->timerID               = g_timeout_add(timeoutInMilliseconds, ak_timer_proc_gtk, pTimer);
    pTimer->timeoutInMilliseconds = timeoutInMilliseconds;
    pTimer->callback              = callback;
    pTimer->pUserData             = pUserData;

    return pTimer;
}

void ak_delete_timer(ak_timer* pTimer)
{
    if (pTimer == NULL) {
        return;
    }

    g_source_remove(pTimer->timerID);
    free(pTimer);
}
#endif



///////////////////////////////////////////////////////////////////////////////
//
// Private APIs
//
///////////////////////////////////////////////////////////////////////////////

static int ak_main_loop(ak_application* pApplication)
{
#ifdef AK_USE_WIN32
    (void)pApplication;

    MSG msg;
    BOOL bRet;
    while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0)
    {
        if (bRet == -1)
        {
            // Unknown error.
            return -42;
        }

        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    return 0;
#endif

#ifdef AK_USE_GTK
    (void)pApplication;
    gtk_main();
    return 0;   // TODO: Return proper error codes.
#endif
}

static void ak_on_config_error(void* pUserData, const char* message)
{
    ak_application* pApplication = pUserData;
    assert(pApplication != NULL);

    ak_errorf(pApplication, "[CONFIG] %s", message);
}

static bool ak_load_and_apply_config(ak_application* pApplication)
{
    assert(pApplication != NULL);


    // We load the theme first because it contains the data we need for drawing the window which will be shown when the
    // main config is applied.
    char themePath[DRVFS_MAX_PATH];
    if (ak_get_theme_file_path(pApplication, themePath, sizeof(themePath))) {
        ak_theme_load_from_file(pApplication, &pApplication->theme, themePath);
    }


    // We first need to try and open the config file. If we can't find it we need to try and load the default config. If both
    // fail, we need to return false.
    char configPath[DRVFS_MAX_PATH];
    if (ak_get_config_file_path(pApplication, configPath, sizeof(configPath)))
    {
        drvfs_file* pConfigFile = drvfs_open(ak_get_application_vfs(pApplication), configPath, DRVFS_READ, 0);
        if (pConfigFile != NULL)
        {
            ak_config config;
            if (ak_parse_config_from_file(&config, pConfigFile, ak_on_config_error, pApplication))
            {
                bool result = ak_apply_config(pApplication, &config);

                ak_uninit_config(&config);
                return result;
            }
        }
    }


    // If we get here we want to try loading the default config.
    if (pApplication->onGetDefaultConfig)
    {
        ak_config config;
        if (ak_parse_config_from_string(&config, pApplication->onGetDefaultConfig(pApplication), ak_on_config_error, pApplication))
        {
            bool result = ak_apply_config(pApplication, &config);

            ak_uninit_config(&config);
            return result;
        }
    }


    // If we get here, both configs have failed to load.
    return false;
}

static bool ak_apply_config(ak_application* pApplication, ak_config* pConfig)
{
    assert(pApplication != NULL);
    assert(pConfig      != NULL);

    // We need to find the initial layout object.
    ak_layout* pInitialLayout = ak_config_find_root_layout_by_name(pConfig, pConfig->currentLayoutName);
    if (pInitialLayout == NULL) {
        pInitialLayout = pConfig->pRootLayout->pFirstChild;

        if (pInitialLayout == NULL) {
            return false;
        }
    }

    return ak_apply_layout(pApplication, pInitialLayout, NULL);
}

static bool ak_apply_layout(ak_application* pApplication, ak_layout* pLayout, drgui_element* pWorkingPanel)
{
    assert(pApplication != NULL);
    assert(pLayout      != NULL);

    if (strcmp(pLayout->type, AK_LAYOUT_TYPE_LAYOUT) == 0)
    {
        // It's a root level layout object - we just iterate over every child and call this function recursively.
        assert(pWorkingPanel == NULL);

        for (ak_layout* pChild = pLayout->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling)
        {
            if (!ak_apply_layout(pApplication, pChild, pWorkingPanel))
            {
                return false;
            }
        }
    }
    else if (strcmp(pLayout->type, AK_LAYOUT_TYPE_WINDOW) == 0)
    {
        // It's an application window.
        ak_window* pParentWindow = ak_get_element_window(pWorkingPanel);

        ak_window* pWindow = ak_create_window(pApplication, ak_window_type_application, pParentWindow, 0, NULL);
        if (pWindow == NULL) {
            return false;
        }

        // We need to parse the attributes.
        ak_window_layout_attributes attr;
        if (!ak_parse_window_layout_attributes(pLayout->attributes, &attr)) {
            return false;
        }

        ak_set_window_name(pWindow, attr.name);
        ak_set_window_title(pWindow, attr.title);
        ak_set_window_position(pWindow, attr.posX, attr.posY);
        ak_set_window_size(pWindow, attr.width, attr.height);

        if (attr.maximized) {
            ak_show_window_maximized(pWindow);
        } else {
            ak_show_window(pWindow);
        }


        // There should only be one child item, and it should be a panel. If not, it's an error.
        if (pLayout->pFirstChild == NULL) {
            return false;
        }

        return ak_apply_layout(pApplication, pLayout->pFirstChild, ak_get_window_panel(pWindow));
    }
    else if (strcmp(pLayout->type, AK_LAYOUT_TYPE_PANEL) == 0)
    {
        // It's a panel. If it's a split panel we just split it and load the next two panels which correspond to the two split partitions. If
        // it's not split, we just leave it be and iterate over what should be a list of tools.

        ak_panel_layout_attributes attr;
        if (!ak_parse_panel_layout_attributes(pLayout->attributes, &attr)) {
            return false;
        }

        // We only set the panel type for panels that are not the top-level panel.
        if (pWorkingPanel != ak_get_window_panel(ak_get_panel_window(pWorkingPanel))) {
            if (attr.type[0] != '\0') {
                ak_panel_set_type(pWorkingPanel, attr.type);
            }
        } else {
            if (attr.type[0] != '\0') {
                ak_warning(pApplication, "Attempting to set panel type of a top-level panel which is illegal.");
            }
        }


        if (attr.splitAxis == ak_panel_split_axis_none)
        {
            // It's not a split panel which means the next items should be just a list of tools.
            for (ak_layout* pChild = pLayout->pFirstChild; pChild != NULL; pChild = pChild->pNextSibling)
            {
                if (!ak_apply_layout(pApplication, pChild, pWorkingPanel))
                {
                    return false;
                }
            }
        }
        else
        {
            // It's a split panel which means there should be two children. If not, it's an error.
            ak_layout* pChildLayout1 = pLayout->pFirstChild;
            if (pChildLayout1 == NULL) {
                return false;
            }

            ak_layout* pChildLayout2 = pChildLayout1->pNextSibling;
            if (pChildLayout2 == NULL) {
                return false;
            }


            if (!ak_panel_split(pWorkingPanel, attr.splitAxis, attr.splitPos)) {
                return false;
            }


            return ak_apply_layout(pApplication, pChildLayout1, ak_panel_get_split_panel_1(pWorkingPanel)) && ak_apply_layout(pApplication, pChildLayout2, ak_panel_get_split_panel_2(pWorkingPanel));
        }
    }
    else if (strcmp(pLayout->type, AK_LAYOUT_TYPE_TOOL) == 0)
    {
        // It's a tool. Tools are instantiated based on it's type and attributes. The type is the first token in the config script's
        // attribute list and the attributes is the remainder.
        //
        // When instantiating tools, we don't actually fail - we just silently ignore it. Thus, we never return false at this point.

        char toolType[AK_MAX_WINDOW_NAME_LENGTH];
        const char* toolAttributes = dr_first_non_whitespace(dr_next_token(pLayout->attributes, toolType, sizeof(toolType)));
        if (toolAttributes != NULL)
        {
            drgui_element* pTool = ak_create_tool_by_type_and_attributes(pApplication, ak_get_element_window(pWorkingPanel), toolType, toolAttributes);
            if (pTool != NULL) {
                if (ak_panel_attach_tool(pWorkingPanel, pTool)) {
                    drgui_show(pTool);
                }
            }
        }
    }

    return true;
}

static void ak_delete_tools_recursive(ak_application* pApplication, drgui_element* pPanel)
{
    if (pApplication == NULL || pPanel == NULL) {
        return;
    }

    // If it's split, we call this recursively.
    if (ak_panel_get_split_axis(pPanel) != ak_panel_split_axis_none)
    {
        ak_delete_tools_recursive(pApplication, ak_panel_get_split_panel_1(pPanel));
        ak_delete_tools_recursive(pApplication, ak_panel_get_split_panel_2(pPanel));
    }
    else
    {
        drgui_element* pFirstTool = NULL;
        while ((pFirstTool = ak_panel_get_first_tool(pPanel)) != NULL)
        {
            ak_application_delete_tool(pApplication, pFirstTool, true);    // "true" means to force deletion of the tool.

            // As a safety precaution to avoid an infinite loop, if the first tool was not detached we'll forcefully detach it.
            if (pFirstTool == ak_panel_get_first_tool(pPanel)) {
                ak_panel_detach_tool(pPanel, pFirstTool);
            }
        }
    }
}





///////////////////////////////////////////////////////////////////////////////
//
// Private APIs Declared in ak_application_private.h
//
///////////////////////////////////////////////////////////////////////////////

ak_window* ak_get_application_first_window(ak_application* pApplication)
{
    assert(pApplication != NULL);
    return pApplication->pFirstWindow;
}

ak_window* ak_get_application_next_non_child_window(ak_application* pApplication, ak_window* pWindow)
{
    if (pApplication == NULL || pWindow == NULL) {
        return NULL;
    }

    ak_window* pNextSibling = ak_get_next_sibling_window(pWindow);
    if (pNextSibling != NULL) {
        return pNextSibling;
    }

    return ak_get_application_next_non_child_window(pApplication, ak_get_parent_window(pWindow));
}

ak_window* ak_get_application_next_window(ak_application* pApplication, ak_window* pWindow)
{
    assert(pApplication != NULL);
    assert(pWindow != NULL);

    ak_window* pFirstChild = ak_get_first_child_window(pWindow);
    if (pFirstChild != NULL) {
        return pFirstChild;
    }

    return ak_get_application_next_non_child_window(pApplication, pWindow);
}


void ak_application_on_close_window(ak_window* pWindow)
{
    assert(pWindow != NULL);

    ak_window_on_close(pWindow);
}

bool ak_application_on_hide_window(ak_window* pWindow, unsigned int flags)
{
    assert(pWindow != NULL);

    return ak_window_on_hide(pWindow, flags);
}

bool ak_application_on_show_window(ak_window* pWindow)
{
    assert(pWindow != NULL);

    return ak_window_on_show(pWindow);
}

void ak_application_on_activate_window(ak_window* pWindow)
{
    assert(pWindow != NULL);

    ak_window_on_activate(pWindow);
}

void ak_application_on_deactivate_window(ak_window* pWindow)
{
    assert(pWindow != NULL);

    ak_window_on_deactivate(pWindow);
    //ak_application_hide_non_ancestor_popups(pWindow);
}

void ak_application_on_focus_window(ak_window* pWindow)
{
    assert(pWindow != NULL);

    ak_application* pApplication = ak_get_window_application(pWindow);
    assert(pApplication != NULL);

    pApplication->pPrevFocusedWindow = NULL;
}

void ak_application_on_unfocus_window(ak_window* pWindow)
{
    assert(pWindow != NULL);

    ak_application* pApplication = ak_get_window_application(pWindow);
    assert(pApplication != NULL);

    pApplication->pPrevFocusedWindow = pWindow;

    // Popup window's that lose focus need to be hidden.
    if (ak_get_window_type(pWindow) == ak_window_type_popup) {
        ak_hide_window(pWindow, AK_AUTO_HIDE_FROM_LOST_FOCUS);
    }
}

void ak_application_on_mouse_enter(ak_window* pWindow)
{
    assert(pWindow != NULL);

    ak_window_on_mouse_enter(pWindow);
}

void ak_application_on_mouse_leave(ak_window* pWindow)
{
    assert(pWindow != NULL);

    ak_window_on_mouse_leave(pWindow);

    // Let the GUI know about the event.
    drgui_post_inbound_event_mouse_leave(ak_get_window_panel(pWindow));
}

void ak_application_on_mouse_move(ak_window* pWindow, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    assert(pWindow != NULL);

    // Let the GUI know about the event.
    drgui_post_inbound_event_mouse_move(ak_get_window_panel(pWindow), relativeMousePosX, relativeMousePosY, stateFlags);
}

void ak_application_on_mouse_button_down(ak_window* pWindow, int mouseButton, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    assert(pWindow != NULL);

    ak_window_on_mouse_button_down(pWindow, mouseButton, relativeMousePosX, relativeMousePosY);

    // Any popup window that is not an ancestor of the input window needs to be hidden.
    ak_application_hide_non_ancestor_popups(pWindow);

    // Let the GUI know about the event.
    drgui_post_inbound_event_mouse_button_down(ak_get_window_panel(pWindow), mouseButton, relativeMousePosX, relativeMousePosY, stateFlags);
}

void ak_application_on_mouse_button_up(ak_window* pWindow, int mouseButton, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    assert(pWindow != NULL);

    ak_window_on_mouse_button_up(pWindow, mouseButton, relativeMousePosX, relativeMousePosY);

    // Let the GUI know about the event.
    drgui_post_inbound_event_mouse_button_up(ak_get_window_panel(pWindow), mouseButton, relativeMousePosX, relativeMousePosY, stateFlags);
}

void ak_application_on_mouse_button_dblclick(ak_window* pWindow, int mouseButton, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    assert(pWindow != NULL);

    ak_window_on_mouse_button_dblclick(pWindow, mouseButton, relativeMousePosX, relativeMousePosY);

    // Let the GUI know about the event.
    drgui_post_inbound_event_mouse_button_dblclick(ak_get_window_panel(pWindow), mouseButton, relativeMousePosX, relativeMousePosY, stateFlags);
}

void ak_application_on_mouse_wheel(ak_window* pWindow, int delta, int relativeMousePosX, int relativeMousePosY, int stateFlags)
{
    assert(pWindow != NULL);

    ak_window_on_mouse_wheel(pWindow, delta, relativeMousePosX, relativeMousePosY);

    // Let the GUI know about the event.
    drgui_post_inbound_event_mouse_wheel(ak_get_window_panel(pWindow), delta, relativeMousePosX, relativeMousePosY, stateFlags);
}

void ak_application_on_key_down(ak_window* pWindow, drgui_key key, int stateFlags)
{
    assert(pWindow != NULL);

    ak_window_on_key_down(pWindow, key, stateFlags);
    drgui_post_inbound_event_key_down(ak_get_window_panel(pWindow)->pContext, key, stateFlags);

    ak_application* pApplication = ak_get_window_application(pWindow);
    if (pApplication != NULL && pApplication->onKeyDown) {
        pApplication->onKeyDown(pApplication, pWindow, key, stateFlags);
    }
}

void ak_application_on_key_up(ak_window* pWindow, drgui_key key, int stateFlags)
{
    assert(pWindow != NULL);

    ak_window_on_key_up(pWindow, key, stateFlags);
    drgui_post_inbound_event_key_up(ak_get_window_panel(pWindow)->pContext, key, stateFlags);

    ak_application* pApplication = ak_get_window_application(pWindow);
    if (pApplication != NULL && pApplication->onKeyUp) {
        pApplication->onKeyUp(pApplication, pWindow, key, stateFlags);
    }
}

void ak_application_on_printable_key_down(ak_window* pWindow, unsigned int character, int stateFlags)
{
    assert(pWindow != NULL);

    ak_window_on_printable_key_down(pWindow, character, stateFlags);
    drgui_post_inbound_event_printable_key_down(ak_get_window_panel(pWindow)->pContext, character, stateFlags);
}


void ak_application_on_panel_activated(ak_application* pApplication, drgui_element* pPanel)
{
    if (pApplication == NULL || pPanel == NULL) {
        return;
    }

    if (pApplication->onPanelActivated) {
        pApplication->onPanelActivated(pApplication, pPanel);
    }
}

void ak_application_on_panel_deactivated(ak_application* pApplication, drgui_element* pPanel)
{
    if (pApplication == NULL || pPanel == NULL) {
        return;
    }

    if (pApplication->onPanelDeactivated) {
        pApplication->onPanelDeactivated(pApplication, pPanel);
    }
}


void ak_application_on_tool_activated(ak_application* pApplication, drgui_element* pTool)
{
    if (pApplication == NULL || pTool == NULL) {
        return;
    }

    if (pApplication->onToolActivated) {
        pApplication->onToolActivated(pApplication, pTool);
    }
}

void ak_application_on_tool_deactivated(ak_application* pApplication, drgui_element* pTool)
{
    if (pApplication == NULL || pTool == NULL) {
        return;
    }

    if (pApplication->onToolDeactivated) {
        pApplication->onToolDeactivated(pApplication, pTool);
    }
}


void ak_application_track_top_level_window(ak_window* pWindow)
{
    assert(pWindow != NULL);

    ak_application* pApplication = ak_get_window_application(pWindow);
    assert(pApplication != NULL);

    if (pApplication->pFirstWindow != NULL)
    {
        ak_set_prev_sibling_window(pApplication->pFirstWindow, pWindow);
        ak_set_next_sibling_window(pWindow, pApplication->pFirstWindow);
    }

    pApplication->pFirstWindow = pWindow;
}

void ak_application_untrack_top_level_window(ak_window* pWindow)
{
    assert(pWindow != NULL);

    ak_application* pApplication = ak_get_window_application(pWindow);
    assert(pApplication != NULL);

    if (pApplication->pFirstWindow != NULL)
    {
        if (pApplication->pFirstWindow == pWindow) {
            pApplication->pFirstWindow = ak_get_next_sibling_window(pWindow);
        }

        if (ak_get_next_sibling_window(pWindow) != NULL) {
            ak_set_prev_sibling_window(ak_get_next_sibling_window(pWindow), ak_get_prev_sibling_window(pWindow));
        }

        if (ak_get_prev_sibling_window(pWindow) != NULL) {
            ak_set_next_sibling_window(ak_get_prev_sibling_window(pWindow), ak_get_next_sibling_window(pWindow));
        }

        ak_set_next_sibling_window(pWindow, NULL);
        ak_set_prev_sibling_window(pWindow, NULL);
    }
}


void ak_application_hide_non_ancestor_popups(ak_window* pWindow)
{
    assert(pWindow != NULL);

    ak_application* pApplication = ak_get_window_application(pWindow);
    assert(pApplication != NULL);

    for (ak_window* pOtherWindow = ak_get_first_window(pApplication); pOtherWindow != NULL; pOtherWindow = ak_get_next_window(pApplication, pOtherWindow))
    {
        if (pOtherWindow != pWindow && ak_get_window_type(pOtherWindow) == ak_window_type_popup && !ak_is_window_ancestor(pOtherWindow, pWindow))
        {
            ak_hide_window(pOtherWindow, AK_AUTO_HIDE_FROM_LOST_FOCUS);
        }
    }
}



/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>
*/

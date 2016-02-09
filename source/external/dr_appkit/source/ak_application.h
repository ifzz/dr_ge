// Public domain. See "unlicense" statement at the end of this file.

#ifndef ak_application_h
#define ak_application_h

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ak_application ak_application;
typedef struct ak_window ak_window;
typedef struct ak_theme ak_theme;
typedef struct ak_timer ak_timer;
typedef struct drgui_context drgui_context;
typedef struct drgui_element drgui_element;
typedef struct dr2d_context dr2d_context;
typedef struct drvfs_context drvfs_context;

typedef bool             (* ak_run_proc)           (ak_application* pApplication);
typedef void             (* ak_log_proc)           (ak_application* pApplication, const char* message);
typedef const char*      (* ak_layout_config_proc) (ak_application* pApplication);
typedef drgui_element* (* ak_create_tool_proc)   (ak_application* pApplication, ak_window* pWindow, const char* type, const char* attributes);
typedef bool             (* ak_delete_tool_proc)   (ak_application* pApplication, drgui_element* pTool, bool force);

typedef void (* ak_application_on_key_down_proc)         (ak_application* pApplication, ak_window* pWindow, drgui_key key, int stateFlags);
typedef void (* ak_application_on_key_up_proc)           (ak_application* pApplication, ak_window* pWindow, drgui_key key, int stateFlags);
typedef void (* ak_application_on_panel_activated_proc)  (ak_application* pApplication, drgui_element* pPanel);
typedef void (* ak_application_on_panel_deactivated_proc)(ak_application* pApplication, drgui_element* pPanel);
typedef void (* ak_application_on_tool_activated_proc)   (ak_application* pApplication, drgui_element* pTool);
typedef void (* ak_application_on_tool_deactivated_proc) (ak_application* pApplication, drgui_element* pTool);
typedef void (* ak_application_on_handle_action_proc)    (ak_application* pApplication, const char* pActionName);
typedef int  (* ak_application_on_exec_proc)             (ak_application* pApplication, const char* cmd);

typedef void (* ak_timer_proc)(ak_timer* pTimer, void* pUserData);


/// Creates a new application object.
///
/// @remarks
///     <pName> is used for determining where to read and write user configuration files, log files, etc.
///     <pName> cannot be longer than AK_MAX_APPLICATION_NAME_LENGTH which is defined in ak_build_config.h.
///     <pName> can be a path-style string such as "MyApplication/MySubApplication". In this case, the log
///     theme, config, etc. files will be opened based on this path.
///     @par
///     User-defined data can be associated with an application by specifying the number of extra bytes
///     that are required. Use ak_application_get_extra_data() to retrieve a pointer to a buffer that can
///     be used as the storage for the user-defined data. The user-defined data is initially undefined.
///     @par
///     <pExtraData> is a pointer to the initial user-defined data. This can be null, in which case the
///     extra data is left initially undefined. This will create a copy of the extra data.
///     @par
///     Multiple application objects can be created per process, however this function is not thread-safe
///     due to a small section of global state that's required for the Windows build.
ak_application* ak_create_application(const char* pName, size_t extraDataSize, const void* pExtraData);

/// Deletes an application object that was created with ak_create_application().
///
/// @remarks
///     This function is not thread safe.
void ak_delete_application(ak_application* pApplication);


/// Begins running the application.
///
/// @return The result code.
///
/// @remarks
///     Only a single application instance can be run at a time.
///     @par
///     This loop should be terminated with a call to ak_post_quit_message(). A call to ak_post_quit_message()
///     will cause this function to return, after which another application can begin running if required.
///     @par
///     This is where the application will load configs, create windows, create GUI elements and enter into
///     the main loop.
///     @par
///     A return value of 0 indicates the application was terminated naturally.
int ak_run_application(ak_application* pApplication);

/// Posts a quit message.
///
/// @remarks
///     This is how the main loop must be terminated. Do not terminate from the main loop with ak_delete_application().
void ak_post_quit_message(ak_application* pApplication, int exitCode);


/// Destroys every window in the application.
void ak_delete_all_application_windows(ak_application* pApplication);


/// Retrieves the name of the application.
///
/// @remarks
///     If null was passed to ak_create_application(), the returned string will be equal to the value of
///     AK_DEFAULT_APPLICATION_NAME which is defined in ak_build_config.h.
const char* ak_get_application_name(ak_application* pApplication);

/// Retrieves a pointer ot the application's virtual file system context.
drvfs_context* ak_get_application_vfs(ak_application* pApplication);

/// Retrieves the size of the extra data that is associated with the application.
size_t ak_get_application_extra_data_size(ak_application* pApplication);

/// Retrieves a pointer to the buffer containing the user-defined data.
void* ak_get_application_extra_data(ak_application* pApplication);

/// Retrieves a pointer to the GUI drawing context.
dr2d_context* ak_get_application_drawing_context(ak_application* pApplication);

/// Retrieves a pointer to the GUI context associated with the application.
drgui_context* ak_get_application_gui(ak_application* pApplication);

/// Retrieves a pointer to the object representing the application's theme.
ak_theme* ak_get_application_theme(ak_application* pApplication);


/// Posts a log message.
void ak_log(ak_application* pApplication, const char* message);

/// Posts a formatted log message.
void ak_logf(ak_application* pApplication, const char* format, ...);

/// Posts a warning to the log.
void ak_warning(ak_application* pApplication, const char* message);

/// Posts a formatted warning log message.
void ak_warningf(ak_application* pApplication, const char* format, ...);

/// Posts an error log message.
void ak_error(ak_application* pApplication, const char* message);

/// Posts a formatted error log message.
void ak_errorf(ak_application* pApplication, const char* format, ...);

/// Sets the function to call when a log message is posted.
void ak_set_log_callback(ak_application* pApplication, ak_log_proc proc);

/// Retrieves a pointer to the log callback function, if any.
ak_log_proc ak_get_log_callback(ak_application* pApplication);


/// Retrieves the path of the directory that contains the log file.
bool ak_get_log_file_folder_path(ak_application* pApplication, char* pathOut, size_t pathOutSize);

/// Retrieves the path of the directory that contains the config file.
bool ak_get_config_file_folder_path(ak_application* pApplication, char* pathOut, size_t pathOutSize);

/// Retrieves the path of the config file.
bool ak_get_config_file_path(ak_application* pApplication, char* pathOut, size_t pathOutSize);

/// Retrieves the path of the theme file.
bool ak_get_theme_file_path(ak_application* pApplication, char* pathOut, size_t pathOutSize);


/// Sets the function to call when the default config of a layout is required.
void ak_set_on_default_config(ak_application* pApplication, ak_layout_config_proc proc);

/// Retrieves the function to call when the default config of a layout is required.
ak_layout_config_proc ak_get_on_default_config(ak_application* pApplication);


/// Retrieves the window that the given element is contained in.
///
/// @remarks
///     This runs in linear time.
ak_window* ak_get_element_window(drgui_element* pElement);

/// Retrieves the first occurance of the window with the given name.
ak_window* ak_get_window_by_name(ak_application* pApplication, const char* pName);


/// Retrieves a pointer to the first window in the given application.
ak_window* ak_get_first_window(ak_application* pApplication);

/// Retrieves a pointer to the next window in the given application.
ak_window* ak_get_next_window(ak_application* pApplication, ak_window* pWindow);

/// Retrieves a pointer to the primary window.
///
/// @remarks
///     The primary window is the first application window that was created.
ak_window* ak_get_primary_window(ak_application* pApplication);


/// Retrieves a pointer to the first panel.
///
/// @remarks
///     Use this in conjunction with ak_get_next_panel() to iterate over every panel in the main hierarchy.
///     @par
///     This will only find panels that are part of the main hierarchy and will not find those that are part of custom tools.
drgui_element* ak_get_first_panel(ak_application* pApplication);

/// Retrieves a pointer to the next panel.
drgui_element* ak_get_next_panel(ak_application* pApplication, drgui_element* pPanel);


/// Retrieves a pointer to the panel with the given type.
///
/// @remarks
///     This is a slow recursive function.
///     @par
///     This will only find panels that are part of the main hierarchy and will not find those that are part of custom tools.
drgui_element* ak_find_first_panel_by_type(ak_application* pApplication, const char* pPanelType);

/// Retrieves a pointer to the next panel with the given type.
///
/// @remarks
///     This will only find panels that are part of the main hierarchy and will not find those that are part of custom tools.
drgui_element* ak_find_next_panel_by_type(ak_application* pApplication, drgui_element* pPanel, const char* pPanelType);


/// Sets the function to call just before the application enters into the main loop.
///
/// @remarks
///     If the callback returns true, running will continue like normal. If it returns false, the application will be terminated.
void ak_set_on_run(ak_application* pApplication, ak_run_proc proc);

/// Retrieves the function to call just before the application enters into the main loop.
ak_run_proc ak_get_on_run(ak_application* pApplication);

/// Sets the callback function to call when a custom tool needs to be instantiated.
///
/// @remarks
///     The tool must be created with ak_create_tool() and without an initial parent. The tool will be attached
///     to the appropriate panel at a higher level.
void ak_set_on_create_tool(ak_application* pApplication, ak_create_tool_proc proc);

/// Retrieves a pointer to the callback function that is called when a custom tool needs to be instantiated.
ak_create_tool_proc ak_get_on_create_tool(ak_application* pApplication);


/// Sets the callback function to call when a custom tool needs to be deleted.
void ak_set_on_delete_tool(ak_application* pApplication, ak_delete_tool_proc proc);

/// Retrieves a pointer to the callback function that is called when a custom tool needs to be deleted.
ak_delete_tool_proc ak_get_on_delete_tool(ak_application* pApplication);


/// Creates a tool from it's type and attributes.
///
/// @remarks
///     If the tool type is not a built-in type, it will call the onCreateTool callback.
drgui_element* ak_create_tool_by_type_and_attributes(ak_application* pApplication, ak_window* pWindow, const char* type, const char* attributes);

/// Deletes the given tool.
///
/// @return True if the tool was deleted; false if not.
///
/// @remarks
///     This may return false if the tool contains something like a text editor or whatnot and a confirmation dialog needs
///     to be shown before deleting the tool. In this case, false would be returned so that the tool is not deleted immediately,
///     and instead tool will be attempted to be deleted later on in response to the dialog.
bool ak_application_delete_tool(ak_application* pApplication, drgui_element* pTool, bool force);


/// Sets the function to call when the application received a key down event.
///
/// @remarks
///     This callback can be used to do things like handling shortcuts.
void ak_set_on_key_down(ak_application* pApplication, ak_application_on_key_down_proc proc);

/// Sets the function to call when the application receives a key up event.
///
/// @remarks
///     This callback can be used to do things like handling shortcuts.
void ak_set_on_key_up(ak_application* pApplication, ak_application_on_key_up_proc proc);


/// Sets the function to call when a panel is activated.
void ak_set_on_panel_activated(ak_application* pApplication, ak_application_on_panel_activated_proc proc);

/// Sets the function to call when a panel is deactivated.
void ak_set_on_panel_deactivated(ak_application* pApplication, ak_application_on_panel_deactivated_proc proc);


/// Sets the function to call when a tool is activated.
void ak_set_on_tool_activated(ak_application* pApplication, ak_application_on_tool_activated_proc proc);

/// Sets the function to call when a tool is deactivated.
void ak_set_on_tool_deactivated(ak_application* pApplication, ak_application_on_tool_deactivated_proc proc);


/// Sets the function to call when an event needs to be handled.
void ak_set_on_handle_action(ak_application* pApplication, ak_application_on_handle_action_proc proc);

/// Handles the given action.
void ak_handle_action(ak_application* pApplication, const char* pActionName);


/// Sets the function to call when a command needs to be executed.
void ak_set_on_exec(ak_application* pApplication, ak_application_on_exec_proc proc);

/// A generic function for executing a command.
///
/// @remarks
///     The return value is specific to the callback that was set with ak_set_on_exec(). The return value is not
///     used by easy_appkit and exists only for host applications.
int ak_exec(ak_application* pApplication, const char* cmd);


/// Creates a timer associated with the given application.
///
/// @remarks
///     The given callback function will be called from the main application loop.
ak_timer* ak_create_timer(ak_application* pApplication, unsigned int timeoutInMilliseconds, ak_timer_proc callback, void* pUserData);

/// Deletes the given timer.
void ak_delete_timer(ak_timer* pTimer);


#ifdef __cplusplus
}
#endif

#endif

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

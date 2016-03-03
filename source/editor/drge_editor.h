// Public domain. See "unlicense" statement at the end of dr_ge.h.

typedef struct drgui_element drge_subeditor;

typedef struct drge_editor drge_editor;
struct drge_editor
{
    // The context that owns the editor. This is set to the context that was passed to drge_create_editor().
    drge_context* pContext;

    // A pointer to the dr_appkit application object. This is required in order to interface with dr_appkit.
    ak_application* pAKApp;

    // A pointer to the main window.
    ak_window* pMainWindow;

    // The focused tab group.
    drgui_element* pFocusedTabGroup;

    // The command bar.
    drgui_element* pCmdBar;


    //// OPTIONS ////

    // The scale to apply when scrolling text files with the mouse wheel. Can be negative, in which case the scroll
    // direction will be reversed.
    int mouseWheelScale;

    // Whether or not the command bar should be dynamically toggled.
    bool isCmdBarAlwaysShown;
};

// Creates the object representing the editor, but does not start running it.
//
// This does not create any windows and does not begin running the editor. It just allocates memory and initializes
// the internal data structures. Use drge_editor_run() to show the editor and enter into it's main loop.
drge_editor* drge_create_editor(drge_context* pContext);

// Deletes the given editor. Do not call this while the main loop is running. Use drge_editor_close() to
// cleanly exit from the main loop.
void drge_delete_editor(drge_editor* pEditor);


// Begins running the editor. This is where the main application loop is run and will not return until the
// editor has been closed.
//
// Returns the result code which be used as the return value for main().
int drge_editor_run(drge_editor* pEditor);

// Closes the given editor.
void drge_editor_close(drge_editor* pEditor);


// Opens the file at the given path. Returns false if the file could not be opened. If the file is already
// open, the editor will place focus on that file and return true.
bool drge_editor_open_file(drge_editor* pEditor, const char* filePath);

// Saves the focused file.
bool drge_editor_save_focused_file(drge_editor* pEditor);

// Checks if the file at the given path is already open, and if so places focus on it. Returns true if the
// file is already open; false otherwise.
bool drge_editor_try_focus_file_by_path(drge_editor* pEditor, const char* filePath);

// Retireves a pointer to the focused sub-editor.
drge_subeditor* drge_editor_get_focused_subeditor(drge_editor* pEditor);



// Shows the command bar.
void drge_editor_show_command_bar(drge_editor* pEditor);

// Hides the command bar.
void drge_editor_hide_command_bar(drge_editor* pEditor);

// Hides the command bar, but only if auto-hide is enabled. Returns true if the command bar was hidden, false otherwise.
bool drge_editor_try_hide_command_bar(drge_editor* pEditor);

// Determines whether or not the command bar is visible.
bool drge_editor_is_showing_command_bar(drge_editor* pEditor);

// Toggles the command bar.
void drge_editor_toggle_command_bar(drge_editor* pEditor);

// Places keyboard focus on the command bar and shows it.
void drge_editor_focus_command_bar(drge_editor* pEditor);

// Places keyboard focus on the command bar and sets the command text.
void drge_editor_focus_command_bar_and_set_command(drge_editor* pEditor, const char* cmd);


// Updates the status section of the command bar based on the currently focused sub-editor.
void drge_editor_update_status_bar(drge_editor* pEditor);

// Called from the command bar when it receives keyboard focus. This gives the application a chance to show the command bar.
void drge_editor_on_command_bar_capture_keyboard(drge_editor* pEditor, drgui_element* pPrevCapturedElement);

// Called from the command bar when it loses keyboard focus. This gives the application a chance to hide the command bar.
void drge_editor_on_command_bar_release_keyboard(drge_editor* pEditor, drgui_element* pNewCapturedElement);

// Retrieves a pointer to the command line tool.
drgui_element* drge_editor_get_command_bar(drge_editor* pEditor);
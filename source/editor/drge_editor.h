// Public domain. See "unlicense" statement at the end of dr_ge.h.

typedef struct drge_editor drge_editor;
struct drge_editor
{
    // The context that owns the editor. This is set to the context that was passed to drge_create_editor().
    drge_context* pContext;

    // A pointer to the dr_appkit application object. This is required in order to interface with dr_appkit.
    ak_application* pAKApp;

    // A pointer to the main window.
    ak_window* pMainWindow;
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

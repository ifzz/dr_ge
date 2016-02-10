// Public domain. See "unlicense" statement at the end of dr_ge.h.

typedef struct drge_window drge_window;
typedef struct drge_timer  drge_timer;
typedef struct drge_editor drge_editor;

typedef struct drge_context drge_context;
struct drge_context
{
    // The object representing the command line that was used to initialize this context.
    dr_cmdline cmdline;

    // The main game window. dr_ge currently only supports a single game window.
    drge_window* pWindow;

    // We want to use high resolution timing when stepping the game, however this is actually a platform
    // specific thing. We use an abstraction here to keep this file clean of platform-specific code. This
    // is defined in lt_platform_layout.c.
    drge_timer* pTimer;

    // The file system context we'll use for loading all files.
    drvfs_context* pVFS;

    // The log file.
    drvfs_file* pLogFile;


#ifndef DR_GE_DISABLE_EDITOR
    // A pointer to the object representing the editor.
    drge_editor* pEditor;
#endif


    // Keeps track of whether or not we are running portable mode.
    bool isPortable;

    // Whether or not terminal output is disabled.
    bool isTerminalOutputDisabled;

    // Whether or not the context is wanting to close. This is the variable that controls the main game loop.
    bool wantsToClose;


    //// Config ////

    // The game name. This is loaded from the config and used as the window title.
    char name[64];
};


// drge_create_context()
drge_context* drge_create_context(int argc, char** argv);

// drge_create_context_win32()
drge_context* drge_create_context_win32(char* cmdlineStr);

// drge_delete_context()
void drge_delete_context(drge_context* pContext);


// Enters into the main game loop for the given context.
//
// This function checks whether or not the "editor" flag is set on the command line, and if so, enters into
// the editor's application loop via drge_run_editor(). Otherwise it enters into the main game loop via
// drge_run_game().
int drge_run(drge_context* pContext);

// Enters into the main game loop.
int drge_run_game(drge_context* pContext);

#ifndef DR_GE_DISABLE_EDITOR
// Enters into the main application loop for the editor of the given context.
int drge_open_and_run_editor(drge_context* pContext);
#endif

// Determines if the given context is pending closing.
//
// This will return true after a prior call to drge_request_close().
bool drge_wants_to_close(drge_context* pContext);

// Lets the context know that it should close after processing the frame it's currently running.
//
// This is the proper way to escape from the main game loop. Note that this is not exit from the
// loop immediately, but instead waits until the current frame is finished so that it can leave
// the loop cleanly.
void drge_request_close(drge_context* pContext);


// Helper function for creating a context and running the game.
int drge_startup_and_run(int argc, char** argv);

// Same as drge_startup_and_run(), but takes a WinMain style command line.
int drge_startup_and_run_winmain(char* cmdlineStr);


// Updates and renders a single frame.
void drge_do_frame(drge_context* pContext);

// Steps the game by a single frame. This does not render anything.
void drge_step(drge_context* pContext);

// Renders the game based on it's current state.
void drge_render(drge_context* pContext);


// Retrieves a pointer to the object representing the file system of the given context.
drvfs_context* drge_get_vfs(drge_context* pContext);

// Determines whether or not the game is running in portable mode.
bool drge_is_portable(drge_context* pContext);


// Posts a log message.
void drge_log(drge_context* pContext, const char* message);

// Posts a formatted log message.
void drge_logf(drge_context* pContext, const char* format, ...);

// Posts a warning log message.
void drge_warning(drge_context* pContext, const char* message);

// Posts a formatted warning log message.
void drge_warningf(drge_context* pContext, const char* format, ...);

// Posts an error log message.
void drge_error(drge_context* pContext, const char* message);

// Posts a formatted error log message.
void drge_errorf(drge_context* pContext, const char* format, ...);


// Retrieves the path of the directory that contains the log file.
//
// The returned string will be different depending on whether or not DR_GE_PORTABLE is set.
void drge_get_log_file_folder_path(drge_context* pContext, char* pathOut, size_t pathOutSize);

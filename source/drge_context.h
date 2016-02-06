// Public domain. See "unlicense" statement at the end of dr_ge.h.

typedef struct drge_context drge_context;


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

// Enters into the main application loop for the editor of the given context.
int drge_run_editor(drge_context* pContext);


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

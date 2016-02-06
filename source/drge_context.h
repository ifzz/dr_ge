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

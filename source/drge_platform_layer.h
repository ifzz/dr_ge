// Public domain. See "unlicense" statement at the end of dr_ge.h.

typedef struct drge_window drge_window;

// Options for creating windows.
#define DRGE_WINDOW_CENTERED      (1 << 0)
#define DRGE_WINDOW_FULLSCREEN    (1 << 1)


///////////////////////////////////////////////////////////////////////////////
//
// Windows
//
///////////////////////////////////////////////////////////////////////////////

// Initializes the window system.
bool drge_init_window_system();

// Uninitializes the window system.
void drge_uninit_window_system();


// Creates a new game window.
drge_window* drge_create_window(drge_context* pContext, const char* pTitle, unsigned int resolutionX, unsigned int resolutionY, unsigned int options);

// Deletes the given window.
void drge_delete_window(drge_window* pWindow);

// Retrieves a pointer to the dr_ge context that owns the given window.
drge_context* lt_get_window_context(drge_window* pWindow);


#ifdef _WIN32
/// Win32 Only: Retrieves the HWND of the given window.
void* drge_get_window_hwnd(drge_window* pWindow);

/// Win32 Only: Retrieves the HDC of the given window.
void* drge_get_window_hdc(drge_window* pWindow);
#endif


///////////////////////////////////////////////////////////////////////////////
//
// Main Loop
//
///////////////////////////////////////////////////////////////////////////////

/// Enters into the main application window.
int drge_main_loop(drge_context* pGame);



///////////////////////////////////////////////////////////////////////////////
//
// Timers
//
///////////////////////////////////////////////////////////////////////////////

typedef struct drge_timer drge_timer;

/// Creates a timer object.
drge_timer* drge_create_timer();

/// Deletes the given timer.
void drge_delete_timer(drge_timer* pTimer);

/// "Ticks" the timer, and returns the time since the last tick, in seconds.
double drge_tick_timer(drge_timer* pTimer);
// Public domain. See "unlicense" statement at the end of dr_ge.h.

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
};

static drge_context* drge_create_context_cmdline(dr_cmdline cmdline)
{
    drge_init_window_system();

    drge_context* pContext = malloc(sizeof(*pContext));
    if (pContext == NULL) {
        return NULL;
    }

    pContext->cmdline = cmdline;
    pContext->pWindow = NULL;
    pContext->pTimer  = NULL;



    return pContext;
}

drge_context* drge_create_context(int argc, char** argv)
{
    dr_cmdline cmdline;
    dr_init_cmdline(&cmdline, argc, argv);          // Won't fail in practice.

    return drge_create_context_cmdline(cmdline);
}

drge_context* drge_create_context_win32(char* cmdlineStr)
{
    dr_cmdline cmdline;
    dr_init_cmdline_win32(&cmdline, cmdlineStr);    // Won't fail in practice.

    return drge_create_context_cmdline(cmdline);
}

void drge_delete_context(drge_context* pContext)
{
    drge_uninit_window_system();

    free(pContext);
}


int drge_run(drge_context* pContext)
{
    if (pContext == NULL) {
        return -1;  // No context.
    }

    if (dr_cmdline_key_exists(&pContext->cmdline, "editor")) {
        return drge_run_editor(pContext);
    } else {
        return drge_run_game(pContext);
    }
}

int drge_run_game(drge_context* pContext)
{
    if (pContext == NULL) {
        return -1;
    }

    pContext->pWindow = drge_create_window(pContext, "dr_ge", 640, 480, DRGE_WINDOW_CENTERED);
    if (pContext->pWindow == NULL) {
        return -2;  // Failed to create the game window.
    }

    pContext->pTimer = drge_create_timer();
    if (pContext->pTimer == NULL) {
        return -3;  // Failed to create the timer.
    }

    int result = drge_main_loop(pContext);

    drge_delete_timer(pContext->pTimer);
    drge_delete_window(pContext->pWindow);
    return result;
}

int drge_run_editor(drge_context* pContext)
{
    if (pContext == NULL) {
        return -1;
    }

    // TODO: Open and run the editor.

    return 0;
}


static int drge_startup_and_run_cmdline(dr_cmdline cmdline)
{
    drge_context* pContext = drge_create_context_cmdline(cmdline);
    if (pContext == NULL) {
        return -1;  // Failed to create the main context.
    }

    int result = drge_run(pContext);

    drge_delete_context(pContext);
    return result;
}

int drge_startup_and_run(int argc, char** argv)
{
    dr_cmdline cmdline;
    dr_init_cmdline(&cmdline, argc, argv);          // Won't fail in practice.

    return drge_startup_and_run_cmdline(cmdline);
}

int drge_startup_and_run_winmain(char* cmdlineStr)
{
    dr_cmdline cmdline;
    dr_init_cmdline_win32(&cmdline, cmdlineStr);    // Won't fail in practice.

    return drge_startup_and_run_cmdline(cmdline);
}


void drge_do_frame(drge_context* pContext)
{
    if (pContext == NULL) {
        return;
    }

    // This function is called by gtge_main_loop() whenever the game needs to be stepped and rendered. We are keeping it simple and doing
    // everything in a single thread. We step the game first, and then render.
    drge_step(pContext);
    drge_render(pContext);
}

void drge_step(drge_context* pContext)
{
    if (pContext == NULL) {
        return;
    }

    double dtSeconds = drge_tick_timer(pContext->pTimer);
}

void drge_render(drge_context* pContext)
{
    if (pContext == NULL) {
        return;
    }
}

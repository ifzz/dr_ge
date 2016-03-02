// Public domain. See "unlicense" statement at the end of dr_ge.h.

#ifdef _WIN32
#include <windows.h>

static const char* g_LTWndClassName = "LTMainWindow";

struct drge_window
{
    // A pointer to the game object that owns the window.
    drge_context* pContext;

    // A handle to the Win32 window.
    HWND hWnd;
};

// This is the event handler for the main game window. The operating system will notify the application of events
// through this function, such as when the mouse is moved over a window, a key is pressed, etc.
static LRESULT DefaultWindowProcWin32(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    drge_window* pWindow = (drge_window*)GetWindowLongPtrA(hWnd, 0);
    if (pWindow != NULL)
    {
        switch (msg)
        {
            case WM_CLOSE:
            {
                PostQuitMessage(0);
                return 0;
            }

            default: break;
        }
    }

    return DefWindowProcA(hWnd, msg, wParam, lParam);
}



bool drge_init_window_system()
{
    // The Windows operating system likes to automatically change the size of the game window when DPI scaling is
    // used. For example, if the user has their DPI set to 200%, the operating system will try to be helpful and
    // automatically resize every window by 200%. The size of the window controls the resolution the game runs at,
    // but we want that resolution to be set explicitly via something like an options menu. Thus, we don't want the
    // operating system to be changing the size of the window to anything other than what we explicitly request. To
    // do this, we just tell the operation system that it shouldn't do DPI scaling and that we'll do it ourselves
    // manually.
    dr_win32_make_dpi_aware();


    // We now need to register the window class that we'll use for the main game window. This needs to be done
    // before any window is created with drge_create_window().
    WNDCLASSEXA wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.cbSize        = sizeof(wc);
    wc.cbWndExtra    = sizeof(void*);
    wc.lpfnWndProc   = (WNDPROC)DefaultWindowProcWin32;
    wc.lpszClassName = g_LTWndClassName;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.style         = CS_OWNDC | CS_DBLCLKS;
    if (!RegisterClassExA(&wc)) {
        return false;
    }

    return true;
}

void drge_uninit_window_system()
{
    UnregisterClassA(g_LTWndClassName, GetModuleHandleA(NULL));
}


drge_window* drge_create_window(drge_context* pContext, const char* pTitle, unsigned int resolutionX, unsigned int resolutionY, unsigned int options)
{
    // This assumes the window class has already been registered by drge_init_window_system().
    HWND hWnd = CreateWindowExA(0, g_LTWndClassName, pTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, resolutionX, resolutionY, NULL, NULL, NULL, NULL);
    if (hWnd == NULL) {
        return NULL;
    }

    // We should have a window, but before showing it we need to make a few small adjustments to the position and size. First, we need to
    // honour the LT_WINDOW_CENTERED option. Second, we need to make a small change to the size of the window such that the client size
    // is equal to resolutionX and resolutionY. When we created the window, we specified resolutionX and resolutionY as the dimensions,
    // however this includes the size of the outer border. The outer border should not be included, so we need to stretch the window just
    // a little bit such that the area inside the borders are exactly equal to resolutionX and resolutionY.
    //
    // We use a single API to both move and resize the window.

    UINT swpflags = SWP_NOZORDER | SWP_NOMOVE;

    RECT windowRect;
    RECT clientRect;
    GetWindowRect(hWnd, &windowRect);
    GetClientRect(hWnd, &clientRect);

    int windowWidth  = (int)resolutionX + ((windowRect.right - windowRect.left) - (clientRect.right - clientRect.left));
    int windowHeight = (int)resolutionY + ((windowRect.bottom - windowRect.top) - (clientRect.bottom - clientRect.top));

    int windowPosX = 0;
    int windowPosY = 0;
    if ((options & DRGE_WINDOW_CENTERED) != 0)
    {
        // We need to center the window. To do this properly, we want to reposition based on the monitor it started on.
        MONITORINFO mi;
        ZeroMemory(&mi, sizeof(mi));
        mi.cbSize = sizeof(MONITORINFO);
        if (GetMonitorInfoA(MonitorFromWindow(hWnd, 0), &mi))
        {
            windowPosX = ((mi.rcMonitor.right - mi.rcMonitor.left) - windowWidth)  / 2;
            windowPosY = ((mi.rcMonitor.bottom - mi.rcMonitor.top) - windowHeight) / 2;

            swpflags &= ~SWP_NOMOVE;
        }
    }

    SetWindowPos(hWnd, NULL, windowPosX, windowPosY, windowWidth, windowHeight, swpflags);


    // At this point the window should be in the correct position and at the correct size. It should now be safe to create the window
    // object and show the window.
    drge_window* pWindow = malloc(sizeof(*pWindow));
    if (pWindow == NULL) {
        DestroyWindow(hWnd);
        return NULL;
    }

    pWindow->pContext = pContext;
    pWindow->hWnd     = hWnd;


    // The window has been position and sized correctly, but now we need to attach our own window object to the internal Win32 object.
    SetWindowLongPtrA(hWnd, 0, (LONG_PTR)pWindow);


    // Finally we can show our window.
    ShowWindow(hWnd, SW_SHOWNORMAL);

    return pWindow;
}

void drge_delete_window(drge_window* pWindow)
{
    if (pWindow == NULL) {
        return;
    }

    DestroyWindow(pWindow->hWnd);
    free(pWindow);
}


drge_context* drge_get_window_context(drge_window* pWindow)
{
    if (pWindow == NULL) {
        return NULL;
    }

    return pWindow->pContext;
}


void* drge_get_window_hwnd(drge_window* pWindow)
{
    return pWindow->hWnd;
}

void* drge_get_window_hdc(drge_window* pWindow)
{
    return GetDC(pWindow->hWnd);
}



int drge_main_loop(drge_context* pContext)
{
    while (!drge_wants_to_close(pContext))
    {
        // Handle window events.
        MSG msg;
        if (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT) {
                return (int)msg.wParam;  // Received a quit message.
            }

            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }

        // After handling the next event in the queue we let the game know it should do the next frame.
        drge_do_frame(pContext);
    }

    return 0;
}



struct drge_timer
{
    /// The high performance counter frequency.
    LARGE_INTEGER frequency;

    /// The timer counter at the point in the time the timer was last ticked.
    LARGE_INTEGER counter;
};

drge_timer* drge_create_timer()
{
    drge_timer* pTimer = malloc(sizeof(*pTimer));
    if (pTimer == NULL) {
        return NULL;
    }

    if (!QueryPerformanceFrequency(&pTimer->frequency) || !QueryPerformanceCounter(&pTimer->counter)) {
        free(pTimer);
        return NULL;
    }

    return pTimer;
}

void drge_delete_timer(drge_timer* pTimer)
{
    free(pTimer);
}

double drge_tick_timer(drge_timer* pTimer)
{
    LARGE_INTEGER oldCounter = pTimer->counter;
    if (!QueryPerformanceCounter(&pTimer->counter)) {
        return 0;
    }

    return (pTimer->counter.QuadPart - oldCounter.QuadPart) / (double)pTimer->frequency.QuadPart;
}
#endif

#ifndef _WIN32
#include <X11/Xlib.h>

struct drge_window
{
    // A pointer to the game object that owns the window.
    drge_context* pContext;

    // A handle to the X11 window.
    Window x11Window;
};



// The reference count for how many times drge_init_window_system() has been called.
int g_X11InitCounter = 0;

// The global display.
Display* g_X11Display = NULL;

// The atom for the WM_DELETE_WINDOW event so we can cleanly handle the close button on the window.
Atom g_WM_DELETE_WINDOW = 0;


bool drge_init_window_system()
{
    // Check if it's already initialized.
    if (g_X11Display != NULL) {
        g_X11InitCounter += 1;
        return true;
    }

    g_X11Display = XOpenDisplay(NULL);
    if (g_X11Display == NULL) {
        return false;
    }

    g_WM_DELETE_WINDOW = XInternAtom(g_X11Display, "WM_DELETE_WINDOW", False);

    g_X11InitCounter = 1;
    return true;
}

void drge_uninit_window_system()
{
    if (g_X11Display == NULL) {
        return;
    }

    if (g_X11InitCounter > 0) {
        g_X11InitCounter -= 1;
    } else {
        assert(false);  // If you have triggered this assert it means you have not correctly matched your drge_init_window_system() and drge_uninit_window_system() calls.
    }

    if (g_X11InitCounter == 0)
    {
        XCloseDisplay(g_X11Display);
        g_X11Display = NULL;
    }
}


drge_window* drge_create_window(drge_context* pContext, const char* pTitle, unsigned int resolutionX, unsigned int resolutionY, unsigned int options)
{
    XSetWindowAttributes wa;
    wa.colormap          = CopyFromParent;
    wa.border_pixel      = 0;
    wa.event_mask        = StructureNotifyMask | SubstructureNotifyMask | ExposureMask | ButtonPressMask | ButtonReleaseMask | KeyPressMask | KeyReleaseMask | PointerMotionMask | ButtonMotionMask | FocusChangeMask | PropertyChangeMask;
    wa.overrdrge_editor_redirect = false;

    Window x11Window = XCreateWindow(g_X11Display, RootWindow(g_X11Display, DefaultScreen(g_X11Display)), 0, 0, resolutionX, resolutionY, 0, CopyFromParent, InputOutput, CopyFromParent, CWBorderPixel | CWColormap | CWEventMask | CWOverrideRedirect, &wa);
    if (x11Window == 0) {
        return NULL;
    }

    // So we can handle the close button properly...
    XSetWMProtocols(g_X11Display, x11Window, &g_WM_DELETE_WINDOW, 1);

    // Set the title.
    XStoreName(g_X11Display, x11Window, pTitle);

    // TODO: Handle options.
    (void)options;

    // Show the window.
    XMapWindow(g_X11Display, x11Window);



    // At this point the window should be in the correct position and at the correct size. It should now be safe to create the window
    // object and show the window.
    drge_window* pWindow = malloc(sizeof(*pWindow));
    if (pWindow == NULL) {
        return NULL;
    }

    pWindow->pContext  = pContext;
    pWindow->x11Window = x11Window;

    return pWindow;
}

void drge_delete_window(drge_window* pWindow)
{
    if (pWindow == NULL) {
        return;
    }

    XDestroyWindow(g_X11Display, pWindow->x11Window);
    free(pWindow);
}


drge_context* drge_get_window_context(drge_window* pWindow)
{
    if (pWindow == NULL) {
        return NULL;
    }

    return pWindow->pContext;
}



int drge_main_loop(drge_context* pContext)
{
    while (!drge_wants_to_close(pContext))
    {
        // Handle window events.
        int eventCount = XPending(g_X11Display);
        if (eventCount > 0)
        {
            XEvent e;
            XNextEvent(g_X11Display, &e);

            switch (e.type)
            {
                case ClientMessage:
                {
                    if (e.xclient.data.l[0] == (long)g_WM_DELETE_WINDOW) {
                        drge_request_close(pContext);
                    }
                } break;
            }
        }

        // After handling the next event in the queue we let the game know it should do the next frame.
        drge_do_frame(pContext);
    }

    return 0;
}



struct drge_timer
{
    long long unused;
};

drge_timer* drge_create_timer()
{
    drge_timer* pTimer = malloc(sizeof(*pTimer));
    if (pTimer == NULL) {
        return NULL;
    }

    pTimer->unused = 0;

    return pTimer;
}

void drge_delete_timer(drge_timer* pTimer)
{
    free(pTimer);
}

double drge_tick_timer(drge_timer* pTimer)
{
    if (pTimer == NULL) {
        return 0;
    }

    return 0;
}
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

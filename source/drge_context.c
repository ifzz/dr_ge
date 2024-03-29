// Public domain. See "unlicense" statement at the end of dr_ge.h.

static void drge_load_default_config(drge_context* pContext)
{
    if (pContext == NULL) {
        return;
    }

    strcpy_s(pContext->name, sizeof(pContext->name), "My Game");
}

typedef struct
{
    drge_context* pContext;
    drfs_file* pConfigFile;
} drge_load_config_data;

static size_t drge_load_config_read(void* pUserData, void* pDataOut, size_t bytesToRead)
{
    drge_load_config_data* pData = pUserData;
    assert(pData != NULL);

    size_t bytesRead;
    drfs_read(pData->pConfigFile, pDataOut, bytesToRead, &bytesRead);

    return bytesRead;
}

static void drge_load_config_pair(void* pUserData, const char* key, const char* value)
{
    drge_load_config_data* pData = pUserData;
    assert(pData != NULL);

    drge_context* pContext = pData->pContext;

    if (strcmp(key, "Name") == 0) {
        strncpy_s(pContext->name, sizeof(pContext->name), value, _TRUNCATE);
        return;
    }

    if (strcmp(key, "BaseDirectory") == 0)
    {
        // Base directories are listed in order of priority, but important to remember that the executable's directory must always
        // be the lowest priority base directory. Therefore we always add the base directory to the second last position.
        drfs_context* pVFS = drge_get_vfs(pContext);
        assert(drfs_get_base_directory_count(pVFS) > 0);

        char absolutePath[DRFS_MAX_PATH];
        drpath_to_absolute(value, drfs_get_base_directory_by_index(pVFS, drfs_get_base_directory_count(pVFS) - 1), absolutePath, sizeof(absolutePath));

        drfs_insert_base_directory(pVFS, absolutePath, drfs_get_base_directory_count(pVFS) - 1);
        return;
    }
}

static void drge_load_config_error(void* pUserData, const char* message, unsigned int line)
{
    drge_load_config_data* pData = pUserData;
    assert(pData != NULL);

    drge_errorf(pData->pContext, "[CONFIG]:%d %s", line, message);
}

static void drge_load_config(drge_context* pContext)
{
    if (pContext == NULL) {
        return;
    }

    // Load the default config first so we have default values for those that aren't specified in the config file.
    drge_load_default_config(pContext);

    drfs_file* pConfigFile;
    if (drfs_open(drge_get_vfs(pContext), "config.cfg", DRFS_READ, &pConfigFile) != drfs_success) {
        return;
    }

    drge_load_config_data data;
    data.pContext = pContext;
    data.pConfigFile = pConfigFile;
    dr_parse_key_value_pairs(drge_load_config_read, drge_load_config_pair, drge_load_config_error, &data);

    drfs_close(pConfigFile);
}

static void drge_open_log_file(drge_context* pContext)
{
    if (pContext == NULL || pContext->pVFS == NULL) {
        return;
    }

    if (pContext->pLogFile != NULL) {
        return; // Already open.
    }

    char logPath[DRFS_MAX_PATH];
    drge_get_log_file_folder_path(pContext, logPath, sizeof(logPath));
    drpath_append(logPath, sizeof(logPath), "dr_log.log");

    pContext->pLogFile;
    drfs_open(drge_get_vfs(pContext), logPath, DRFS_WRITE | DRFS_TRUNCATE | DRFS_CREATE_DIRS, &pContext->pLogFile);
}

static drge_context* drge_create_context_cmdline(dr_cmdline cmdline)
{
    drge_init_window_system();

    drge_context* pContext = malloc(sizeof(*pContext));
    if (pContext == NULL) {
        return NULL;
    }

    memset(pContext, 0, sizeof(*pContext));
    pContext->cmdline  = cmdline;


#ifdef DR_GE_PORTABLE
    pContext->isPortable = true;
#else
    pContext->isPortable = dr_cmdline_key_exists(&cmdline, "portable");
#endif
    pContext->isTerminalOutputDisabled = dr_cmdline_key_exists(&cmdline, "silent");
    pContext->wantsToClose = false;


    // The file system. The lowest priority base path is always the directory containing the executable.
    pContext->pVFS = drfs_create_context();
    if (pContext->pVFS == NULL) {
        goto on_error;
    }

    char executableDirPath[DRFS_MAX_PATH];
    if (!dr_get_executable_path(executableDirPath, sizeof(executableDirPath))) {
        // This is actually a critical error because we need a reliable relative path to load assets and whatnot.
        goto on_error;
    }

    drfs_add_base_directory(pContext->pVFS, drpath_base_path(executableDirPath));


    // The config file. This needs to be done after initializing the file system so we can load the file. Needs to come
    // before loading the config file because the config contains the game name which we need for determining where to
    // place the log file.
    drge_load_config(pContext);

    // The log file. Always do this after loading the config.
    drge_open_log_file(pContext);


    // Graphics.
    pContext->pVulkan = drvkCreateContext(NULL);
    pContext->pGraphicsWorld = drge_create_graphics_world(pContext->pVulkan);

    return pContext;


on_error:
    drvkDeleteContext(pContext->pVulkan);

    if (pContext->pLogFile) {
        drfs_close(pContext->pLogFile);
    }

    if (pContext->pVFS) {
        drfs_delete_context(pContext->pVFS);
    }

    free(pContext);
    return NULL;
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
    if (pContext == NULL) {
        return;
    }

    drfs_close(pContext->pLogFile);
    drfs_delete_context(pContext->pVFS);
    free(pContext);

    drge_uninit_window_system();
}


int drge_run(drge_context* pContext)
{
    if (pContext == NULL) {
        return -1;  // No context.
    }

#ifndef DR_GE_DISABLE_EDITOR
    if (dr_cmdline_key_exists(&pContext->cmdline, "editor"))
    {
        return drge_open_and_run_editor(pContext);
    }
    else
#endif
    {
        return drge_run_game(pContext);
    }
}

int drge_run_game(drge_context* pContext)
{
    if (pContext == NULL) {
        return -1;
    }

    pContext->pWindow = drge_create_window(pContext, pContext->name, 640, 480, DRGE_WINDOW_CENTERED);
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

#ifndef DR_GE_DISABLE_EDITOR
int drge_open_and_run_editor(drge_context* pContext)
{
    if (pContext == NULL || pContext->pEditor != NULL) {
        return -1;
    }

    // The editor should not already be open.
    if (pContext->pEditor != NULL) {
        return -2;
    }

    pContext->pEditor = drge_create_editor(pContext);
    if (pContext->pEditor == NULL) {
        return -3;
    }

    int result = drge_editor_run(pContext->pEditor);

    drge_delete_editor(pContext->pEditor);
    pContext->pEditor = NULL;

    return result;
}
#endif


bool drge_wants_to_close(drge_context* pContext)
{
    if (pContext == NULL) {
        return false;
    }

    return pContext->wantsToClose;
}

void drge_request_close(drge_context* pContext)
{
    if (pContext == NULL) {
        return;
    }

    pContext->wantsToClose = true;
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

    //double dtSeconds = drge_tick_timer(pContext->pTimer);
}

void drge_render(drge_context* pContext)
{
    if (pContext == NULL) {
        return;
    }
}


drfs_context* drge_get_vfs(drge_context* pContext)
{
    if (pContext == NULL) {
        return NULL;
    }

    return pContext->pVFS;
}

bool drge_is_portable(drge_context* pContext)
{
    if (pContext == NULL) {
        return false;
    }

    return pContext->isPortable;
}


void drge_log(drge_context* pContext, const char* message)
{
    if (pContext == NULL || message == NULL) {
        return;
    }

    // Log file.
    if (pContext->pLogFile != NULL)
    {
        char dateTime[64];
        dr_datetime_short(dr_now(), dateTime, sizeof(dateTime));

        drfs_write_string(pContext->pLogFile, "[");
        drfs_write_string(pContext->pLogFile, dateTime);
        drfs_write_string(pContext->pLogFile, "]");
        drfs_write_line  (pContext->pLogFile, message);
        drfs_flush(pContext->pLogFile);
    }


    // Terminal.
    if (!pContext->isTerminalOutputDisabled) {
        printf("%s\n", message);
    }
}

void drge_logf(drge_context* pContext, const char* format, ...)
{
    if (pContext == NULL || format == NULL) {
        return;
    }

    va_list args;
    va_start(args, format);
    {
        char msg[4096];
        vsnprintf(msg, sizeof(msg), format, args);

        drge_log(pContext, msg);
    }
    va_end(args);
}

void drge_warning(drge_context* pContext, const char* message)
{
    drge_logf(pContext, "[WARNING] %s", message);
}

void drge_warningf(drge_context* pContext, const char* format, ...)
{
    if (pContext == NULL || format == NULL) {
        return;
    }

    va_list args;
    va_start(args, format);
    {
        char msg[4096];
        vsnprintf(msg, sizeof(msg), format, args);

        drge_warning(pContext, msg);
    }
    va_end(args);
}

void drge_error(drge_context* pContext, const char* message)
{
    drge_logf(pContext, "[ERROR] %s", message);
}

void drge_errorf(drge_context* pContext, const char* format, ...)
{
    if (pContext == NULL || format == NULL) {
        return;
    }

    va_list args;
    va_start(args, format);
    {
        char msg[4096];
        vsnprintf(msg, sizeof(msg), format, args);

        drge_error(pContext, msg);
    }
    va_end(args);
}


void drge_get_log_file_folder_path(drge_context* pContext, char* pathOut, size_t pathOutSize)
{
    if (pContext == NULL || pathOut == NULL || pathOutSize == 0) {
        return;
    }

    if (pContext->isPortable)
    {
        dr_get_executable_path(pathOut, pathOutSize);
        drpath_base_path(pathOut);
        drpath_append(pathOut, pathOutSize, "var/log");
    }
    else
    {
        char logFolderPath[DRFS_MAX_PATH];
        if (dr_get_log_folder_path(logFolderPath, sizeof(logFolderPath)))
        {
            // We found the log folder path.

            // If the log folder path is relative, assume it's relative to the executable.
            if (drpath_is_relative(logFolderPath)) {
                dr_get_executable_path(pathOut, pathOutSize);
                drpath_base_path(pathOut);
                drpath_append(pathOut, pathOutSize, logFolderPath);
            } else {
                strcpy_s(pathOut, pathOutSize, logFolderPath);
            }

            // The folder path needs to be namespaced based on the game name.
            drpath_append(pathOut, pathOutSize, pContext->name);
        }
        else
        {
            // We failed to retrieve the per-user log folder path so we'll just fall back to the executable's directory.
            dr_get_executable_path(pathOut, pathOutSize);
            drpath_base_path(pathOut);
            drpath_append(pathOut, pathOutSize, "var/log");
        }
    }
}

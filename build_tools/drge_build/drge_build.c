
#define DR_UTIL_IMPLEMENTATION
#include "../../source/external/dr_util.h"

#define DR_PATH_IMPLEMENTATION
#include "../../source/external/dr_path.h"

#define DR_VFS_IMPLEMENTATION
#include "../../source/external/dr_vfs.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#else
#include <unistd.h>
#endif

#define ERROR_NONE              0
#define ERROR_UNKNOWN_COMPILER  1
#define ERROR_UNKNOWN_TARGET    2

typedef enum
{
    drge_compiler_none,
    drge_compiler_gcc,
    drge_compiler_clang,
    drge_compiler_msvc
} drge_compiler;

typedef enum
{
    drge_target_none,
    drge_target_debug,
    drge_target_release
} drge_target;

typedef struct
{
    /// Whether or not the pre-build step should be ignored.
    bool noPrebuild;

    /// Whether or not the post-build step should be ignored.
    bool noPostbuild;

    /// Whether or not the build should be performed.
    bool doBuild;

    /// The compiler.
    drge_compiler compiler;

    /// The build target.
    drge_target target;

    /// The virtual file system we'll be using to open and copy files. There is only a single base directory added to this context
    /// which is the directory that contains this executable.
    drvfs_context* pVFS;


} drge_build_context;


void print_help()
{
    printf("  -h, --help                  Display this information\n");
    printf("  --prebuild                  Perform ONLY the pre-build step\n");
    printf("  --postbuild                 Perform ONLY the post-build step\n");
    printf("  --noprebuild                Skip the pre-build step\n");
    printf("  --nopostbuild               Skip the post-build step\n");
    printf("  --compiler {gcc|clang|msvc} Specify the compiler to use when compiling\n");
    printf("  --target {debug|release}    Specify the build target\n");
}


void copy_and_log_file(drge_build_context* pContext, const char* srcPath, const char* dstPath)
{
    assert(pContext != NULL);
    assert(srcPath != NULL);
    assert(dstPath != NULL);

    // Make sure the directory exists.
    char dstPathDir[DRVFS_MAX_PATH];
    drpath_copy_base_path(dstPath, dstPathDir, sizeof(dstPathDir));
    drvfs_create_directory(pContext->pVFS, dstPathDir);

    if (drvfs_copy_file(pContext->pVFS, srcPath, dstPath, false)) {
        printf("  Copied file %s to %s\n", srcPath, dstPath);
    } else {
        printf("  ERROR: Failed to copy %s to %s\n", srcPath, dstPath);
    }
}


void prebuild(drge_build_context* pContext)
{
    assert(pContext != NULL);

    printf("--- Performing Pre-Build Step ---\n");
    printf("1) Adding external dependencies to source tree...\n");
    {
        copy_and_log_file(pContext, "../../dr_libs/dr_audio.h", "../source/external/dr_audio.h");
        copy_and_log_file(pContext, "../../dr_libs/dr_wav.h", "../source/external/dr_wav.h");
        copy_and_log_file(pContext, "../../dr_libs/dr_flac.h", "../source/external/dr_flac.h");
        copy_and_log_file(pContext, "../../dr_libs/dr_2d.h", "../source/external/dr_2d.h");
        copy_and_log_file(pContext, "../../dr_libs/dr_mtl.h", "../source/external/dr_mtl.h");
        copy_and_log_file(pContext, "../../dr_libs/dr_fsw.h", "../source/external/dr_fsw.h");
        copy_and_log_file(pContext, "../../dr_libs/dr_vfs.h", "../source/external/dr_vfs.h");
        copy_and_log_file(pContext, "../../dr_libs/dr_gui.h", "../source/external/dr_gui.h");
        copy_and_log_file(pContext, "../../dr_libs/dr_math.h", "../source/external/dr_math.h");
        copy_and_log_file(pContext, "../../dr_libs/dr_path.h", "../source/external/dr_path.h");
        copy_and_log_file(pContext, "../../dr_libs/dr_util.h", "../source/external/dr_util.h");
        copy_and_log_file(pContext, "../../dr_libs/dr_vulkan.h", "../source/external/dr_vulkan.h");

        // dr_appkit.
        copy_and_log_file(pContext, "../../dr_appkit/dr_appkit.h", "../source/external/dr_appkit/dr_appkit.h");
        copy_and_log_file(pContext, "../../dr_appkit/dr_appkit.c", "../source/external/dr_appkit/dr_appkit.c");

        drvfs_iterator i;
        if (drvfs_begin(pContext->pVFS, "../../dr_appkit/source", &i))
        {
            do
            {
                char relativePathSrc[DRVFS_MAX_PATH];
                char relativePathDst[DRVFS_MAX_PATH];
                drpath_copy_and_append(relativePathSrc, sizeof(relativePathSrc), "../../dr_appkit/source", drpath_file_name(i.info.absolutePath));
                drpath_copy_and_append(relativePathDst, sizeof(relativePathDst), "../source/external/dr_appkit/source", drpath_file_name(i.info.absolutePath));

                copy_and_log_file(pContext, relativePathSrc, relativePathDst);
            } while (drvfs_next(pContext->pVFS, &i));
        }

        // dr_gui WIP.
        copy_and_log_file(pContext, "../../dr_libs/wip/dr_gui_scrollbar.h", "../source/external/wip/dr_gui_scrollbar.h");
        copy_and_log_file(pContext, "../../dr_libs/wip/dr_gui_tab_bar.h", "../source/external/wip/dr_gui_tab_bar.h");
        copy_and_log_file(pContext, "../../dr_libs/wip/dr_gui_text_layout.h", "../source/external/wip/dr_gui_text_layout.h");
        copy_and_log_file(pContext, "../../dr_libs/wip/dr_gui_textbox.h", "../source/external/wip/dr_gui_textbox.h");
        copy_and_log_file(pContext, "../../dr_libs/wip/dr_gui_tree_view.h", "../source/external/wip/dr_gui_tree_view.h");
    }

    printf("--- Pre-Build Step Complete ---\n");
}

void postbuild(drge_build_context* pContext)
{
    assert(pContext != NULL);

    printf("--- Performing Post-Build Step ---\n");
    printf("--- Post-Build Step Complete ---\n");
}


int build_gcc(drge_build_context* pContext)
{
    assert(pContext != NULL);
    return ERROR_NONE;
}

int build_clang(drge_build_context* pContext)
{
    assert(pContext != NULL);
    return ERROR_NONE;
}

int build_msvc(drge_build_context* pContext)
{
    assert(pContext != NULL);
    return ERROR_NONE;
}

int build(drge_build_context* pContext)
{
    assert(pContext != NULL);

    // Pre-build.
    if (!pContext->noPrebuild) {
        prebuild(pContext);
    }


    // Build
    switch (pContext->compiler)
    {
    case drge_compiler_gcc:   return build_gcc(pContext);
    case drge_compiler_clang: return build_clang(pContext);
    case drge_compiler_msvc:  return build_msvc(pContext);
    default: return ERROR_UNKNOWN_COMPILER;
    }

    
    // Post-build.
    if (!pContext->noPostbuild) {
        postbuild(pContext);
    }
}


bool on_cmdline(const char* key, const char* value, void* pUserData)
{
    drge_build_context* pContext = pUserData;
    assert(pContext       != NULL);
    assert(pContext->pVFS != NULL);

    if (strcmp(key, "[path]") == 0)
    {
        char basedir[DRVFS_MAX_PATH];
        drpath_copy_base_path(value, basedir, sizeof(basedir));

        if (drpath_is_relative(basedir))
        {
            char cwd[DRVFS_MAX_PATH];
#ifdef _WIN32
            if (_getcwd(cwd, sizeof(cwd)) == NULL)
#else
            if (getcwd(cwd, sizeof(cwd)) == NULL)
#endif
            {
                // Should never get here.
                printf("Error: Unable to get current working directory.\n");
                return false;
            }

            drpath_append(cwd, sizeof(cwd), basedir);
            dr_strcpy_s(basedir, sizeof(basedir), cwd);
        }


        // The base directory should be cleaned before adding it to the virtual file system, if only for sanity.
        char basedirClean[DRVFS_MAX_PATH];
        drpath_clean(basedir, basedirClean, sizeof(basedirClean));

        // We should have a valid base directory.
        drvfs_add_base_directory(pContext->pVFS, basedirClean);
        drvfs_set_base_write_directory(pContext->pVFS, basedirClean);

        printf("Working Directory: %s\n\n", basedirClean);
    }


    if (strcmp(key, "h") == 0 || strcmp(key, "help") == 0)
    {
        print_help();
        pContext->doBuild = false;
        return false;
    }


    if (strcmp(key, "prebuild") == 0)
    {
        prebuild(pContext);
        pContext->doBuild = false;
        return false;
    }

    if (strcmp(key, "postbuild") == 0)
    {
        postbuild(pContext);
        pContext->doBuild = false;
        return false;
    }



    if (strcmp(key, "noprebuild") == 0)
    {
        pContext->noPrebuild = true;
        return true;
    }

    if (strcmp(key, "nopostbuild") == 0)
    {
        pContext->noPostbuild = true;
        return true;
    }



    if (strcmp(key, "compiler") == 0)
    {
        if (strcmp(value, "gcc") == 0) {
            pContext->compiler = drge_compiler_gcc;
            return true;
        }
        if (strcmp(value, "clang") == 0) {
            pContext->compiler = drge_compiler_clang;
            return true;
        }
        if (strcmp(value, "msvc") == 0) {
            pContext->compiler = drge_compiler_msvc;
            return true;
        }

        printf("Error: Unknown or unsupported compiler: %s\n", value);
    }

    if (strcmp(key, "target") == 0)
    {
        if (strcmp(value, "debug") == 0) {
            pContext->target = drge_target_debug;
            return true;
        }
        if (strcmp(value, "release") == 0) {
            pContext->target = drge_target_release;
            return true;
        }

        printf("Error: Unknown or unsupported target: %s\n", value);
    }


    return true;
}

int main(int argc, char** argv)
{
    int result = ERROR_NONE;

    dr_cmdline cmdline;
    dr_init_cmdline(&cmdline, argc, argv);

    drge_build_context context;
    context.noPrebuild   = false;
    context.noPostbuild  = false;
    context.doBuild      = true;
    context.compiler     = drge_compiler_none;
    context.target       = drge_target_none;
    context.pVFS         = drvfs_create_context();
    dr_parse_cmdline(&cmdline, on_cmdline, &context);


    // If we did the pre- or post-build (--prebuild or --postbuild), don't continue with the rest of the build.
    if (context.doBuild)
    {
        // At this point we now want to continue with a build.
        if (result == ERROR_NONE && context.compiler == drge_compiler_none) {
            printf("Error: No compiler specified. Use --compiler {gcc|clang|msvc} to specify a compiler.\n");
            result = ERROR_UNKNOWN_COMPILER;
        }

        if (result == ERROR_NONE && context.target == drge_target_none) {
            printf("Error: No target specified. Use --target {debug|release} to specify a target.\n");
            result = ERROR_UNKNOWN_TARGET;
        }


        // We should have a valid compiler and target. For now we are going to do a full rebuild, but later on we might do
        // partial builds.
        if (result == ERROR_NONE) {
            result = build(&context);
        }
    }


    // Shutdown.
    drvfs_delete_context(context.pVFS);

    return result;
}
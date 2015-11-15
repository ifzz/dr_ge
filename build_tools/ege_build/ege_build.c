#include "../../include/external/easy_util/easy_util.h"
#include "../../include/external/easy_path/easy_path.h"
#include "../../include/external/easy_fs/easy_vfs.h"
#include "../../include/external/gb_string.h"
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
    ege_compiler_none,
    ege_compiler_gcc,
    ege_compiler_clang,
    ege_compiler_msvc
} ege_compiler;

typedef enum
{
    ege_target_none,
    ege_target_debug,
    ege_target_release
} ege_target;

typedef struct
{
    /// Whether or not the pre-build step should be ignored.
    bool noPrebuild;

    /// Whether or not the post-build step should be ignored.
    bool noPostbuild;

    /// Whether or not the build should be performed.
    bool doBuild;

    /// The compiler.
    ege_compiler compiler;

    /// The build target.
    ege_target target;

    /// The virtual file system we'll be using to open and copy files. There is only a single base directory added to this context
    /// which is the directory that contains this executable.
    easyvfs_context* pVFS;


} ege_build_context;


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


void copy_and_log_file(ege_build_context* pContext, const char* srcPath, const char* dstPath)
{
    assert(pContext != NULL);
    assert(srcPath != NULL);
    assert(dstPath != NULL);

    if (easyvfs_copy_file(pContext->pVFS, srcPath, dstPath, false)) {
        printf("  Copied file %s to %s\n", srcPath, dstPath);
    } else {
        printf("  ERROR: Failed to copy %s to %s\n", srcPath, dstPath);
    }
}


void prebuild(ege_build_context* pContext)
{
    assert(pContext != NULL);

    printf("--- Performing Pre-Build Step ---\n");
    printf("1) Adding external dependencies to source tree...\n");
    {
        copy_and_log_file(pContext, "../../easy_audio/easy_audio.h", "../include/external/easy_audio/easy_audio.h");
        copy_and_log_file(pContext, "../../easy_audio/easy_audio.h", "../source/external/easy_audio/easy_audio.h");
        copy_and_log_file(pContext, "../../easy_audio/easy_audio.c", "../source/external/easy_audio/easy_audio.c");

        copy_and_log_file(pContext, "../../easy_draw/easy_draw.h", "../include/external/easy_draw/easy_draw.h");
        copy_and_log_file(pContext, "../../easy_draw/easy_draw.h", "../source/external/easy_draw/easy_draw.h");
        copy_and_log_file(pContext, "../../easy_draw/easy_draw.c", "../source/external/easy_draw/easy_draw.c");
        copy_and_log_file(pContext, "../../easy_draw/easy_mtl.h", "../include/external/easy_draw/easy_mtl.h");
        copy_and_log_file(pContext, "../../easy_draw/easy_mtl.h", "../source/external/easy_draw/easy_mtl.h");
        copy_and_log_file(pContext, "../../easy_draw/easy_mtl.c", "../source/external/easy_draw/easy_mtl.c");

        copy_and_log_file(pContext, "../../easy_fs/easy_fsw.h", "../include/external/easy_fs/easy_fsw.h");
        copy_and_log_file(pContext, "../../easy_fs/easy_fsw.h", "../source/external/easy_fs/easy_fsw.h");
        copy_and_log_file(pContext, "../../easy_fs/easy_fsw.c", "../source/external/easy_fs/easy_fsw.c");
        copy_and_log_file(pContext, "../../easy_fs/easy_vfs.h", "../include/external/easy_fs/easy_vfs.h");
        copy_and_log_file(pContext, "../../easy_fs/easy_vfs.h", "../source/external/easy_fs/easy_vfs.h");
        copy_and_log_file(pContext, "../../easy_fs/easy_vfs.c", "../source/external/easy_fs/easy_vfs.c");

        copy_and_log_file(pContext, "../../easy_gui/easy_gui.h", "../include/external/easy_gui/easy_gui.h");
        copy_and_log_file(pContext, "../../easy_gui/easy_gui.h", "../source/external/easy_gui/easy_gui.h");
        copy_and_log_file(pContext, "../../easy_gui/easy_gui.c", "../source/external/easy_gui/easy_gui.c");

        copy_and_log_file(pContext, "../../easy_math/easy_math.h", "../include/external/easy_math/easy_math.h");
        copy_and_log_file(pContext, "../../easy_math/easy_math.h", "../source/external/easy_math/easy_math.h");
        copy_and_log_file(pContext, "../../easy_math/easy_math.c", "../source/external/easy_math/easy_math.c");

        copy_and_log_file(pContext, "../../easy_path/easy_path.h", "../include/external/easy_path/easy_path.h");
        copy_and_log_file(pContext, "../../easy_path/easy_path.h", "../source/external/easy_path/easy_path.h");
        copy_and_log_file(pContext, "../../easy_path/easy_path.c", "../source/external/easy_path/easy_path.c");

        copy_and_log_file(pContext, "../../easy_physics/easy_physics.h", "../include/external/easy_physics/easy_physics.h");
        copy_and_log_file(pContext, "../../easy_physics/easy_physics.h", "../source/external/easy_physics/easy_physics.h");
        copy_and_log_file(pContext, "../../easy_physics/easy_physics.c", "../source/external/easy_physics/easy_physics.c");

        copy_and_log_file(pContext, "../../easy_util/easy_util.h", "../include/external/easy_util/easy_util.h");
        copy_and_log_file(pContext, "../../easy_util/easy_util.h", "../source/external/easy_util/easy_util.h");
        copy_and_log_file(pContext, "../../easy_util/easy_util.c", "../source/external/easy_util/easy_util.c");
    }

    printf("--- Pre-Build Step Complete ---\n");
}

void postbuild(ege_build_context* pContext)
{
    assert(pContext != NULL);

    printf("--- Performing Post-Build Step ---\n");
    printf("--- Post-Build Step Complete ---\n");
}


int build_gcc(ege_build_context* pContext)
{
    assert(pContext != NULL);
    return ERROR_NONE;
}

int build_clang(ege_build_context* pContext)
{
    assert(pContext != NULL);
    return ERROR_NONE;
}

int build_msvc(ege_build_context* pContext)
{
    assert(pContext != NULL);
    return ERROR_NONE;
}

int build(ege_build_context* pContext)
{
    assert(pContext != NULL);

    // Pre-build.
    if (!pContext->noPrebuild) {
        prebuild(pContext);
    }


    // Build
    switch (pContext->compiler)
    {
    case ege_compiler_gcc:   return build_gcc(pContext);
    case ege_compiler_clang: return build_clang(pContext);
    case ege_compiler_msvc:  return build_msvc(pContext);
    default: return ERROR_UNKNOWN_COMPILER;
    }

    
    // Post-build.
    if (!pContext->noPostbuild) {
        postbuild(pContext);
    }
}


bool on_cmdline(const char* key, const char* value, void* pUserData)
{
    ege_build_context* pContext = pUserData;
    assert(pContext       != NULL);
    assert(pContext->pVFS != NULL);

    if (strcmp(key, "[path]") == 0)
    {
        char basedir[EASYVFS_MAX_PATH];
        easypath_copybasepath(value, basedir, sizeof(basedir));

        if (easypath_isrelative(basedir))
        {
            char cwd[EASYVFS_MAX_PATH];
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

            easypath_append(cwd, sizeof(cwd), basedir);
            easypath_strcpy(basedir, sizeof(basedir), cwd);
        }


        // The base directory should be cleaned before adding it to the virtual file system, if only for sanity.
        char basedirClean[EASYVFS_MAX_PATH];
        easypath_clean(basedir, basedirClean, sizeof(basedirClean));

        // We should have a valid base directory.
        easyvfs_add_base_directory(pContext->pVFS, basedirClean);
        easyvfs_set_base_write_directory(pContext->pVFS, basedirClean);

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
            pContext->compiler = ege_compiler_gcc;
            return true;
        }
        if (strcmp(value, "clang") == 0) {
            pContext->compiler = ege_compiler_clang;
            return true;
        }
        if (strcmp(value, "msvc") == 0) {
            pContext->compiler = ege_compiler_msvc;
            return true;
        }

        printf("Error: Unknown or unsupported compiler: %s\n", value);
    }

    if (strcmp(key, "target") == 0)
    {
        if (strcmp(value, "debug") == 0) {
            pContext->target = ege_target_debug;
            return true;
        }
        if (strcmp(value, "release") == 0) {
            pContext->target = ege_target_release;
            return true;
        }

        printf("Error: Unknown or unsupported target: %s\n", value);
    }


    return true;
}

int main(int argc, char** argv)
{
    int result = ERROR_NONE;

    easyutil_cmdline cmdline;
    easyutil_init_cmdline(&cmdline, argc, argv);

    ege_build_context context;
    context.noPrebuild   = false;
    context.noPostbuild  = false;
    context.doBuild      = true;
    context.compiler     = ege_compiler_none;
    context.target       = ege_target_none;
    context.pVFS         = easyvfs_create_context();
    easyutil_parse_cmdline(&cmdline, on_cmdline, &context);


    // If we did the pre- or post-build (--prebuild or --postbuild), don't continue with the rest of the build.
    if (context.doBuild)
    {
        // At this point we now want to continue with a build.
        if (result == ERROR_NONE && context.compiler == ege_compiler_none) {
            printf("Error: No compiler specified. Use --compiler {gcc|clang|msvc} to specify a compiler.\n");
            result = ERROR_UNKNOWN_COMPILER;
        }

        if (result == ERROR_NONE && context.target == ege_target_none) {
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
    easyvfs_delete_context(context.pVFS);

    return result;
}
// Public domain. See "unlicense" statement at the end of this file.

// OPTIONS
//
// #define DR_GE_DISABLE_EDITOR
//   Excludes the editor from the build. On Linux, this removes the dependency on GTK.
//
// #define DR_GE_PORTABLE
//   Builds a portable version of the engine. That is, configures the engine such that it stores configs, saves, logs, etc. relative
//   to the executable location rather than the user directory.
//
// #define DR_GE_USE_EXTERNAL_REPOS (Engine Developers Only)
//   Uses external repositories for dr_libs and dr_appkit which are assumed to be located in the same directory as the dr_ge
//   repository, such as:
//     <root directory>
//       - dr_ge
//       - dr_libs
//       - dr_appkit
//
//   When this is unset (the default), the internal copy of each repo is used. This is not recommended because errors will be reported
//   with respect to the internal files, however those errors would need to be fixed in the main repositories instead. This can cause
//   annoying confusion, so if you're actively developing dr_libs, dr_appkit and dr_ge, you best use this option.

#ifndef dr_ge_h
#define dr_ge_h

// These #defines enable us to load large files on Linux platforms. They need to be placed before including any headers.
#ifndef _WIN32
#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif

#ifndef _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 64
#endif
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

#ifdef DR_GE_USE_EXTERNAL_REPOS
#include "../dr_libs/dr_util.h"
#include "../dr_libs/dr_path.h"
#include "../dr_libs/dr_vfs.h"
#else
#include "source/external/dr_libs/dr_util.h"
#include "source/external/dr_libs/dr_path.h"
#include "source/external/dr_libs/dr_vfs.h"
#endif

#include "source/drge_context.h"
#include "source/drge_platform_layer.h"

#ifndef DR_GE_DISABLE_EDITOR
#ifdef DR_GE_USE_EXTERNAL_REPOS
//#include "../dr_appkit/dr_appkit.h"
#else
//#include "source/external/dr_appkit/dr_appkit.h"
#endif
#include "source/editor/drge_editor.h"
#endif  //DR_GE_DISABLE_EDITOR


#ifdef DR_GE_IMPLEMENTATION
#include "source/drge_context.c"
#include "source/drge_platform_layer.c"

// dr_libs
#define DR_UTIL_IMPLEMENTATION
#define DR_PATH_IMPLEMENTATION
#define DR_VFS_IMPLEMENTATION

#ifdef DR_GE_USE_EXTERNAL_REPOS
#include "../dr_libs/dr_util.h"
#include "../dr_libs/dr_path.h"
#include "../dr_libs/dr_vfs.h"
#else
#include "source/external/dr_libs/dr_util.h"
#include "source/external/dr_libs/dr_path.h"
#include "source/external/dr_libs/dr_vfs.h"
#endif

#ifndef DR_GE_DISABLE_EDITOR
#include "source/editor/drge_editor.c"

// dr_appkit
#define DR_APPKIT_IMPLEMENTATION
#ifdef DR_GE_USE_EXTERNAL_REPOS
//#include "../dr_appkit/dr_appkit.h"
#else
//#include "source/external/dr_appkit/dr_appkit.h"
#endif

#endif  //DR_GE_DISABLE_EDITOR
#endif  //DR_GE_IMPLEMENTATION

#endif  //dr_ge_h


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

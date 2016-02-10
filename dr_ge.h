// Public domain. See "unlicense" statement at the end of this file.

// USAGE
//
// Building the engine is easy - just do something like the following in one .c file.
//   #define DR_GE_IMPLEMENTATION
//   #include "dr_ge.h"
//
// There may be a few system level dependencies, but they should all be ubiquitous enough that you shouldn't need to worry about
// installing anything.
//
// Windows Dependencies:
//   - msimg32.lib (Only if building the editing tools.) This should always be pre-installed, but may need to be explicitly added
//     to your list of link libraries if the linker can't find AlphaBlend.
//
// Linux Dependencies:
//   - pthreads
//   - Xlib
//   - GTK+ 3 (Only if building the editing tools)
//
//
//
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
//   When this is left unset, the internal copy of dr_libs and dr_appkit will be used to build the engine. This is not recommended
//   for engine developers because the main repositories are where the most up-to-date versions of these libraries are located.

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

// Standard headers.
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

// Platform headers. Never expose these publicly. Ever.
#ifdef DR_GE_IMPLEMENTATION
#ifdef _WIN32
#include <windows.h>
#endif
#ifdef __linux__
#include <gdk/gdk.h>
#include <gtk/gtk.h>
#endif
#endif

// Platform libraries, for simplifying MSVC builds.
#ifdef _WIN32
#if defined(_MSC_VER) || defined(__clang__)
#pragma comment(lib, "msimg32.lib")
#endif
#endif


// dr_libs headers.
#define DR_GUI_INCLUDE_WIP

#ifdef DR_GE_USE_EXTERNAL_REPOS
#include "../dr_libs/dr_util.h"
#include "../dr_libs/dr_path.h"
#include "../dr_libs/dr_vfs.h"
#include "../dr_libs/dr_gui.h"
#include "../dr_libs/dr_2d.h"
#include "../dr_libs/dr_audio.h"
#include "../dr_libs/dr_wav.h"
#include "../dr_libs/dr_flac.h"
#include "../dr_libs/dr_mtl.h"
#include "../dr_libs/dr_math.h"
#else
#include "source/external/dr_util.h"
#include "source/external/dr_path.h"
#include "source/external/dr_vfs.h"
#include "source/external/dr_gui.h"
#include "source/external/dr_2d.h"
#include "source/external/dr_audio.h"
#include "source/external/dr_wav.h"
#include "source/external/dr_flac.h"
#include "source/external/dr_mtl.h"
#include "source/external/dr_math.h"
#endif

// dr_ge headers.
#include "source/drge_context.h"
#include "source/drge_platform_layer.h"

// dr_ge editor headers.
#ifndef DR_GE_DISABLE_EDITOR
#ifdef DR_GE_USE_EXTERNAL_REPOS
#include "../dr_libs/dr_fsw.h"
#include "../dr_appkit/source/dr_appkit_inner.h"
#else
#include "source/external/dr_fsw.h"
#include "source/external/dr_appkit/source/dr_appkit_inner.h"
#endif

#include "source/editor/drge_editor_tool_types.h"
#include "source/editor/drge_editor.h"
#include "source/editor/drge_editor_main_menu.h"
#include "source/editor/drge_editor_sub_editor.h"
#include "source/editor/drge_editor_text_editor.h"
#include "source/editor/drge_editor_image_editor.h"
#endif  //DR_GE_DISABLE_EDITOR


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// IMPLEMENTATION
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#ifdef DR_GE_IMPLEMENTATION
// stb_image
#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_STDIO
#include "source/external/stb_image.h"

// stb_vorbis. This is a special case because if we place the implementation here it results in compiler
// errors on the Windows build. Suspecting it's due to conflicting types, but not sure. To address this
// we just place the header here and then place the implementation down the bottom.
#define STB_VORBIS_HEADER_ONLY
#define STB_VORBIS_NO_STDIO
#include "source/external/stb_vorbis.c"



// dr_ge source files.
#include "source/drge_context.c"
#include "source/drge_platform_layer.c"

// dr_libs
#define DR_UTIL_IMPLEMENTATION
#define DR_PATH_IMPLEMENTATION
#define DR_VFS_IMPLEMENTATION
#define DR_GUI_IMPLEMENTATION
#define DR_2D_IMPLEMENTATION
#define DR_AUDIO_IMPLEMENTATION
#define DR_WAV_IMPLEMENTATION
#define DR_FLAC_IMPLEMENTATION
#define DR_MTL_IMPLEMENTATION
#define DR_MATH_IMPLEMENTATION

#ifdef DR_GE_USE_EXTERNAL_REPOS
#include "../dr_libs/dr_util.h"
#include "../dr_libs/dr_path.h"
#include "../dr_libs/dr_vfs.h"
#include "../dr_libs/dr_gui.h"
#include "../dr_libs/dr_2d.h"
#include "../dr_libs/dr_audio.h"
#include "../dr_libs/dr_wav.h"
#include "../dr_libs/dr_flac.h"
#include "../dr_libs/dr_mtl.h"
#include "../dr_libs/dr_math.h"
#else
#include "source/external/dr_util.h"
#include "source/external/dr_path.h"
#include "source/external/dr_vfs.h"
#include "source/external/dr_gui.h"
#include "source/external/dr_2d.h"
#include "source/external/dr_audio.h"
#include "source/external/dr_wav.h"
#include "source/external/dr_flac.h"
#include "source/external/dr_mtl.h"
#include "source/external/dr_math.h"
#endif

#ifndef DR_GE_DISABLE_EDITOR
#include "source/editor/drge_editor.c"
#include "source/editor/drge_editor_main_menu.c"
#include "source/editor/drge_editor_sub_editor.c"
#include "source/editor/drge_editor_text_editor.c"
#include "source/editor/drge_editor_image_editor.c"

// dr_appkit
#define DR_FSW_IMPLEMENTATION
#define DR_APPKIT_IMPLEMENTATION
#ifdef DR_GE_USE_EXTERNAL_REPOS
#include "../dr_libs/dr_fsw.h"
#include "../dr_appkit/source/dr_appkit_inner.h"
#else
#include "source/external/dr_appkit/source/dr_appkit_inner.h"
#include "source/external/dr_fsw.h"
#endif

// stb_vorbis implementation.
#include "source/external/stb_vorbis.c"

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

// Public domain. See "unlicense" statement at the end of this file.

#ifndef dr_ge_h
#define dr_ge_h

#include <stdlib.h>

#include "../dr_libs/dr_util.h"
#include "../dr_libs/dr_path.h"

#include "source/drge_context.h"
#include "source/drge_platform_layer.h"

#ifdef DR_GE_IMPLEMENTATION
#include "source/drge_context.c"
#include "source/drge_platform_layer.c"

#define DR_UTIL_IMPLEMENTATION
#include "../dr_libs/dr_util.h"

#define DR_PATH_IMPLEMENTATION
#include "../dr_libs/dr_path.h"
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

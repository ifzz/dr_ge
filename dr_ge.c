// Public domain. See "unlicense" statement at the end of dr_ge.h.

// Configure your build from here.

//#define DR_GE_DISABLE_EDITOR
//  Excludes the editor from the build. On Linux, this removes the dependency on GTK.
//
//#define DR_GE_PORTABLE
//  Builds a portable version of the engine. That is, configures the engine such that it stores configs, saves, logs, etc. relative
//  to the executable location rather than the user directory.

#define DR_GE_IMPLEMENTATION
#include "dr_ge.h"

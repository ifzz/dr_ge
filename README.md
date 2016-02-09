# About
(If you've stumbled across this project please keep in mind that this is
still very early in development and incomplete.)

dr_ge is a simple, ultra light-weight game engine written in C and released
into the public domain. Here's what makes it different from the rest.
 * A simple build system with no external dependencies (see below)
 * A small code base
 * A simple, clean, no BS API
 * Public domain
 
There are a few things this project does differently that goes against popular
opinion - but that's the point. I want to do something a bit different, and
there are other projects out there that use more modern languages and don't
focus as much on keeping things simple and ultra light-weight.


# Dependencies
There are no dependencies that need to be built external to the engine, however
there are a few dependencies whose repositories you'll need to clone for the
time being while the engine's still in early development:
 * dr_libs (https://github.com/mackron/dr_libs)
 * dr_appkit (Optional: Only used for editing tools) (https://github.com/mackron/dr_appkit)
 
With these repositories you should clone them to a common folder so that your
directory structure looks like the following:
 - <root directory>
   - dr_ge
   - dr_libs
   - dr_appkit (only if including editing tools with the build)

Later on when these libraries become a bit more stable they will be added to
dr_ge's repository directly.
 
Other dependencies are included with dr_ge's repository directly and should
not require any special handling at all, but are listed here for credit:
 * stb libraries (https://github.com/nothings/stb)
   * stb_image
   * stb_vorbis
 
 
# Non-dependencies
There's a few libraries that are commonly used that we are not going to depend
on, mainly because they complicate the build system which violates the engine's
first rule. These libraries include:

 * **SDL on Windows** (maybe on Linux, but we'll see)
 * **FreeType on Windows** (GDI's font APIs aren't that bad)
 * **libpng** and **zlib** (stb_image and miniz are good enough, and are easier to build)
   
   
# Building
Building the engine is easy:
 1) Clone dr_ge, dr_libs and dr_appkit into a common directory
 2) Add dr_ge/dr_ge.c to your project as you would like any other source file, or do
something like the following in a single .c file:
    #define DR_GE_IMPLEMENTATION
	#include "dr_ge.h"
	
In the future cloning repositories will be optional and the entire engine,
including dependencies, will be wholly contained within a single .h file.
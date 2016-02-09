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


# Building
Building the engine is easy - just clone the dr_ge repository and either:
 - Add dr_ge.c to your project like you would any other source file; or
 - Do something like the following in one .c file:

    ```c
    #define DR_GE_IMPLEMENTATION
    #include "dr_ge.h"
    ```

You may need to link to some system level libraries which are detailed in
dr_ge.h.

In the future cloning repositories will be optional and the entire engine,
including dependencies, will be wholly contained within a single .h file.


# Dependencies
All dependencies are included in the repository and should not require any
special handling.
 * dr_libs (https://github.com/mackron/dr_libs)
 * dr_appkit (editor only) (https://github.com/mackron/dr_appkit)
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

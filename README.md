# About
(If you've stumbled across this project please keep in mind that this is
still very early in development and incomplete.)

dr_ge is a simple, ultra light-weight game engine written in C and released
into the public domain. Here's what makes it different from the rest.
 * A simple build system with no external dependencies
 * A small code base
 * A simple, clean, no BS API
 * Public domain
 
There are a few things this project does differently that goes against popular
opinion - but that's the point. I want to do something a bit different, and
there are other projects out there that use more modern languages and don't
focus as much on keeping things simple and ultra light-weight.


# Dependencies
There are very few dependencies because that would complicate the build system.
There are, however, a few dependencies that are important or simple enough to
allow:
 * dr_libs (https://github.com/mackron/dr_libs)
 * dr_appkit (Editing tools only) (https://github.com/mackron/dr_appkit)
 * stb libraries (https://github.com/nothings/stb)
   * stb_image
   * stb_vorbis

Note that all of the above libraries are included in the source tree directly
so there is no need to compile those separately.
 
 
# Non-dependencies
There's a few libraries that are commonly used that we are not going to depend
on, mainly because they complicate the build system which violates the engine's
first rule. These libraries include:

 * **SDL on Windows** (maybe on Linux, but we'll see)
 * **FreeType on Windows** (GDI's font APIs aren't that bad)
 * **libpng** and **zlib** (stb_image and miniz are good enough, and are easier to build)
   
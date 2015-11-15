# Plugins
The engine uses plugins to provide functionality for certain things. This
document discusses the rationale behine this design, it's pros and cons and
explains how plugins work.


## Rationale
There are several reasons a plugin architecture has been chosen:
 1) It allows third-party dependencies to be offloaded to plugins which can
    be pre-built and placed into the repository directly. This way developers
	who want to get up and running with the engine do not need to mess around
	with third-party dependencies and instead just use the pre-built plugin
	binaries.
 2) It allows alternate implementations to be dropped in without needing to
    recompile. For example, one could have an implementation of a scripting
	plugin that uses the reference Lua 5.1 library, and then another that uses
	LuaJIT. The user can then choose which one to use via a config entry.
 3) When a third-party dependency is written in C++ or uses a different
    licence, they can be isolated to the plugin rather than placed in the main
	code base. This keeps the engine's main code base clean and consistent.
	
Because one of the primary goals of the engine is to keep the build system
simple, offloading third-party libraries into plugins and making use of
pre-built binaries helps with this.

A downside to a plugin architecture is that it complicates the distribution of
a game because there are more binaries to distribute. This can be mitigated
somewhat by having plugins stored in a standard location.

Another downside is that it introduces another API layer which is required for
the plugin abstraction.


## How it Works
There are different categories of plugins. Currently, these include:
 - Physics
 - Scripting
 
At load time, the engine will iterate over each .dll or .so file (depending on
the platform) in the "plugins" folder which should be placed relative to the
executable.

When a specific plugin is specified in the game config, the engine will choose
that plugin if it's available. If that plugin does not exist, or a specific
plugin was not specified, the engine will decide for itself which to use.

Every plugin should implement a function with the following signature
    int ege_plugin(char* pNameOut, unsigned int nameSizeInBytes)
	
This function returns the plugin type and outputs the name of the plugin. The
plugin name is used to allow the engine to reference it by name for things like
allowing config files to reference the plugin with human-readable names or
whatnot.


## For Developers
Only a single plugin of each type is maintained in the main repository because
because otherwise it just creates extra unnecessary code maintanence. If you
would like to make your own plugin, you will need to maintain it in a separate
repository.


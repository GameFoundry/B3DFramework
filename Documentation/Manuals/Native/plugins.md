Creating custom plugins						{#customPlugins}
===============
[TOC]

Many systems in Banshee are implemented through plugins, libraries that are separate from the core of the engine and can be dynamically loaded or unloaded. If possible, it is the prefered way of extending the engine.

The default Banshee @ref bs::CoreApplication "Application" supports plugins for the following systems:
 - Rendering API - Wrappers for render APIs like DirectX or OpenGL. See the manual about working with [low level render API] (@ref renderAPI) for more information.
 - Renderer - Renderer that determines how is the scene displayed (lighting, shadows, post-processing, etc.). See the manual about implementing [custom renderers](@ref renderer).
 - Input - Reports input events (mouse, keyboard, gamepad, etc.)
 - Physics - Runs the physics simulation (like NVIDIA PhysX)
 - Importers - Importers that handle conversion of some third party resource format into an engine-ready format. See the manual about implementing [custom importers](@ref customImporters).
 
The supported plugins will be automatically loaded and unloaded by the application as needed, all you need to do is to provide names of their libraries to the @ref bs::START_UP_DESC "START_UP_DESC" used to initialize the application. All plugins should be in the same folder as the main application executable. 

Aside from the supported plugins you can also create fully custom plugins that you load or unload manually.

# Implementing supported plugins {#customPlugins_a}
All supported plugins implement an informal interface through global "extern" methods. The interface supports three methods:
 - loadPlugin() - Called when the plugin is initially loaded
 - updatePlugin() - Called every frame
 - unloadPlugin() - Called just before the plugin is unloaded
 
You may choose to implement some, or none of these, although usually at least `loadPlugin()` method is needed so the plugin can register itself with the necessary system. A simple implementation might look like so:
~~~~~~~~~~~~~{.cpp}

extern "C" BS_MYPLUGIN_EXPORT void* loadPlugin()
{
	// Do something on load
	return nullptr; // Not used
}

extern "C" BS_MYPLUGIN_EXPORT void updatePlugin()
{
	// Do something every frame
}

extern "C" BS_MYPLUGIN_EXPORT void unloadPlugin()
{
	// Do something before unload
}

~~~~~~~~~~~~~

It's important that all instances of types (classes/structs) from the plugin are deleted before plugin unload happens. If this doesn't happen, and an object instance is deleted after the plugin has been unloaded you will end up with a corrupt virtual function table and a crash. Normally this is handled for you, but it's good to keep in mind depending on what your plugin is doing.

The exact implementations of these methods differ depending for which system are you implementing a plugin for, but we won't go into details for individual systems here. In most cases it just involves registering the plugin instance using some global manager. For example check "BsSLPlugin.cpp" in the @ref BansheeSL plugin, which registers a new importer that supports ".bsl" files.

# Custom plugins {#customPlugins_b}
Custom plugins can do whatever you wish, engine has no expectations from them so its up to your to load/unload them and to call their methods.

To load a custom plugin you can use:
 - @ref bs::CoreApplication::loadPlugin "Application::loadPlugin" - Accepts the name of the plugin library and outputs the library object. Optionally you may also pass a parameter to the `loadPlugin` method, if yours accepts one.
 - @ref bs::CoreApplication::unloadPlugin "Application::unloadPlugin" - Unloads a previously loaded plugin. 

Both of those methods internally call @ref bs::DynLibManager "DynLibManager". You can use it directly if you do not need the plugin interface (`loadPlugin` and etc.), it has two methods:
 - @ref bs::DynLibManager::load "DynLibManager::load" - Accepts a file name to the library, and returns the @ref bs::DynLib "DynLib" object if the load is successful or null otherwise. 
 - @ref bs::DynLibManager::unload "DynLibManager::unload" - Unloads a previously loaded library.
 
Once the library is loaded you can use the @ref bs::DynLib "DynLib" object, and its @ref bs::DynLib::getSymbol "DynLib::getSymbol" method to retrieve a function pointer within the dynamic library, and call into it. For example if we wanted to retrieve a function pointer for the `loadPlugin` method from the previous chapter:
~~~~~~~~~~~~~{.cpp}
// Load library
DynLib* myLibrary = DynLibManager::instance().load("myPlugin");

// Retrieve function pointer (symbol)
typedef void* (*LoadPluginFunc)();
LoadPluginFunc loadPluginFunc = (LoadPluginFunc)myLibrary->getSymbol("loadPlugin");

// Call the function
loadPluginFunc();

// Assuming we're done, unload the plugin
DynLibManager::instance().unload(myLibrary);
~~~~~~~~~~~~~
Plugins						{#plugins}
===============
[TOC]

Many systems in bs::f are implemented through plugins, libraries that are separate from the core of the engine and can be dynamically loaded or unloaded. If possible, it is the prefered way of extending the engine.

bs::f supports plugins for the following systems:
 - Audio - Systems for providing audio playback.
 - Importers - Importers that handle conversion of some third party resource format into an engine-ready format.
 - Physics - Runs the physics simulation.
 - Renderer - Determines how is the scene displayed (lighting, shadows, post-processing, etc.). 
 - Rendering backend - Wrappers for render APIs like DirectX, OpenGL or Vulkan.
 
You can choose which plugins are loaded on **Application** start-up, by filling out the @ref bs::START_UP_DESC "START_UP_DESC" structure.

~~~~~~~~~~~~~{.cpp}
START_UP_DESC startUpDesc;

// Required plugins
startUpDesc.renderAPI = "bsfD3D11RenderAPI";
startUpDesc.renderer = "bsfRenderBeast";
startUpDesc.audio = "bsfOpenAudio";
startUpDesc.physics = "bsfPhysX";
startUpDesc.input = "bsfOISInput";

// List of importer plugins we plan on using for importing various resources
startUpDesc.importers.push_back("bsfFreeImgImporter"); // For importing textures
startUpDesc.importers.push_back("bsfFBXImporter"); // For importing meshes
startUpDesc.importers.push_back("bsfFontImporter"); // For importing fonts
startUpDesc.importers.push_back("bsfSL"); // For importing shaders

// ... also set up primary window in startUpDesc ...

Application::startUp(startUpDesc);

// ... create scene, run main loop, shutdown
~~~~~~~~~~~~~ 
 
In this manual we'll focus on general functionality common to all plugins, while we'll talk about how to implement plugins for specific systems in later manuals. 

# Generating a CMake project {#plugins_a}
Plugins are always created as their own projects/libraries. bs::f uses the CMake build system for managing its projects. Therefore the first step you need to take is to create your own CMake project. This involves creating a new folder in the /Source/Plugins directory (e.g. Source/Plugins/MyPlugin), with a CMakeLists.txt file inside it. CMakeLists.txt will contain references to needed header & source files, as well as dependencies to any other libraries. 
 
An example CMakeLists.txt might look like so:
~~~~~~~~~~~~~
# Source files
set(SOURCE_FILES
	"Include/MyHeader.h"
	"Source/MyPlugin.cpp"	
)

# Target
## Registers our plugin a specific name (MyPlugin) and with the relevant source files
add_library(MyPlugin SHARED ${SOURCE_FILES})

# Include directories
## Including just the current folder
target_include_directories(MyPlugin PRIVATE "./")

# Libraries
## Link with bsf
target_link_libraries(MyPlugin PUBLIC bsf)
~~~~~~~~~~~~~

Note that we won't go into details about CMake syntax, it's complex and there are many other guides already written on it.

After creating your project's CMake file, you need to register it with the main CMakeLists.txt present in the /Source directory. Simply append the following line:
~~~~~~~~~~~~~
# Provided your plugin is located in Source/Plugins/MyPlugin folder
add_subdirectory(Plugins/MyPlugin)
~~~~~~~~~~~~~

# Plugin interface {#customPlugins_b}
If you wish to create a plugin for any of the systems listed above, you will need to implement an informal interface through global "extern" methods. The interface supports three functions:
 - **loadPlugin()** - Called when the plugin is initially loaded
 - **updatePlugin()** - Called every frame
 - **unloadPlugin()** - Called just before the plugin is unloaded
 
You may choose to implement some, or none of these, although usually at least **loadPlugin()** method is needed so the plugin can register itself with the necessary system.
~~~~~~~~~~~~~{.cpp}
class MyPlugin : Module<MyPlugin>
{
};

extern "C" BS_MYPLUGIN_EXPORT void* loadPlugin()
{
	MyPlugin::startUp();

	return nullptr; // Not used
}

extern "C" BS_MYPLUGIN_EXPORT void updatePlugin()
{
	// Do something every frame
}

extern "C" BS_MYPLUGIN_EXPORT void unloadPlugin()
{
	MyPlugin::shutDown();
}

// BS_MYPLUGIN_EXPORT is a macro for a compiler-specific export attribute
// (e.g. __declspec(dllexport) for Visual Studio (MSVC))
~~~~~~~~~~~~~

After you have your plugin interface, all you need to do is to pass the name of your plugin (as defined in CMake) to one of the entries in **START_UP_DESC** for it to be loaded.

> It's important that all objects created by the plugin are deleted before plugin unload happens. If this doesn't happen, and an object instance is deleted after the plugin has been unloaded you will end up with a corrupt virtual function table and a crash. 

# Fully custom plugins {#plugins_c}
You can also create a fully customized plugin that doesn't implement functionality for any existing engine system. The engine has no interface expectations for such plugins, and it's up to you to manually load/unload them, as well as to manually call their functions.

To load a custom plugin you can use:
 - @ref bs::CoreApplication::loadPlugin "Application::loadPlugin()" - Accepts the name of the plugin library and outputs the library object. Optionally you may also pass a parameter to the **loadPlugin** method, if your plugin defines one.
 - @ref bs::CoreApplication::unloadPlugin "Application::unloadPlugin()" - Unloads a previously loaded plugin. 

~~~~~~~~~~~~~{.cpp}
DynLib* pluginLib;
gApplication()::loadPlugin("MyPlugin", &pluginLib);
// Do something
gApplication()::unloadPlugin(pluginLib);
~~~~~~~~~~~~~ 
 
Both of those methods internally call **DynLibManager** which we described earlier. In fact you can also use it directly for loading plugins, as an alternative to this approach.

Once the library is loaded you can use the @ref bs::DynLib "DynLib" object, and its @ref bs::DynLib::getSymbol "DynLib::getSymbol()" method to retrieve a function pointer within the dynamic library, and call into it. 
~~~~~~~~~~~~~{.cpp}
// Retrieve function pointer (symbol)
typedef void* (*LoadPluginFunc)();
LoadPluginFunc loadPluginFunc = (LoadPluginFunc)pluginLib->getSymbol("loadPlugin");

// Call the function
loadPluginFunc();
~~~~~~~~~~~~~
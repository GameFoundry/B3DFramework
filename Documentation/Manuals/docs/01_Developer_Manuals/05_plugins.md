---
title: Plugins
---

Many systems in the framework are implemented through plugins. Plugins can be built as dynamic libraries (loaded at runtime) or statically linked into a monolithic build, controlled by the `B3D_MONOLITHIC_BUILD` CMake option. If possible, it is the preferred way of extending the engine.

Framework supports plugins for the following systems:
 - Audio - Systems for providing audio playback.
 - Importers - Importers that handle conversion of some third party resource format into an engine-ready format.
 - Physics - Runs the physics simulation.
 - Renderer - Determines how is the scene displayed (lighting, shadows, post-processing, etc.). 
 - Rendering backend - Wrappers for render APIs like DirectX, OpenGL or Vulkan.
 
You can choose which plugins are loaded on **Application** start-up, by filling out the @b3d::START_UP_DESC structure.

~~~~~~~~~~~~~{.cpp}
START_UP_DESC startUpDesc;

// Required plugins
startUpDesc.renderAPI = "bsfD3D11RenderAPI";
startUpDesc.renderer = "bsfRenderBeast";
startUpDesc.audio = "bsfOpenAudio";
startUpDesc.physics = "bsfPhysX";
// List of importer plugins we plan on using for importing various resources
startUpDesc.importers.push_back("bsfFreeImgImporter"); // For importing textures
startUpDesc.importers.push_back("bsfFBXImporter"); // For importing meshes
startUpDesc.importers.push_back("bsfFontImporter"); // For importing fonts
startUpDesc.importers.push_back("bsfSL"); // For importing shaders

// ... also set up primary window in startUpDesc ...

Application::StartUp(startUpDesc);

// ... create scene, run main loop, shutdown
~~~~~~~~~~~~~ 
 
In this manual we'll focus on general functionality common to all plugins, while we'll talk about how to implement plugins for specific systems in later manuals. 

# Generating a CMake project
Plugins are always created as their own projects/libraries. Framework uses the CMake build system for managing its projects. Therefore the first step you need to take is to create your own CMake project. This involves creating a new folder in the /Source/Plugins directory (e.g. Source/Plugins/MyPlugin), with a CMakeLists.txt file inside it. CMakeLists.txt will contain references to needed header & source files, as well as dependencies to any other libraries. 
 
An example CMakeLists.txt might look like so:
~~~~~~~~~~~~~
# Source files
set(SOURCE_FILES
	"Include/MyHeader.h"
	"Source/MyPlugin.cpp"	
)

# Target
## Registers our plugin with a specific name (MyPlugin) and with the relevant source files
## Use ENGINE for plugins that can be statically linked, or IMPORTER for plugins that always remain dynamic
B3DAddPlugin(MyPlugin ENGINE ${SOURCE_FILES})

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

# Plugin interface
If you wish to create a plugin for any of the systems listed above, you will need to implement an informal interface through global "extern" methods. Each plugin exposes named registration functions following a consistent naming convention based on the CMake target name:

 - **LoadPlugin_\<TARGET\>()** - Called when the plugin is loaded (the core registration function)
 - **UnloadPlugin_\<TARGET\>(void*)** - Called when the plugin is unloaded
 - **UpdatePlugin_\<TARGET\>()** - Called every frame (optional)

Additionally, the existing dynamic-mode trampolines call the named functions:
 - **LoadPlugin()** - Trampoline that calls LoadPlugin_\<TARGET\>()
 - **UnloadPlugin()** - Trampoline that calls UnloadPlugin_\<TARGET\>()

All non-importer plugins follow the same pattern: **LoadPlugin_\<TARGET\>()** allocates a factory object and returns it as a `void*`, and **UnloadPlugin_\<TARGET\>(void*)** receives that same pointer and deletes it. The factory is the object that owns the plugin's lifecycle — the matching manager (PhysicsManager, AudioManager, GpuBackendManager, RendererManager) stores it and invokes its StartUp/ShutDown (or Create) methods.

~~~~~~~~~~~~~{.cpp}
class MyPluginFactory : public SomeFactoryBase
{
public:
	void StartUp() override { /* start MyPlugin's module */ }
	void ShutDown() override { /* stop MyPlugin's module */ }
};

// Named registration function — called directly in monolithic mode, and via
// the trampoline below in dynamic mode. Returns a factory pointer that the
// matching manager takes ownership of.
extern "C" void* LoadPlugin_MyPlugin()
{
	return static_cast<void*>(B3DNew<MyPluginFactory>());
}

extern "C" void UnloadPlugin_MyPlugin(void* instance)
{
	B3DDelete(static_cast<SomeFactoryBase*>(instance));
}

// Dynamic-mode trampolines — the engine's PluginLoader looks up the symbols
// "LoadPlugin" and "UnloadPlugin" by name in the plugin DLL, so both
// trampolines must be present.
extern "C" B3D_PLUGIN_EXPORT void* LoadPlugin()
{
	return LoadPlugin_MyPlugin();
}

extern "C" B3D_PLUGIN_EXPORT void UnloadPlugin(MyPluginFactory* instance)
{
	UnloadPlugin_MyPlugin(instance);
}
~~~~~~~~~~~~~

After you have your plugin interface, all you need to do is to pass the name of your plugin (as defined in CMake) to one of the entries in **START_UP_DESC** for it to be loaded.

> It's important that all objects created by the plugin are deleted before plugin unload happens. If this doesn't happen, and an object instance is deleted after the plugin has been unloaded you will end up with a corrupt virtual function table and a crash. 

# Fully custom plugins
You can also create a fully customized plugin that doesn't implement functionality for any existing engine system. The engine has no interface expectations for such plugins, and it's up to you to manually load/unload them, as well as to manually call their functions.

To load a custom plugin you can use:
 - @b3d::Application::LoadPlugin - Accepts the name of the plugin. Optionally you may also pass a parameter to the **LoadPlugin** method, if your plugin defines one.
 - @b3d::Application::UnloadPlugin - Unloads a previously loaded plugin. 

~~~~~~~~~~~~~{.cpp}
GetApplication().LoadPlugin("MyPlugin");
// Do something
GetApplication().UnloadPlugin("MyPlugin");
~~~~~~~~~~~~~ 
 
Both of those methods internally use **PluginLoader** which transparently handles both dynamic and monolithic build modes.

---
title: Architecture
---

This manual will explain the architecture of the framework, to give you a better idea of how everything is structured and where to locate particular systems.

Framework is implemented throughout many separate libraries. Spreading the engine implementation over different libraries ensures multiple things:
 - Portions of the engine can be easily modified or replaced
 - User can choose which portions of the engine he requires
 - Internals are easier to understand as libraries form a clear architecture between themselves, while ensuring source code isn't all bulked into one big package
 - It ensures a higher quality design by decoupling code and forcing an order of dependencies
 
All the libraries can be separated into two main categories:
 - Engine - These are the core libraries of the engine. They are designed in layers, where each layer is built on top of the previous layer and provides higher level and more specific functionality than the previous one.
 - Plugins - These are separate, independant, and in most cases optional libraries containing various high level systems. They usually implement some interface that was defined in one of the engine layers. You are able to design your own plugins that completely replace certain portion of the engine functionality without having to modify the engine itself (e.g. use a new physics library, or a renderer backend)
 
# Engine #
The engine layers contain the core of the engine. All the essentials and all the abstract interfaces for plugins belong here.

The engine is split into two layers to decouple code. Lower layers do not know about higher layers and low level code never caters to specialized high level code. This makes the design cleaner and forces a certain direction for dependencies.

Lower layers were designed to be more general purpose than higher layers. They provide very general techniques usually usable in various situations, and they attempt to cater to everyone. On the other hand higher layers provide a lot more focused and specialized techniques. This might mean relying on very specific rendering APIs, platforms or plugins but it also means using newer, fancier and maybe not as widely accepted techniques (e.g. some new rendering algorithm or a shader).

Going from the lowest to highest the layers are:
## bsfUtility ##
This is the lowest layer of the engine. It is a collection of very decoupled and separate systems that are likely to be used throughout all of the higher layers. Essentially a collection of tools that are in no way tied into a larger whole. Most of the functionality isn't even game engine specific, like providing @b3d::FileSystem[file-system access], @b3d::Path[file path parsing], @b3d::Event[events], @b3d::Math[math library], @b3d::RTTITypeBase[RTTI system], @b3d::ThreadPool[threading primitives and managers], among various others.

All symbols are in the `b3d::` namespace, exported via `B3D_EXPORT`, and the primary prerequisite header is `B3DUtilityPrerequisites.h`.

## bsfEngine ##
This layer builds upon the utility layer and contains the full breadth of engine functionality: abstract interfaces for all major systems (@b3d::GpuBackend, @b3d::Resources, @b3d::Importer, @b3d::Input, @b3d::Physics, @b3d::Audio and more), higher-level systems (@b3d::GUIManager, @b3d::ScriptManager), scene management, and the @b3d::Application entry-point class. Plugin implementations provide the concrete backends for most of the abstract interfaces.

Code running on the main thread lives in the `b3d::` namespace. Code that runs on the render thread lives in the `b3d::render::` namespace. All symbols are exported via `B3D_EXPORT` and the primary prerequisite header is `B3DPrerequisites.h`.

# Plugins #
Framework provides a wide variety of plugins out of the box. The plugins are loaded dynamically and allow you to change engine functionality completely transparently to other systems (e.g. you can choose to load a DirectX 12 renderer instead of a Vulkan one). Some plugins are completely optional and you can choose to ignore them (e.g. importer plugins can usually be ignored for game builds). Most importantly the plugins segregate the code, ensuring the design of the engine is decoupled and clean. Each plugin is based on an abstract interface defined in the engine layers.

## Render backend ##		
Render backend plugins allow you to use a different backend for performing hardware accelerated rendering. Its interface is provided through @b3d::GpuBackend (device enumeration and debugging/profiling), @b3d::GpuDevice (resource creation), and @b3d::render::GpuCommandBuffer (recording rendering commands like draw calls, state changes, and resource barriers). 

The following plugins all have their own implementations of the @b3d::GpuBackend interface, as well as any related types (e.g. @b3d::GpuBuffer, @b3d::Texture):
 - **bsfVulkanGpuBackend** - Provides a render backend using Vulkan.
 - **bsfNullGpuBackend** - Provides a render backend that perform no operations

## Importers ##		
Importers allow you to convert various types of files into formats easily readable by the engine. Normally importers are only used during development, and the game itself will only use previously imported assets (although ultimately that's up to the user).

All importers implement a relatively simple interface represented by the @b3d::SpecificImporter class. The engine can start with zero importers, or with as many as you need. See the [importer](Developer_Manuals/Resources/customImporters) manual to learn more about importers and how to create your own. Some important importers are provided out of the box:
 - **bsfFreeImgImporter** - Handles import of most popular image formats, like .png, .psd, .jpg, .bmp and similar. It uses the FreeImage library for reading the image files and converting them into engine's @b3d::Texture format.
 - **bsfFBXImporter** - Handles import of FBX mesh files. Uses Autodesk FBX SDK for reading the files and converting them into engine's @b3d::Mesh format.
 - **bsfFontImporter** - Handles import of TTF and OTF font files. Uses FreeType for reading the font files and converting them into engine's @b3d::Font format.
 - **bsfSL** - Provides an implementation of the Banshee shader language that allows you to easily define an entire pipeline state in a single file. Imports .bsl files into engine's @b3d::Shader format.

## Others ##

### bsfPhysX ###
Handles physics: rigidbodies, colliders, triggers, joints, character controller and similar. Implements the @b3d::Physics interface and any related classes (e.g. @b3d::Rigidbody, @b3d::Collider). Uses NVIDIA PhysX as the backend.

### bsfMono ###
Provides access to the C# scripting language using the Mono runtime. This allows the C++ code to call into C# code, and vice versa, as well as providing various meta-data about the managed code, and other functionality. 

### bsfOpenAudio ###
Provides implementation of the audio system using the OpenAL library for audio playback, as well as FLAC, Ogg and Vorbis libraries for audio format import.

### bsfFMOD ###
Provides implementation of the audio system using the FMOD library. Provides audio playback and audio file import for many formats.

### bsfRenderBeast ###			
Framework's default renderer. Implements the @b3d::render::Renderer interface. This plugin might seem similar to the render API plugins mentioned above but it is a higher level system. While render API plugins provide low level access to rendering functionality the renderer handles rendering of all scene objects in a specific manner without requiring the developer to issue draw calls manually. A specific set of options can be configured, both globally and per object that control how an object is rendered, as well as specifying completely custom materials. The renderer handles physically based rendering, HDR, shadows, global illumination and similar features. Renderer-internal types live in the `b3d::render::` namespace.

Architecture									{#architecture}
===============
[TOC]

This manual will explain the architecture of bs::f, to give you a better idea of how everything is structured and where to locate particular systems.

bs::f is implemented throughout many separate libraries. Spreading the engine implementation over different libraries ensures multiple things:
 - Portions of the engine can be easily modified or replaced
 - User can choose which portions of the engine he requires
 - Internals are easier to understand as libraries form a clear architecture between themselves, while ensuring source code isn't all bulked into one big package
 - It ensures a higher quality design by decoupling code and forcing an order of dependencies
 
All the libraries can be separated into two main categories:
 - Foundation - These are the core libraries of the engine. They are designed in layers, where each layer is built on top of the previous layer and provides higher level and more specific functionality than the previous one.
 - Plugins - These are separate, independant, and in most cases optional libraries containing various high level systems. They usually implement some interface that was defined in one of the foundation layers. You are able to design your own plugins that completely replace certain portion of the engine functionality without having to modify the engine itself (e.g. use a new physics library, or a renderer backend)
 
To give you a better idea here is a diagram showing how all the libraries connect. You can use this for reference when we talk about the individual library purposes later on.
![bs::f libraries](ArchitectureSimple.png)  
 
# Foundation #										{#arch_layers}
The foundation layers contain the core of the engine. All the essentials and all the abstract interfaces for plugins belong here. 

The foundation was split into layers to further decouple code. Lower layers do not know about higher layers and low level code never caters to specialized high level code. This makes the design cleaner and forces a certain direction for dependencies.

Lower layers were designed to be more general purpose than higher layers. They provide very general techniques usually usable in various situations, and they attempt to cater to everyone. On the other hand higher layers provide a lot more focused and specialized techniques. This might mean relying on very specific rendering APIs, platforms or plugins but it also means using newer, fancier and maybe not as widely accepted techniques (e.g. some new rendering algorithm or a shader).

Going from the lowest to highest the layers are:
## bsfUtility ## {#arch_utility}
This is the lowest layer of the engine. It is a collection of very decoupled and separate systems that are likely to be used throughout all of the higher layers. Essentially a collection of tools that are in no way tied into a larger whole. Most of the functionality isn't even game engine specific, like providing [file-system access](@ref bs::FileSystem), [file path parsing](@ref bs::Path), [events](@ref bs::Event), [math library](@ref bs::Math), [RTTI system](@ref bs::RTTITypeBase), [threading primitives and managers](@ref bs::ThreadPool), among various others.

## bsfCore ## {#arch_core}
This layer builds upon the utility layer by providing abstract interfaces for most of the engine systems. It is the largest layer in bs::f containing systems like @ref bs::RenderAPI "render API", @ref bs::Resources "resource management", @ref bs::Importer "asset import", @ref bs::Input "input", @ref bs::Physics "physics" and more. Implementations of its interfaces are for the most part implemented as plugins, and not part of the layer itself. The layer tries to be generic and include only functionality that is common, while leaving more specialized functionality for higher layers.

## bsfEngine ## {#arch_engine}			
This layer builds upon the abstraction provided by the core layer and provides actual implementations of the core layer interfaces. Since most of the interfaces are implemented as plugins this layer doesn't contain too much of its own code, but is rather in charge of linking everything together. Aside from linking plugins together it also contains some specialized code, like the @ref bs::GUIManager "GUI" and @ref bs::ScriptManager "script" managers, as well as various other functionality that was not considered generic enough to be included in the core layer.

# Plugins #										{#arch_plugins}
bs::f provides a wide variety of plugins out of the box. The plugins are loaded dynamically and allow you to change engine functionality completely transparently to other systems (e.g. you can choose to load an OpenGL renderer instead of a Vulkan one). Some plugins are completely optional and you can choose to ignore them (e.g. importer plugins can usually be ignored for game builds). Most importantly the plugins segregate the code, ensuring the design of the engine is decoupled and clean. Each plugin is based on an abstract interface implemented in one of the layers (for the most part, bsfCore and %bsfEngine layers).

## Render backend ##							{#arch_rapi}		
Render backend plugins allow you to use a different backend for performing hardware accelerated rendering. Its interface is provided primarily though @ref bs::RenderAPI "RenderAPI", which handles low level rendering, including features like vertex/index buffers, creating rasterizer/depth/blend states, shader programs, render targets, textures, draw calls and similar. 

The following plugins all have their own implementations of the @ref bs::RenderAPI "RenderAPI" interface, as well as any related types (e.g. @ref bs::VertexBuffer "VertexBuffer", @ref bs::IndexBuffer "IndexBuffer"):
 - **bsfD3D11RenderAPI** - Provides a render backend using DirectX 11. 
 - **bsfVulkanRenderAPI** - Provides a render backend using Vulkan. 
 - **bsfGLRenderAPI** - Provides a render backend using OpenGL. 

## Importers ##									{#arch_importers}		
Importers allow you to convert various types of files into formats easily readable by the engine. Normally importers are only used during development, and the game itself will only use previously imported assets (although ultimately that's up to the user).

All importers implement a relatively simple interface represented by the @ref bs::SpecificImporter "SpecificImporter" class. The engine can start with zero importers, or with as many as you need. See the [importer](@ref customImporters) manual to learn more about importers and how to create your own. Some important importers are provided out of the box:
 - **bsfFreeImgImporter** - Handles import of most popular image formats, like .png, .psd, .jpg, .bmp and similar. It uses the FreeImage library for reading the image files and converting them into engine's @ref bs::Texture "Texture" format.
 - **bsfFBXImporter** - Handles import of FBX mesh files. Uses Autodesk FBX SDK for reading the files and converting them into engine's @ref bs::Mesh "Mesh" format.
 - **bsfFontImporter** - Handles import of TTF and OTF font files. Uses FreeType for reading the font files and converting them into engine's @ref bs::Font "Font" format.
 - **bsfSL** - Provides an implementation of the bs::f shader language that allows you to easily define an entire pipeline state in a single file. Imports .bsl files into engine's @ref bs::Shader "Shader" format.

## Others ##									{#arch_others}

### bsfPhysX ###				{#arch_physx}
Handles physics: rigidbodies, colliders, triggers, joints, character controller and similar. Implements the @ref bs::Physics "Physics" interface and any related classes (e.g. @ref bs::Rigidbody "Rigidbody", @ref bs::Collider "Collider"). Uses NVIDIA PhysX as the backend.

### bsfMono ###					{#arch_mono}
Provides access to the C# scripting language using the Mono runtime. This allows the C++ code to call into C# code, and vice versa, as well as providing various meta-data about the managed code, and other functionality. 

### bsfOpenAudio ###	{#arch_oa}
Provides implementation of the audio system using the OpenAL library for audio playback, as well as FLAC, Ogg and Vorbis libraries for audio format import.

### bsfFMOD ### {#arch_fmod}
Provides implementation of the audio system using the FMOD library. Provides audio playback and audio file import for many formats.

### bsfRenderBeast ###		 {#arch_renderBeast}			
bs::f's default renderer. Implements the @ref bs::ct::Renderer "Renderer" interface. This plugin might seem similar to the render API plugins mentioned above but it is a higher level system. While render API plugins provide low level access to rendering functionality the renderer handles rendering of all scene objects in a specific manner without requiring the developer to issue draw calls manually. A specific set of options can be configured, both globally and per object that control how an object is rendered, as well as specifying completely custom materials. e.g. the renderer will handle physically based rendering, HDR, shadows, global illumination and similar features.
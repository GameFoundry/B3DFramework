Architecture									{#architecture}
===============
[TOC]

This manual will explain the architecture of Banshee, to give you a better idea of how everything is structured and where to locate particular systems.

Banshee is implemented throughout many separate libraries. Spreading the engine implementation over different libraries ensures multiple things:
 - Portions of the engine can be easily modified or replaced
 - User can choose which portions of the engine he requires
 - Internals are easier to understand as libraries form a clear architecture between themselves, while ensuring source code isn't all bulked into one big package
 - It ensures a higher quality design by decoupling code and forcing an order of dependencies
 
All the libraries can be separated into four main categories:
 - Layers - These are the core libraries of the engine. Each layer is built on top of the previous layer and provides higher level and more specific functionality than the other.
 - Plugins - These are separate, independant, and in most cases optional libraries containing various high level systems. They always depend on a specific layer, depending how high level the system they implement is. You are able to design your own plugins that completely replace certain portion of the engine functionality without having to modify the engine itself (e.g. use OpenGL instead of DirectX)
 - Scripting - These are libraries used for interop between the C++ core and the scripting language, as well as the managed scripting libraries themselves. Generally these can also be counted as plugins, but they are a bit more tightly coupled with the engine core.
 - Executable - These are small pieces of code meant to initialize and start up the engine/editor.
 
To give you a better idea here is a diagram showing how all the libraries connect:
![Banshee's libraries](ArchitectureSimple.png)  
 
# Layers #										{#arch_layers}
The layers contain the core of the engine. All the essentials and all the abstract interfaces for plugins belong here. The engine core was split into multiple layers for two reasons:
 - To give developers a chance to pick the level of functionality they need. Some people will want just core and utility and start working on their own engine or low level technologies, while others might be just interested in game development and will stick with the editor layer.
 - To decouple code. Lower layers do not know about higher layers and low level code never caters to specialized high level code. This makes the design cleaner and forces a certain direction for dependencies.

Lower layers were designed to be more general purpose than higher layers. They provide very general techniques usually usable in various situations, and they attempt to cater to everyone. On the other hand higher layers provide a lot more focused and specialized techniques. This might mean relying on very specific rendering APIs, platforms or plugins but it also means using newer, fancier and maybe not as widely accepted techniques (e.g. some new rendering algorithm or a shader).

Going from the lowest to highest the layers are:
## BansheeUtility ##					
This is the lowest layer of the engine. It is a collection of very decoupled and separate systems that are likely to be used throughout all of the higher layers. Essentially a collection of tools that are in no way tied into a larger whole. Most of the functionality isn't even game engine specific, like providing [file-system access](@ref bs::FileSystem), [file path parsing](@ref bs::Path), [events](@ref bs::Event), [math library](@ref bs::Math), [RTTI system](@ref bs::RTTITypeBase), [threading primitives and managers](@ref bs::ThreadPool), among various others.

See the [utilities](@ref utilities) manual for an overview of all the most important systems in this layer. 

## BansheeCore ##		
This layer builds upon the utility layer by providing abstract interfaces for most of the engine systems. The interfaces themselves are implemented in the form of plugins, and are not part of the layer itself (for the most part). This layer glues all the engine's systems together and provides a foundation that the engine is built on. The layer tries to be generic and include only functionality that is common, while leaving more specialized functionality for higher layers. Some systems provided by this layer are @ref bs::RenderAPI "render API wrapper", @ref bs::Resources "resource management", @ref bs::Importer "asset import", @ref bs::Input "input", @ref bs::Physics "physics" and more.

## %BansheeEngine ##				
This layer builds upon the abstraction provided by the core layer and provides actual implementations of the core layer interfaces. Since most of the interfaces are implemented as plugins this layer doesn't contain too much of its own code, but is rather in charge of linking everything together. Aside from linking plugins together it also contains some specialized code, like the @ref bs::GUIManager "GUI" and @ref bs::ScriptManager "script" managers, as well as various other functionality that was not considered generic enough to be included in the core layer.

## BansheeEditor ##					
And finally the top layer is the editor. It builts upon everything else so far and provides various editor specific features like the project library, build system, editor window management, scene view tools and similar. Large portions of the editor are implemented in the scripting code, and this layer provides more of a set of helper tools used by the scripting system. If you are going to work with this layer you will also be working closely with the scripting interop code and the scripting code (see below).

# Plugins #										{#arch_plugins}
Banshee provides a wide variety of plugins out of the box. The plugins are loaded dynamically and allow you to change engine functionality completely transparently to other systems (e.g. you can choose to load an OpenGL renderer instead of a DirectX one). Some plugins are completely optional and you can choose to ignore them (e.g. importer plugins can usually be ignored for game builds). Most importantly the plugins segregate the code, ensuring the design of the engine is decoupled and clean. Each plugin is based on an abstract interface implemented in one of the layers (for the most part, BansheeCore and %BansheeEngine layers).

## Render API ##								{#arch_rapi}		
Render API plugins allow you to use a different backend for performing hardware accelerated rendering. @ref bs::RenderAPI "RenderAPI" handles low level rendering, including features like vertex/index buffers, creating rasterizer/depth/blend states, shader programs, render targets, textures, draw calls and similar. See the [render API](@ref renderAPI) manual to learn more about it.

The following plugins all have their own implementations of the @ref bs::RenderAPI "RenderAPI" interface, as well as any related types (e.g. @ref bs::VertexBuffer "VertexBuffer", @ref bs::IndexBuffer "IndexBuffer"):
 - **BansheeD3D11RenderAPI** - Provides implementation of the @ref bs::RenderAPI "RenderAPI" interface for DirectX 11. 
 - **BansheeD3D9RenderAPI**	- Provides implementation of the @ref bs::RenderAPI "RenderAPI" interface for DirectX 9. 
 - **BansheeGLRenderAPI** - Provides implementation of the @ref bs::RenderAPI "RenderAPI" interface for OpenGL 4.3.

## Importers ##									{#arch_importers}		
Importers allow you to convert various types of files into formats easily readable by the engine. Normally importers are only used during development (e.g. in the editor), and the game itself will only use previously imported assets (although ultimately that's up to the user).

All importers implement a relatively simple interface represented by the @ref bs::SpecificImporter "SpecificImporter" class. The engine can start with zero importers, or with as many as you need. See the [importer](@ref customImporters) manual to learn more about importers and how to create your own. Some important importers are provided out of the box:
 - **BansheeFreeImgImporter** - Handles import of most popular image formats, like .png, .psd, .jpg, .bmp and similar. It uses the FreeImage library for reading the image files and converting them into engine's @ref bs::Texture "Texture" format.
 - **BansheeFBXImporter** - Handles import of FBX mesh files. Uses Autodesk FBX SDK for reading the files and converting them into engine's @ref bs::Mesh "Mesh" format.
 - **BansheeFontImporter** - Handles import of TTF and OTF font files. Uses FreeType for reading the font files and converting them into engine's @ref bs::Font "Font" format.
 - **BansheeSL** - Provides an implementation of the Banshee's shader language that allows you to easily define an entire pipeline state in a single file. Imports .bsl files into engine's @ref bs::Shader "Shader" format.

## Others ##									{#arch_others}

### BansheeOISInput ###							{#arch_ois}
Handles raw mouse/keyboard/gamepad input for multiple platforms. All input plugins implement the @ref bs::RawInputHandler "RawInputHandler" interface. Uses the OIS library specifically modified for Banshee (source code available with Banshee's dependencies). 

Be aware that there is also an @ref bs::OSInputHandler "OSInputHandler" that is used for non-game specific purposes (e.g. tracking cursor, text input), but that is part of the engine core instead of a plugin.

### BansheePhysX ###				
Handles physics: rigidbodies, colliders, triggers, joints, character controller and similar. Implements the @ref bs::Physics "Physics" interface and any related classes (e.g. @ref bs::Rigidbody "Rigidbody", @ref bs::Collider "Collider"). Uses NVIDIA PhysX as the backend.

### BansheeMono ###					
Provides access to the C# scripting language using the Mono runtime. This allows the C++ code to call into C# code, and vice versa, as well as providing various meta-data about the managed code, and other functionality. All the script interop libraries (listed below) depend on this plugin. See the [scripting](@ref scripting) manual to learn more about this layer.

### RenderBeast ###					
Banshee's default renderer. Implements the @ref bs::Renderer "Renderer" interface. This plugin might seem similar to the render API plugins mentioned above but it is a higher level system. While render API plugins provide low level access to rendering functionality the renderer handles rendering of all scene objects in a specific manner without requiring the developer to issue draw calls manually. A specific set of options can be configured, both globally and per object that control how an object is rendered, as well as specifying completely custom materials. e.g. the renderer will handle physically based rendering, HDR, shadows, global illumination and similar features.

See the [renderer](@ref renderer) manual to learn more about how the renderer works and how to implement your own.

# Scripting #									{#arch_scripting}
Scripting libraries can be placed into two different categories: 
 - Interop libraries written in C++. These provide glue code between C++ and C# (e.g. converting types, managing objects and similar). These are prefixed with the letter *S*.
 - Managed libraries containing actual managed code. These are prefixed with the letter *M*.
 
Whenever adding a new type that exists in both script and managed code you will need to access both of these library categories. Read the [scripting](@ref scripting) manual to learn about exposing C++ code to scripts.

Interop libraries:
 - **SBansheeEngine** - Provides glue code between %BansheeEngine layer and the scripting code in MBansheeEngine.
 - **SBansheeEditor** - Provides glue code between BansheeEditor layer and the scripting code in MBansheeEditor.

Managed assemblies:
 - **MBansheeEngine**	- Provides most of the high-level C++ engine API available in %BansheeEngine layer (e.g. scene object model, resources, GUI, materials, etc.) as a high-level C# scripting API. Certain portions of the API are provided in pure C# (like the math library) while most just wrap their C++ counterparts.
 - **MBansheeEditor** - Provides all editor specific functionality from the BansheeEditor layer as a C# scripting API (e.g. project library, building, scene view, etc.). Also implements a large number of editor features in pure C# on top of the provided engine and editor APIs (e.g. specific editor windows and tools).
 
# Complete architecture #			{#arch_complete}
See the diagram below to give you a better idea of the full architecture of Banshee. The diagram is not complete, but rather meant as a way to qive you a rough idea of where everything is, to make it easier to navigate the engine.

![Architecture](@ref Architecture.png) 
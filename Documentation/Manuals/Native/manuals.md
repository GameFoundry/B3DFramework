Manuals									{#manuals}
===============

# Quick start
TODO - [Getting started](@ref gettingStarted) - This is a short guide on how to create a fully functional Banshee application, aimed to get you up and running quickly, without going into too much detail.

# User manuals
A complete set of manuals covering all major functionality provided by Banshee, starting with basics and slowly guiding you through more advanced concepts. This should be the primary entry point for new users.
- [Startup and main loop](@ref startup)
- [Scene objects and components](@ref scenesAndComponents)
- **Resources**
 - [Basics and import](@ref resourceBasicsAndImport)
 - [Saving and loading](@ref resourceSavingAndLoading)
- **Utilities**
 - [Containers](@ref containers)
 - [Strings](@ref strings)
 - [Memory allocation](@ref memory)
 - [Smart pointers](@ref smartPointers) 
 - [Events](@ref events)
 - [File system](@ref fileSystem) 
 - [Math utilities](@ref mathUtilities)
 - [Logging messages](@ref logging)
 - [Measuring time](@ref time)
- **Graphics**
 - [Windows](@ref windows)
 - [Cameras](@ref cameras)
 - [Importing textures](@ref importingTextures)
 - [Importing meshes](@ref importingMeshes)
 - [Materials](@ref simpleMaterial)
 - [Rendering objects](@ref renderingObjects)
 - [Lighting](@ref lights)
- **Input**
 - [Input polling](@ref inputPolling) 
 - [Input events](@ref inputEvents) 
 - [Virtual input](@ref virtualInput)
- [Creating custom components](@ref customComponents)
- [Serializing objects](@ref serializingObjects)
- **GUI**
 - [Sprite textures](@ref spriteTextures)
 - [Basic setup](@ref guiSetup)
 - [Elements](@ref guiElements)
 - [Layouts](@ref guiLayouts)
 - [Styles](@ref guiStyles)
 - [Importing fonts](@ref importingFonts)
 - [Localization](@ref guiLocalization)
- **Audio**
 - [Importing audio files](@ref importingAudio)
 - [Playing audio](@ref playingAudio)
- **Physics**
 - [Colliders](@ref colliders)
 - [Physics mesh import](@ref physicsMesh)
 - [Triggers](@ref triggers)
 - [Dynamic objects](@ref rigidbodies)
 - [Physics material](@ref physicsMaterial)
 - [Character controller](@ref characterController)
 - [Scene queries](@ref sceneQueries)
 - [Joints](@ref joints)
- **Animation**
 - [Loading animation clips](@ref animationClip)
 - [Playing animation](@ref animation)
 - [Bones](@ref bones)
 - [Advanced animation](@ref advancedAnimation)
- **Advanced**
 - [Manipulating textures](@ref advancedTextures)
 - [Offscreen rendering](@ref offscreenRendering)
 - [Creating meshes](@ref creatingMeshes)
 - [Advanced startup](@ref advancedStartup)
 - [Advanced RTTI](@ref advancedRtti)
 - [Prefabs](@ref prefabs)
 - [Cursors](@ref cursors)
 - [Profiling](@ref cpuProfiling)
 
# Developer manuals
A set of manuals covering advanced functionality intented for those wanting to extend the engine or tinker with the internals, rather than for normal users. You are expected to have read the user manuals first.
- [Architecture overview](@ref architecture) 
- **Low level rendering API**
 - [Core thread](@ref coreThread)
 - [GPU programs](@ref gpuPrograms)
 - [Non-programmable states](@ref nonProgrammableStates)
 - [Geometry](@ref geometry)
 - [Render targets](@ref renderTargets)
 - [Drawing](@ref drawing) 
 - [Load-store textures](@ref loadStoreTextures)
 - [Generic buffers](@ref genericBuffers)
 - [Compute](@ref compute)
 - [Command buffers](@ref commandBuffers)

## General guides
Name                                      | Description
------------------------------------------|-------------
[Utilities](@ref utilities)               | Provides an overview of a variety of utility systems used throughout Banshee.
[Resources](@ref resources)  			  | Explains how resources work, including saving, loading and creating brand new resource types.
[Scripting](@ref scripting)               | Shows you how to interact with the scripting system, and how to expose C++ objects to the scripting API.
[Renderer](@ref renderer)    	  		  | Explains how the renderer works on the low level, and how to create a custom renderer so you may fully customize the look of your application.
[BSLFX](@ref bslfx)    	  		  		  | Provides a reference for the Banshee Shading Language FX syntax.
[Custom GUI elements](@ref customGUI)     | Shows you how to create custom GUI elements, manually render text or modify GUI system in a general way.
[Custom importers](@ref customImporters)  | Shows you how to create importers that handle conversion of third party resources into engine ready formats.
[Custom plugins](@ref customPlugins)      | Shows you how to create custom plugins that can be loaded by Banshee.
[Quick reference](@ref quickref)          | Provides a few bits of commonly required information, that might be hard to remember otherwise.
[Porting](@ref porting)                   | Information about how to go on about porting Banshee to a different operating system.
[Code style](@ref style)                  | Information about code style used when writing Banshee.

## Low level render API

Name                                      | Description
------------------------------------------|-------------
[Textures](@ref textures)                 | Shows you how to create, use and manipulate textures.
[Meshes](@ref meshes)                     | Shows you how to create, use and manipulate meshes.
[Materials](@ref materials)				  | Shows you how to create and use materials and shaders.
[Render API](@ref renderAPI)
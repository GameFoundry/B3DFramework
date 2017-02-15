Manuals									{#manuals}
===============
[TOC]

# User guides

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
 
# Developer guides

Here you will find a list of all manuals relating to Banshee's native code, primarily focusing on explaining the engine internals instead of the user-facing code. The manuals are roughly ordered in the way they should be read, although if you are interested in a specific topic feel free to skip ahead as manuals often contain cross references to prerequisite manuals.

## General guides
Name                                      | Description
------------------------------------------|-------------
[Getting started](@ref gettingStarted)    | Shows how to perform some basic operations using Banshee's user-facing interface. Allows you to get a general idea of how Banshee works.
[Architecture](@ref architecture)         | Gives you an overview of the entire architecture of Banshee. Useful starting point for those modifying engine internals as it provides a way to identify major systems.
[Utilities](@ref utilities)               | Provides an overview of a variety of utility systems used throughout Banshee.
[Core thread](@ref coreThread)            | Explains how core (rendering) thread works, how it interacts with simulation thread, what are core objects and how to create your own.
[Resources](@ref resources)  			  | Explains how resources work, including saving, loading and creating brand new resource types.
[RTTI](@ref rtti)                         | Shows you how to add run-time type information for your objects.
[Game objects](@ref gameObjects)          | Explains what are scene objects and components and how can you use them to create your scene.
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
[Render API](@ref renderAPI)              | Explains how to use the render API to use the graphics pipeline and draw objects. 
[Textures](@ref textures)                 | Shows you how to create, use and manipulate textures.
[Meshes](@ref meshes)                     | Shows you how to create, use and manipulate meshes.
[Render targets](@ref renderTargets)	  | Shows you how to create and use render textures and windows.
[GPU programs](@ref gpuPrograms)		  | Shows you how to create and use GPU programs.
[Materials](@ref materials)				  | Shows you how to create and use materials and shaders.
# Features

All features listed here are the ones currently available (implemented). If you are interested in what's coming up in the near future check out our [roadmap](roadmap.md).
  
---------------------------------------------------  
  
## Core
* __General__
  * Modern C++14 code
  * Clean layered design
  * Modular & plugin based
  * Fully documented
  * Platforms: Windows, Linux
* __Renderer__
  * Vulkan, DX11 and OpenGL 4.5 render backends
  * Multi-threaded rendering
  * Node based compositor
  * Modern high fidelity graphics
    * Fast hybrid tiled deferred / clustered forward renderer
    * Physically based shading
    * Area light sources
    * Reflection probes with geometry proxies
    * Indirect illumination via irradiance maps
    * Screen space reflections
    * HDR rendering
	  * Automatic eye adaptation
	  * Tonemapping with adjustable curve
	  * White balance
	  * Color grading
    * Gamma correct rendering	
    * MSAA (both forward and deferred)
    * Shadows
      * Percentage closer shadows
      * Cascaded shadow maps
      * Fully dynamic
    * Post processing effects
	  * Screen space ambient occlusion (SSAO)
	  * Depth of field
	  * Fast approximate anti-aliasing (FXAA)
  * Extensive material & shader system
    * Custom high level shading language (BSL)
	  * Unified shader code for all render backends
	  * HLSL-based with high level extensions
	  * Complete material definitions in a single file
	  * High level concepts like mixins and overloads
    * Low level shader parsing and reflection for HLSL and GLSL
  * Low-level rendering API
	* Fully featured, backend agnostic wrapper for Vulkan/DX/OpenGL
	* Full support for all SM5 features, including:
	   * Geometry, tesselation and compute shaders
	   * Instanced rendering
	   * Texture arrays and GPU buffers
	   * Unordered access textures and buffers
	* Designed to support the next generation of render backends (i.e. Vulkan), including features:
	   * Pipeline state objects
	   * Command lists
	   * GPU queues (async compute and upload)
	   * Explicit multi-GPU support
  * Fully extensible
    * Plugin based render backends
	* Plugin based renderer
	* Support for renderer extensions
* __Asset pipeline__
  * Asynchronous resource loading
  * Extensible importer system
  * Available importer plugins for:
    * FBX, OBJ, DAE meshes
    * PNG, PSD, BMP, JPG, etc. images
    * OTF, TTF fonts
	* OGG, FLAC, WAV, MP3 sounds
    * HLSL, GLSL shaders
* __GUI system__
  * Unicode text rendering and input
  * Easy to use layout based system
  * Many common GUI controls
  * Fully skinnable
  * Automatic batching for fast rendering
  * Supports texture atlases
  * Supports arbitrary 3D transformations
  * Localization support (string tables)
* __Animation__
  * Skeletal animation with skinning
    * 1D and 2D animation blending
    * Additive animation support
    * Animation events
    * Root bone animation
    * Animation sockets for animating in-game objects
    * Post-processing hooks for IK support
  * Blend shape animation
  * Generic property animation
    * Animate any script property
    * Built-in animation curve editor
  * Multi-threaded and GPU accelerated
* __Input__
  * Mouse/keyboard/gamepad support
  * Provides both raw and OS input
  * Virtual input with built-in key mapping
  * Virtual axes for analog input devices
* __Physics__
  * Implemented using NVIDIA PhysX
  * Multi-threaded for best performance
  * Abstract plugin interface extensible for other physics implementations (e.g. Havok, Bullet)
  * Supported features
    * Colliders (Box, Sphere, Capsule, Mesh)
    * Triggers
    * Rigidbody
    * Character controller
    * Joints (Fixed, Distance, Hinge, Spherical, Slider, D6)
	* Scene queries
	* Collision filtering
	* Discrete or continuous collision detection
* __Scripting__
  * C# 7.0
  * Separate high level engine API
  * Integrated runtime for maximum performance
  * Full access to .NET framework
  * Integration with Visual Studio, MonoDevelop
  * Automatic serialization
	* Works with custom components, resources or arbitrary types
	* Save/load data with no additional code
	* Handles complex types (e.g. array, list, dictionary) and references
	* Fast and small memory footprint
* __Audio__
  * 3D sounds (panning, attenuation, doppler effect) and 2D sounds (music, narration)
  * On-the-fly streaming and decompression
  * Multi-channel support up to 7.1 sound
  * Multiple listener support for split-screen
  * Multiple backends
    * OpenAL
	* FMOD
	* Extensible to others
* __Other__
  * CPU & GPU profiler
  * Advanced run-time type information for C++ code
    * Iterate over class fields, safely cast objects, clone objects, detect base types
	* Find references to specific objects (e.g. all resources used in a scene)
	* Serialize/deserialize with no additional code and with automatic versioning
	* Generate diffs
  * Vector (SIMD) instruction API
    * Compiles transparently to all popular instruction sets
    * SSE4.1, AVX, AVX2, AVX512 NEON, NEONv2 and others	
  * Utility library
    * Math
	* File system
    * Events
	* Thread pool
    * Task scheduler
    * Logging
	* Debug drawing
	* Crash reporting
	* Unit testing
	* Custom memory allocators

---------------------------------------------------	
	
## Editor
* __Asset management__
  * Simple drag and drop import for many popular formats
  * Automatic reimport of externally modified assets (e.g. modify a shader, see changes in editor right away)
  * Asset modifications immediately reflected in-game (resource hot-swap)
  * Version control friendly format
* __Powerful object inspector__
  * Exposes script object properties for artists/designers
  * Automatically generated GUI for custom classes
  * Customize visible elements via attributes or create GUI manually
* __Level creation__
  * Simple drag and drop interface
  * Traditional set of tools (Move/Scale/Rotate/Select, etc.)
  * Interface for creating custom 2D and 3D tools
* __Prefab system__
  * Save parts or entire levels as prefabs so they may be re-used later
  * Separate larger levels into smaller prefabs for easier loading
  * Reduce conflicts when multiple people are working on the same level
  * Customize individual prefab instances without breaking the prefab link
  * Supports nesting and complex hierarchies to ensure maintaining complex levels is easy
* __Play in editor__
  * Compile all scripts within editor
  * Scripts and data transparently reloaded after compilation so changes may be tested immediately
  * Pause and frame-step to better test and debug your game
  * Analyze and modify scene while playing
* __Fully extensible__
  * Specialized scripting API only for editor extensions
  * Easy to use without needing to know about engine internals
  * Extend almost anything. Create:
	* Custom editor windows
	* Custom object inspectors
	* Custom 2D/3D tools
	* Code for automating common tasks
* __Game publishing__
  * Build a game ready for distribution from within the editor
  * One click build process, just choose a platform and build
  * Automatically detects only the required resources to minimize build size
  * Automatically packages and outputs an executable
* __Customizable frontend__
  * Dockable layout and floating windows
  * Custom GUI skin & localization support

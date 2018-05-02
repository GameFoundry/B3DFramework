# Features

* __General__
	* Modern C++14 code
	* Clean layered design
	* Modular & plugin based
	* Fully documented
	* Platforms: Windows, Linux, macOS
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
		* Shader bytecode caching
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
	* Built-in support for third party formats:
		* FBX, OBJ, DAE meshes
		* PNG, PSD, BMP, JPG, etc. images
		* OTF, TTF fonts
		* OGG, FLAC, WAV, MP3 sounds
		* HLSL, GLSL shaders
	* Asynchronous resource loading
	* Asynchronous resource import
	* Resource compression
	* Extensible importer system
* __GUI system__
	* All common GUI controls
		* Text
		* Image
		* Button
		* Input box
		* Drop down
		* Slider
		* Checkbox
		* Scroll area
		* And support for custom controls
	* Unicode text rendering and input
	* Easy to use layout based system
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

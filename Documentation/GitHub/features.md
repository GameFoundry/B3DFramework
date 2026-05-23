# Features

* __General__
	* Modern C++17 code
	* Clean layered design
	* Modular & plugin based
	* Fully documented
	* Platforms: Windows, Linux, macOS
* __Scene__
	* Component-based scene graph
	* Multi-scene support with per-scene systems
	* ECS-backed hierarchy for fast traversal and transform updates
	* Prefab system with UUID-based linking and correct nested-prefab handling
	* Transform system for batch scene transform updates
	* Type-safe handles for game objects and components
* __Entity Component System__
	* High-performance ECS backing the scene and renderer
	* Scales to massive object counts via sparse-set component storage
	* Runtime views with batched iteration
	* Serializable ECS fragments with RTTI integration
	* Tag groups for fast filtering
* __Renderer__
	* Vulkan render backend
	* Multi-threaded rendering with a dedicated render thread
	* Batch render-proxy sync for fast render-thread updates
	* Multi-scene rendering 
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
		* Deferred decals
	* Post processing effects
		* Screen space ambient occlusion (SSAO)
		* Gaussian depth-of-field
		* Bokeh depth-of-field
		* Bloom
		* Lens flare
		* Fast approximate anti-aliasing (FXAA)
		* Film grain
		* Chromatic aberration
		* Temporal anti-aliasing (TAA)
	* Extensive material & shader system
		* Custom high level shading language (BSL)
			* Unified shader code for all render backends
			* HLSL-based with high level extensions
			* Complete material definitions in a single file
			* High level concepts like mixins and overloads
		* On-demand shader compilation
		* Persistent shader cache
		* SPIRV-Cross based uniform reflection preserved across shader variations
		* Low level shader parsing and reflection for HLSL and GLSL
	* Fully extensible
		* Plugin based render backends
		* Plugin based renderer
		* Per-scene renderer extensions
* __Low-level GPU API__
	* Fully featured, thread safe, backend-agnostic wrapper
	* Command buffers with explicit render passes
	* Automatic barriers — no manual layout transitions required
	* GPU buffer pooling (permanent and transient) with frame-lifetime allocations
	* Descriptor / parameter set pools with automatic multi-pool allocation
	* Persistent memory-mapped regions for efficient CPU writes
	* Separate submit thread for command buffer submission and presentation
	* GPU queues (async compute and upload)
	* Full support for all SM5 features (geometry and compute shaders, instanced rendering, texture arrays, unordered access textures and buffers)
	* Debug naming for all GPU objects and RenderDoc integration
	* GPU allocators (Two level segregated fit with defragment support & linear for transient allocations)
* __Asset pipeline__
	* Built-in support for third party formats:
		* FBX, OBJ, DAE meshes
		* PNG, PSD, BMP, JPG, etc. images
		* OTF, TTF fonts
		* OGG, FLAC, WAV, MP3 sounds
		* HLSL, GLSL shaders
	* Asynchronous resource loading and import
	* Resource compression
	* Extensible importer system
	* Package-based resource storage, with support for multiple resources bundled per package
	* Thread-safe package operations (load, save, import, reimport, copy, move, delete)
	* Virtual file paths
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
	* CSS-like style sheet engine
		* Class selectors and descendant combinators
		* Pseudo-classes (hover, active, focus) and pseudo-elements
		* Full box model (padding, margin, per-side border, border-radius)
		* CSS variables and specificity-based rule sorting
		* Multiple style sheets with importance ordering
	* Vector graphics rendering
		* GPU-accelerated fill and stroke
		* Scale-9-grid automatic scaling for backgrounds
		* Dynamic texture atlas with tree-based layout
	* Stock icons using FontAwesome with metadata
	* Dynamic fonts with sub-pixel precision rendering
	* Unicode text rendering and input
	* HDPI support with logical / physical unit distinction
	* GUI element culling and scaling
	* Easy to use layout based system
	* Automatic batching for fast rendering
	* Supports arbitrary 3D transformations
	* Localization support (string tables)
	* Minimal redraw (only modified regions redraw, static GUI -> not even swap happens)
* __Animation__
	* Skeletal animation with skinning
		* 1D and 2D animation blending
		* Additive animation support
		* Animation events
		* Root bone animation
		* Animation sockets for animating in-game objects
		* Post-processing hooks for IK support
	* Blend shape animation
	* Multi-threaded and GPU accelerated
	* Per-scene animation simulation
* __Input__
	* Mouse / keyboard / gamepad support
	* Provides both raw and OS input
	* Virtual input with built-in key mapping
	* Virtual axes for analog input devices
* __Physics__
	* Implemented using NVIDIA PhysX
	* Multi-threaded for best performance
	* Per-scene simulation
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
* __Particles__
	* Dual system
		* Multi-threaded CPU simulation
		* Hardware accelerated GPU simulation
	* Distribution based properties
		* Constant
		* Random range
		* Curve
		* Random curve range
	* In-depth emission rules
		* Primitive emitters
			* Box, Sphere, Cone, Rectangle, Circle, Line
			* With customizable properties and emission modes
		* Static mesh emitter
		* Animated (skinned) mesh emitter
		* Continous or burst emission
		* Variety of tweakable initial particle properties
	* Customizable CPU evolver design
		* Modify particle properties over their lifetime
		* Variety of built-in evolvers provided
		* Extensible design to create your own
	* Particle collisions
		* World collisions with physics objects
		* Plane collisions with user defined planes
		* Depth buffer based GPU-only collisions
	* Texture animation
	* Particle sorting
	* Billboard or 3D particles
	* Soft particle rendering
	* Vector field import and simulation
* __Scripting__
	* C# scripting running on a Mono / .NET Core runtime
	* Separate high level engine API
	* Integrated runtime for maximum performance
	* Full access to the .NET framework
	* Integration with Visual Studio
	* Automatic codegen-driven script bindings
	* Garbage collection support for script-exported C++ objects
	* Managed assembly hot-reload
* __Audio__
	* 3D sounds (panning, attenuation, doppler effect) and 2D sounds (music, narration)
	* On-the-fly streaming and decompression
	* Multi-channel support up to 7.1 sound
	* Multiple listener support for split-screen
	* Multiple backends
		* OpenAL
		* FMOD
		* Extensible to others
* __Networking__
	* Built on GameNetworkingSockets
	* Reliable and unreliable transport
* __RTTI & serialization__
	* Advanced run-time type information for C++ code
		* Iterator-based field system with generic container iteration
		* Tuple fields for structured multi-value data
		* Safely cast objects, clone objects, detect base types
		* Find references to specific objects (e.g. all resources used in a scene)
	* Full-featured serialization
		* Binary and intermediate formats
		* Automatic versioning with no additional code
		* Binary delta generation with nested and map-aware support
		* Works with custom components, resources, ECS fragments or arbitrary types
		* Handles complex types (e.g. array, list, dictionary) and references
		* Fast and small memory footprint, with specialized compresson mode for networked transfers
* __Scheduler & threading__
	* Fiber-based task scheduler built on the Marl library
	* Fiber-aware synchronization primitives (Signal, ConditionVariable, wait groups)
	* Yieldable blocking calls that free worker threads instead of stalling them
	* Lock-free single-consumer queues with fiber yield support
	* Tickets for sequential execution with fiber yielding
* __Testing__
	* Unified unit test runner
	* Snapshot tests for full application runs, capturing screen output
	* Headless rendering mode for automated testing
	* Console and JSON test reporting, with test data collection separate from reporting
	* Scoped expected / suppressed warnings for precise failure filtering
	* BansheeForge CI system for automated build and test execution
* __Code generation__
	* BansheeCodeGenerator — Clang / LLVM based C++ parser
	* Automated script binding generation on both C++ and C# code
	* JavaDoc-style documentation parsing, integrated with BansheeDocumentationGenerator for full API reference generation
	* Script export attribute for marking exported classes, methods, fields and enums
* __Build & tooling__
	* CMake-based build with automated source globbing
	* Automated dependency package download and versioning
	* Command line configuration for executables
* __Other__
	* CPU & GPU profiler
	* Vector (SIMD) instruction API
		* Compiles transparently to all popular instruction sets
		* SSE4.1, AVX, AVX2, AVX512, NEON, NEONv2 and others
	* Utility library
		* Math
		* File system
		* Events
		* Thread pool
		* Logging with category support
		* Debug drawing
		* Crash reporting
		* Custom memory allocators, including inline array allocator, stack-based and frame allocators
		* Command line argument parsing
		* Config variable system for runtime configuration
		* Quad/OctTree with SIMD

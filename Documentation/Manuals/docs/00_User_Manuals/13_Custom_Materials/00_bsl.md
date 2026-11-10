---
title: BSL syntax
---

All shaders in the framework are written in BSL (Banshee Shading Language). The core of the language is based on HLSL (High Level Shading Language), with various extensions to make development easier. In this manual we will not cover HLSL syntax, nor talk about shaders in general, and will instead focus on the functionality specific to BSL. If you are not familiar with the concept of a shader, or HLSL syntax, it is suggested you learn about them before continuing.

# Basics

A simple BSL program that renders a mesh all in white looks like this:
~~~~~~~~~~~~~~
shader MyShader
{
	code
	{
		struct VStoFS
		{
			float4 position : SV_Position;
		};
		
		cbuffer PerObject
		{
			float4x4 gMatWorldViewProj;
		}				
		
		struct VertexInput
		{
			float3 position : POSITION;
		};
		
		VStoFS vsmain(VertexInput input)
		{
			VStoFS output;
		
			output.position = mul(gMatWorldViewProj, input.position);			
			return output;
		}

		float4 fsmain(in VStoFS input) : SV_Target0
		{
			return float4(1.0f, 1.0f, 1.0f, 1.0f); 
		}	
	};
};
~~~~~~~~~~~~~~

As you can see, aside from the **shader** and **code** blocks, the shader code looks identical to HLSL. Inside the **shader** block you can use a variety of options, but the minimum required is the **code** block, which allows you to specify programmable shader code. Inside the **code** block you get to use the full power of HLSL, as if you were using it directly.

There are a few restrictions compared to normal HLSL that you must be aware of:
 - All primitive (non-object) shader constants (uniforms in GLSL lingo) must be part of a **cbuffer**. Primitive types are any types that are not textures, buffers or samplers.
 - Entry points for specific shader stages must have specific names. In the example above **vsmain** serves as the entry point for the vertex shader, while **fsmain** serves as the entry point for the fragment (pixel) shader. The full list of entry point names per type:
  - **vsmain** - Vertex shader
  - **gsmain** - Geometry shader
  - **hsmain** - Hull (Tesselation control) shader
  - **dsmain** - Domain (Tesselation evaluation) shader
  - **fsmain** - Fragment (Pixel) shader
  - **csmain** - Compute shader
 - Ray tracing entry points are the exception to these names. They are functions marked with the `[shader("...")]` attribute, under any name, and they can't be in the same pass as the entry points listed above. See [Ray tracing](#ray-tracing).
 - The HLSL `RayQuery` type can't be used. The **RayTraceClosest** and **RayTraceAny** built-in functions replace it.
  
Let's now move onto more advanced functionality specific to BSL.

# Non-programmable states
Aside from the **code** block, a **shader** can also specify five blocks that allow it to control non-programmable parts of the pipeline:
 - **raster** - Allows you to set options related to rasterization, like rasterization mode (fill/wireframe), cull mode, etc.
 - **depth** - Allows you to set options related to depth buffer and depth comparison, like enabling/disabling depth reads or writes, or changing the depth comparison function
 - **stencil** - Allows you to set options related to the stencil buffer and stencil test, like enabling stencil test and setting the test operations
 - **blend** - Allows you to set blending options, like enabling/disabling blending, setting blend operations or setting color the write mask
 - **output** - Allows you to declare the pixel formats of the render targets the shader writes to, required by backends that compile the output format into the fragment program

An example shader using a variety of these blocks is shown:
~~~~~~~~~~~~~~
shader MyShader
{
	// No depth reads or writes
	depth
	{
		read = false;
		write = false;
	};
	
	// Enable blending on the first render target
	blend
	{
		target	
		{
			enabled = true;
			color = { srcA, srcIA, add };
			writemask = RGB;
		};
	};
	
	// Disable culling
	raster
	{
		cull = none;
	};
	
	// Only render if stencil test passes
	stencil
	{
		enabled = true;
		front = { keep, keep, keep, lte };
		reference = 1;
	};

	code
	{
		// ... some shader code
	};
};
~~~~~~~~~~~~~~

Let's cover all available options for each block type. 

## raster

Name                 | Valid values				   | Reference
---------------------|---------------------------- |----------
fill    	  	     | wire, solid (See @b3d::PolygonMode)			   | @b3d::RASTERIZER_STATE_DESC::polygonMode
cull    	  	     | cw, ccw, none (See @b3d::CullingMode)			   | @b3d::RASTERIZER_STATE_DESC::cullMode
scissor				 | true, false				   | @b3d::RASTERIZER_STATE_DESC::scissorEnable
multisample			 | true, false				   | @b3d::RASTERIZER_STATE_DESC::multisampleEnable
lineaa				 | true, false				   | @b3d::RASTERIZER_STATE_DESC::antialiasedLineEnable

## depth
Name                 | Valid values				   | Reference
---------------------|---------------------------- |----------
read    	  	     | true, false				   | @b3d::DEPTH_STENCIL_STATE_DESC::depthReadEnable
write    	  	     | true, false				   | @b3d::DEPTH_STENCIL_STATE_DESC::depthWriteEnable
compare    	  	     | never, always, lt, lte, eq, neq, gte, gt (See @b3d::CompareFunction)				   | @b3d::DEPTH_STENCIL_STATE_DESC::depthComparisonFunc
bias    	  	     | float				   | @b3d::RASTERIZER_STATE_DESC::depthBias
scaledBias    	  	 | float				   | @b3d::RASTERIZER_STATE_DESC::slopeScaledDepthBias
clip    	  	     | true, false				   | @b3d::RASTERIZER_STATE_DESC::depthClipEnable

## stencil
Name                 | Valid values				   | Reference
---------------------|---------------------------- |----------
reference    	  	 | integer			           | Reference value to use for stencil compare operations.
enabled    	  	     | true, false				   | @b3d::DEPTH_STENCIL_STATE_DESC::stencilEnable
readmask    	  	 | integer in [0, 255] range   | @b3d::DEPTH_STENCIL_STATE_DESC::stencilReadMask
writemask    	  	 | integer in [0, 255] range   | @b3d::DEPTH_STENCIL_STATE_DESC::stencilWriteMask
front				 | StencilOp block			   | Stencil operations and compare function for front facing geometry
back				 | StencilOp block			   | Stencil operations and compare function for back facing geometry
 
**front** and **back** options are blocks themselves, and they accept the following options:
Name                 | Valid values				   | Reference
---------------------|---------------------------- |----------
fail    	  	 	| keep, zero, replace, inc, dec, incwrap, decwrap, inverse (See @b3d::StencilOperation)			           | @b3d::DEPTH_STENCIL_STATE_DESC::frontStencilFailOp & @b3d::DEPTH_STENCIL_STATE_DESC::backStencilFailOp
zfail    	  	 	| keep, zero, replace, inc, dec, incwrap, decwrap, inverse (See @b3d::StencilOperation)			           | @b3d::DEPTH_STENCIL_STATE_DESC::frontStencilZFailOp & @b3d::DEPTH_STENCIL_STATE_DESC::backStencilZFailOp
pass    	  	 	| keep, zero, replace, inc, dec, incwrap, decwrap, inverse (See @b3d::StencilOperation)			           | @b3d::DEPTH_STENCIL_STATE_DESC::frontStencilPassOp & @b3d::DEPTH_STENCIL_STATE_DESC::backStencilPassOp
compare				| never, always, lt, lte, eq, neq, gte, gt (See @b3d::CompareFunction) | @b3d::DEPTH_STENCIL_STATE_DESC::frontStencilComparisonFunc & @b3d::DEPTH_STENCIL_STATE_DESC::backStencilComparisonFunc

An example of a stencil block:
~~~~~~~~~~~~~~
shader MyShader
{
	// Only render if stencil test passes
	stencil
	{
		enabled = true;
		front = { 
			fail = keep; // On stencil test fail don't do anything
			zfail = keep; // On depth test fail don't do anything
			pass = inc; // On stencil test pass, increment stencil buffer by one
			compare = always; // Always pass the stencil test
		};
	};
};
~~~~~~~~~~~~~~

**StencilOp** block can also be declared succintly on a single line. Same code as above, written differently:
~~~~~~~~~~~~~~
shader MyShader
{
	// Only render if stencil test passes
	stencil
	{
		enabled = true;
		front = { keep, keep, inc, always }; // Note the order is always: fail op., zfail op., pass op., compare function
	};
};
~~~~~~~~~~~~~~

## blend
Name                 | Valid values				   | Reference
---------------------|---------------------------- |----------
dither    	  	 | true, false			           | @b3d::BLEND_STATE_DESC::alphaToCoverageEnable
independant    	 | true, false			           | @b3d::BLEND_STATE_DESC::independantBlendEnable
target			 | Target block					   | Blend operations for a specific render target. Multiple Target blocks can exist under a single blend block.

**Target** block accepts the following options:
Name                 | Valid values				   | Reference
---------------------|---------------------------- |----------
index    	  	     | positive integer		       | Index of the render target these options are applied to. If not specified the index is derived from the order in which Target blocks are defined.
enabled				 | true, false				   | @b3d::RENDER_TARGET_BLEND_STATE_DESC::blendEnable
writemask		     | R, G, B, A or any combination (e.g. RG, RBA, RGBA). "empty" for zero mask.					   | @b3d::RENDER_TARGET_BLEND_STATE_DESC::renderTargetWriteMask
color    	  	     | BlendOp block	       	   | Represents the blend operation to execute on the color channels.
alpha				 | BlendOp block			   | Represents the blend operation to execute on the alpha channel. Only relevant if independant blend is enabled.

**BlendOp** block accepts the following options:
Name                 | Valid values				   | Reference
---------------------|---------------------------- |----------
source    	  	     | one, zero, dstRGB, srcRGB, dstIRGB, srcIRGB, dstA, srcA, dstIA, srcIA (See @b3d::BlendFactor)		       | @b3d::RENDER_TARGET_BLEND_STATE_DESC::srcBlend, @b3d::RENDER_TARGET_BLEND_STATE_DESC::srcBlendAlpha
dest    	  	     | one, zero, dstRGB, srcRGB, dstIRGB, srcIRGB, dstA, srcA, dstIA, srcIA (See @b3d::BlendFactor)		       | @b3d::RENDER_TARGET_BLEND_STATE_DESC::dstBlend, @b3d::RENDER_TARGET_BLEND_STATE_DESC::dstBlendAlpha
op    	  	     | add, sub, rsub, min, max (See @b3d::BlendOperation)		       | @b3d::RENDER_TARGET_BLEND_STATE_DESC::blendOp, @b3d::RENDER_TARGET_BLEND_STATE_DESC::blendOpAlpha

An example of a **blend** block:
~~~~~~~~~~~~~~
shader MyShader
{
	blend
	{
		target	
		{
			enabled = true;
			color = { 
				source = srcA;
				dest = srcIA;
				op = add;
			};
			writemask = RGB;
		};
	};
};
~~~~~~~~~~~~~~

**BlendOp** block can also be defined succintly on a single line. Same code as above, written differently:
~~~~~~~~~~~~~~
shader MyShader
{
	blend
	{
		target	
		{
			enabled = true;
			color = { srcA, srcIA, add }; // Note the order is always: source, destination, operation
			writemask = RGB;
		};
	};
};
~~~~~~~~~~~~~~

## output
Declares the pixel format that the shader exports. This is different from the render target format, but the two must at least partially match (see below). This is only relevant for backends that bake the export format into the shader - other backends (e.g. Vulkan/D3D12/Metal) ignore this value.

Compatibility between export and render target formats:
 - 8-bit signed/unsigned normalized formats, 16-bit float format: Default shader export value is RGBA16F, which hardware casts correctly to these formats, no need to specify format
 - 16-bit signed/unsigned normalized formats. Default of RGBA16F works, but you will get more precision if you use RGBA16 or RGBA16S
 - 8-bit or 16-bit integer formats: Specify RGBA16I or RGBA16U (hardware casts between 16 and 8-bit)
 - 32-bit formats (signed/unsigned/integer/float): Specify R32F, RG32F or RGBA32F depending on channel count (only the bit count matters, actual float/unsigned/signed/etc. doesn't)

As a rule of thumb, you always need to specify this value if writing 32-bit output. For most other common outputs it can be left at the default value.

Name                 | Valid values				   | Reference
---------------------|---------------------------- |----------
target			 | Target block					   | Format of a specific render target. Multiple Target blocks can exist under a single output block.

**Target** block accepts the following options:
Name                 | Valid values				   | Reference
---------------------|---------------------------- |----------
index    	  	     | positive integer		       | Index of the render target the format applies to. If not specified the index is derived from the order in which Target blocks are defined.
format				 | Pixel format name without the PF_ prefix (e.g. RGBA32F, R32F, RG32F, R16U)	| @b3d::PixelFormat

An example of an **output** block:
~~~~~~~~~~~~~~
shader MyShader
{
	output
	{
		target
		{
			index = 0;
			format = RGBA32F;
		};
	};

	code
	{
		// ...
		float4 fsmain(VStoFS input) : SV_Target0
		{
			// Full 32-bit precision output
		}
	};
};
~~~~~~~~~~~~~~

# Mixins
When writing complex shaders is it is often useful to break them up into components. This is where the concept of a **mixin** comes in. Any shader code or programmable states defined in a **mixin** can be included in any **shader**, or in another **mixin**. Syntax within a **mixin** block is identical to syntax in a **shader** block, meaning you can define code and non-programmable state blocks as shown above.

~~~~~~~~~~~~~~
// Provides common functionality that might be useful for many different shaders
mixin MyMixin
{
	code
	{
		Texture2D gGBufferAlbedoTex;
		Texture2D gDepthBufferTex;
		
		struct SurfaceData 
		{
			float4 albedo;
			float depth;
		};
		
		SurfaceData getGBufferData(uint2 pixelPos)
		{
			SurfaceData output;
			output.albedo = gGBufferAlbedoTex.Load(int3(pixelPos, 0));
			output.depth = gDepthBufferTex.Load(int3(pixelPos, 0)).r;
		
			return output;
		}			
	};
};
~~~~~~~~~~~~~~

When a shader wishes to use a mixin, simply add it to the shader using the same **mixin** keyword, followed by its name.
~~~~~~~~~~~~~~
shader MyShader
{
	mixin MyMixin;

	code
	{
		// ...
	
		float4 fsmain(in VStoFS input) : SV_Target0
		{
			uint2 pixelPos = ...;
			
			// We can now call methods from the mixin
			SurfaceData surfaceData = getGBufferData(pixelPos);
		
			return surfaceData.albedo; 
		}	
	};
};
~~~~~~~~~~~~~~

Included mixins will append their shader code and states to the shader they are included in. If mixin and shader define the same states, the value of the states present on the shader will be used. If multiple included mixins use the same state then the state from the last included mixin will be used. Code is included in the order in which mixins are defined, followed by code from the shader itself.

Often you will want to define mixins in separate files. BSL files are normally stored with the ".bsl" extension, but when writing include files you should use the ".bslinc" extension instead, in order to prevent the system trying to compile the shader code on its own.

In order to include other files in BSL, use the \#include command. The paths are relative to the working folder, or if you are working in Banshee 3D editor, the paths are relative to your project folder. You can use the special variables $ENGINE$ and $EDITOR$ to access paths to the builtin engine and editor shaders.

~~~~~~~~~~~~~~
// Include the code for accessing the GBuffer
#include "$ENGINE$/GBufferInput.bslinc"

shader MyShader
{
	mixin GBufferInput;

	// ...
};
~~~~~~~~~~~~~~

## Mixin overrides
Mixins can override each other if another mixin is defined with the same name. The last defined mixin is considered the override and will be used in the shader. Shaders can also reference mixins that haven't been declared yet. This concept of mixin overrides allows you to change functionality of complex shaders without having to be aware of any code other than the mixin you are overriding. This concept is heavily used when writing surface and lighting shaders, as described in a later manual.

~~~~~~~~~~~~~~
// This mixin overrides the MyMixin behaviour we defined above
mixin MyMixin
{
	code
	{
		struct SurfaceData 
		{
			float4 albedo;
			float depth;
		};
		
		SurfaceData getGBufferData(uint2 pixelPos)
		{
			SurfaceData output;
			output.albedo = float4(1, 0, 0, 1); // Always return red color
			output.depth = 0.0f;
		
			return output;
		}			
	};
};
~~~~~~~~~~~~~~

# Passes
Passes can be used when a shader needs to perform multiple complex operations in a sequence. Each pass can be thought of as its own fully functional shader. By default shaders have one pass, which doesn't have to be explicitly defined, as was the case in all examples above. To explicitly define a pass, use the **pass** block and define the relevant code/state blocks within it, same as it was shown for shaders above. Passes will be executed sequentially one after another in the order they are defined. 

~~~~~~~~~~~~~~
shader MyShader
{
	// First pass
	pass
	{
		code
		{
			// Some shader code	
		};
	};
	
	// Second pass
	pass
	{
		// Enable blending so data written by this pass gets blended with data from the previous pass
		blend
		{
			target	
			{
				enabled = true;
				color = { 
					source = srcA;
					dest = srcIA;
					op = add;
				};
				writemask = RGB;
			};
		};	
	
		code
		{
			// Some shader code
		};
	};
};
~~~~~~~~~~~~~~

# Ray tracing
A shader can trace rays through the scene and react to what they hit. The scene's geometry is stored in an **acceleration structure**, an object built on the CPU that lets the GPU find what a ray hits without testing every triangle. It is made of **instances**, each of which places one mesh in the scene. A shader receives an acceleration structure as a parameter of type `RaytracingAccelerationStructure`, and can trace rays through it in two ways:
 - **Ray tracing passes** - The pass is made of ray tracing programs. One of them starts rays with `TraceRay`, and the GPU runs other programs of the pass for what each ray hits or misses.
 - **Inline ray tracing** - A program of any type, for example a fragment or a compute program, calls **RayTraceClosest** or **RayTraceAny** and gets the hit back as the return value.

Each way needs its own support from the GPU device: the @b3d::RSC_RAY_TRACING_PIPELINE capability for ray tracing passes, and the @b3d::RSC_RAY_QUERY capability for inline ray tracing. Check for them with @b3d::GpuDeviceCapabilities::HasCapability.

A ray is described by the HLSL `RayDesc` struct, with the members `Origin`, `TMin`, `Direction` and `TMax`. BSL declares it for you, together with `BuiltInTriangleIntersectionAttributes`, the `RAY_FLAG_*` and `HIT_KIND_*` constants, and the `RayHit` and `RayCandidate` structs described below. A shader can't declare anything of its own under these names.

## Ray tracing passes
A pass is a ray tracing pass when its code has ray tracing entry points. These don't have fixed names. Any function marked with the `[shader("...")]` attribute is a ray tracing entry point, and the attribute's argument selects the type of program it becomes:

Attribute                    | Program type   | Runs                                                                                                   | Parameters
-----------------------------|----------------|--------------------------------------------------------------------------------------------------------|-----------
`[shader("raygeneration")]`  | Ray generation | Once for every point of a ray tracing dispatch, usually once per pixel. Starts rays with `TraceRay`.   | None
`[shader("miss")]`           | Miss           | When a ray hits nothing.                                                                               | `inout` payload
`[shader("closesthit")]`     | Closest hit    | Once the search for hits ends, for the hit closest to the ray's origin.                                | `inout` payload, `in` hit attributes
`[shader("anyhit")]`         | Any hit        | For every hit found on geometry that isn't marked as opaque, to decide whether the hit counts. `IgnoreHit()` rejects the hit, `AcceptHitAndEndSearch()` accepts it and ends the search, and returning from the function accepts it and continues the search. | `inout` payload, `in` hit attributes
`[shader("intersection")]`   | Intersection   | When a ray enters the bounding box of a procedural primitive, which is geometry described by a box instead of triangles. Reports where the ray hits the shape inside the box with `ReportHit`. | None
`[shader("callable")]`       | Callable       | When another program calls `CallShader`.                                                               | `inout` parameter struct

Every ray tracing entry point returns `void`. Payloads and hit attributes are covered in the next section.

~~~~~~~~~~~~~~
shader RayTracingPass
{
	code
	{
		RaytracingAccelerationStructure gScene;
		RWTexture2D<float4> gOutput;

		struct ColorPayload
		{
			float3 color;
		};

		// Starts one ray per pixel and stores the color the ray comes back with
		[shader("raygeneration")]
		void GenerateRays()
		{
			uint2 pixel = DispatchRaysIndex().xy;
			float2 screenPosition = (float2(pixel) + 0.5f) / float2(DispatchRaysDimensions().xy);

			RayDesc ray;
			ray.Origin = float3(0.0f, 0.0f, -5.0f);
			ray.TMin = 0.001f;
			ray.Direction = normalize(float3(screenPosition * 2.0f - 1.0f, 1.0f));
			ray.TMax = 1000.0f;

			ColorPayload payload;
			payload.color = 0;

			TraceRay(gScene, RAY_FLAG_NONE, 0xFF, 0, 1, 0, ray, payload);

			gOutput[pixel] = float4(payload.color, 1.0f);
		}

		// Runs for rays that leave the scene
		[shader("miss")]
		void SkyMiss(inout ColorPayload payload)
		{
			payload.color = float3(0.3f, 0.5f, 0.9f);
		}

		// Runs for the closest triangle a ray hits
		[shader("closesthit")]
		void SurfaceHit(inout ColorPayload payload, in BuiltInTriangleIntersectionAttributes attributes)
		{
			float2 weights = attributes.barycentrics;
			payload.color = float3(1.0f - weights.x - weights.y, weights.x, weights.y);
		}
	};
};
~~~~~~~~~~~~~~

Each entry point is compiled into its own GPU program. A pass can have any number of entry points of the same type, for example one miss program for camera rays and another one for shadow rays. All entry points of a pass share the parameters declared in the pass.

BSL doesn't decide which of these programs are used together. Ray tracing pipelines are put together on the CPU, from the programs of the pass. This includes the **hit groups**: a hit group is the closest hit, any hit and intersection program that handle the hits on one kind of geometry, and each of the three is optional. The fourth, fifth and sixth argument of `TraceRay` select the hit group and the miss program for a ray, so their values depend on how the pipeline was put together.

A ray tracing pass can't also contain **vsmain**, **fsmain**, **gsmain**, **hsmain**, **dsmain** or **csmain**. A pass with a miss program named `Miss` and a **csmain** function, for example, fails to compile with `Pass 0 mixes ray tracing entry points (Miss) with vsmain/fsmain/gsmain/hsmain/dsmain/csmain.` A shader that needs both kinds of programs declares them in separate passes.

## Payloads and hit attributes
The programs of a ray tracing pass exchange data through structs:
 - The **payload** travels with a ray. The program that calls `TraceRay` fills it in and passes it as the last argument. Miss, closest hit and any hit programs receive it as their `inout` parameter and can change it, and the caller reads the result once `TraceRay` returns.
 - **Hit attributes** describe one hit. Any hit and closest hit programs receive them as their second parameter. For triangles the type is always `BuiltInTriangleIntersectionAttributes`, whose `barycentrics` member holds the weights of the triangle's second and third vertex at the hit. For procedural primitives it is the struct that the intersection program passes to `ReportHit`.
 - A callable program has one `inout` struct parameter, which the caller passes to `CallShader`.

All of them must be structs that hold only numeric data, so they can't have textures, buffers or samplers as members. Hit attributes are also limited to 32 bytes. A larger struct is a compile error, for example `attribute struct 'BigAttributes' is 36 bytes; ray tracing attributes are limited to 32 bytes`.

The programs of a pass don't have to use the same payload struct. The compiler can't check that the struct a ray is traced with matches the programs that end up running for it, because those are chosen in C++. The same holds for the hit attributes of an intersection program and the hit programs in its hit group.

`ReportHit` takes the distance to the hit along the ray, a hit kind and the hit attributes. The hit kind is a number of your choice that any hit and closest hit programs read with `HitKind()`. For triangles `HitKind()` returns `HIT_KIND_TRIANGLE_FRONT_FACE` or `HIT_KIND_TRIANGLE_BACK_FACE`.

~~~~~~~~~~~~~~
shader RayTracingHitPrograms
{
	code
	{
		Texture2D gOpacityTexture;
		SamplerState gOpacitySampler;

		struct ColorPayload
		{
			float3 color;
		};

		// Hit attributes of a sphere, 12 bytes
		struct SphereAttributes
		{
			float3 normal;
		};

		struct ShadingParameters
		{
			float3 normal;
			float3 color;
		};

		// Reports where the ray enters a sphere of radius one, centered on the object's origin
		[shader("intersection")]
		void SphereIntersection()
		{
			float3 origin = ObjectRayOrigin();
			float3 direction = ObjectRayDirection();

			float directionSquared = dot(direction, direction);
			float projection = dot(origin, direction);
			float discriminant = projection * projection - directionSquared * (dot(origin, origin) - 1.0f);

			if (discriminant >= 0.0f)
			{
				float distance = (-projection - sqrt(discriminant)) / directionSquared;

				SphereAttributes attributes;
				attributes.normal = origin + direction * distance;

				ReportHit(distance, 0, attributes);
			}
		}

		// Receives the hit attributes the intersection program reported
		[shader("closesthit")]
		void SphereHit(inout ColorPayload payload, in SphereAttributes attributes)
		{
			ShadingParameters parameters;
			parameters.normal = attributes.normal;
			parameters.color = 0;

			CallShader(0, parameters);

			payload.color = parameters.color;
		}

		// Rejects hits on the transparent parts of a triangle
		[shader("anyhit")]
		void CutoutHit(inout ColorPayload payload, in BuiltInTriangleIntersectionAttributes attributes)
		{
			float opacity = gOpacityTexture.SampleLevel(gOpacitySampler, attributes.barycentrics, 0).a;

			if (opacity < 0.5f)
				IgnoreHit();
		}

		[shader("callable")]
		void ShadeNormal(inout ShadingParameters parameters)
		{
			parameters.color = parameters.normal * 0.5f + 0.5f;
		}
	};
};
~~~~~~~~~~~~~~

## Ray tracing functions
The HLSL functions in this table can only be called from some types of ray tracing programs. Calling one from any other program is a compile error, and so is calling a function that does. None of them can be called from vertex, fragment, compute or other programs outside a ray tracing pass.

Function                                                                         | Ray generation | Miss | Closest hit | Any hit | Intersection | Callable
---------------------------------------------------------------------------------|----------------|------|-------------|---------|--------------|---------
`DispatchRaysIndex`, `DispatchRaysDimensions`                                    | Yes            | Yes  | Yes         | Yes     | Yes          | Yes
`TraceRay`                                                                       | Yes            | Yes  | Yes         | -       | -            | -
`CallShader`                                                                     | Yes            | Yes  | Yes         | -       | -            | Yes
`WorldRayOrigin`, `WorldRayDirection`, `RayTMin`, `RayTCurrent`, `RayFlags`      | -              | Yes  | Yes         | Yes     | Yes          | -
`InstanceIndex`, `InstanceID`, `GeometryIndex`, `PrimitiveIndex`                 | -              | -    | Yes         | Yes     | Yes          | -
`ObjectRayOrigin`, `ObjectRayDirection`                                          | -              | -    | Yes         | Yes     | Yes          | -
`ObjectToWorld3x4`, `ObjectToWorld4x3`, `WorldToObject3x4`, `WorldToObject4x3`   | -              | -    | Yes         | Yes     | Yes          | -
`HitKind`                                                                        | -              | -    | Yes         | Yes     | -            | -
`IgnoreHit`, `AcceptHitAndEndSearch`                                             | -              | -    | -           | Yes     | -            | -
`ReportHit`                                                                      | -              | -    | -           | -       | Yes          | -

Ray tracing programs don't run for a pixel or as part of a thread group, so none of them can use:
 - `Sample`, `SampleBias`, `SampleCmp`, `CalculateLevelOfDetail` and `CalculateLevelOfDetailUnclamped`. Sample textures with `SampleLevel`, `SampleGrad` or `SampleCmpLevelZero` instead.
 - `ddx`, `ddy`, their coarse and fine variants, and `fwidth`.
 - `clip` and `discard`. An any hit program rejects a hit with `IgnoreHit`.
 - `GroupMemoryBarrier`, `AllMemoryBarrier` and the barriers whose name ends in `WithGroupSync`.

## Inline ray tracing
**RayTraceClosest** and **RayTraceAny** trace a ray from the program that calls them, and return what the ray hit as a `RayHit` struct. They can be called from programs of every type, including the programs of a ray tracing pass.
 - `RayHit RayTraceClosest(RaytracingAccelerationStructure scene, uint rayFlags, uint instanceMask, RayDesc ray)` returns the hit closest to the ray's origin.
 - `RayHit RayTraceAny(RaytracingAccelerationStructure scene, uint rayFlags, uint instanceMask, RayDesc ray)` stops at the first hit it finds, which isn't necessarily the closest one. That is enough for a shadow ray, which only asks whether anything is in the way.

Both also have a form with a fifth argument, a struct with callbacks, which is described in the next section. The first four arguments are:
 - `scene` - The acceleration structure to trace through.
 - `rayFlags` - A combination of the `RAY_FLAG_*` constants, see [Ray flags](#ray-flags).
 - `instanceMask` - Selects the instances the ray can hit. An instance is skipped unless its own 8-bit mask, set when the acceleration structure is built, has a bit in common with this value.
 - `ray` - Origin, direction and range of the ray.

Without callbacks, triangles count as hits whether or not they are marked as opaque, and procedural primitives are skipped.

~~~~~~~~~~~~~~
shader InlineRayTracing
{
	code
	{
		RaytracingAccelerationStructure gScene;
		RWTexture2D<float4> gOutput;

		cbuffer LightParameters
		{
			float3 gDirectionToLight;
		};

		[numthreads(8, 8, 1)]
		void csmain(uint3 threadId : SV_DispatchThreadID)
		{
			RayDesc ray;
			ray.Origin = float3(threadId.xy, -10.0f);
			ray.TMin = 0.0f;
			ray.Direction = float3(0.0f, 0.0f, 1.0f);
			ray.TMax = 1000.0f;

			// Find the surface the ray sees
			RayHit hit = RayTraceClosest(gScene, RAY_FLAG_CULL_BACK_FACING_TRIANGLES, 0xFF, ray);

			float4 color = 0;
			if (hit.IsHit)
			{
				RayDesc shadowRay;
				shadowRay.Origin = ray.Origin + ray.Direction * hit.T;
				shadowRay.TMin = 0.001f;
				shadowRay.Direction = gDirectionToLight;
				shadowRay.TMax = 100.0f;

				// Anything between the surface and the light puts the surface in shadow
				bool isInShadow = RayTraceAny(gScene, RAY_FLAG_NONE, 0xFF, shadowRay).IsHit;

				color = float4(hit.Barycentrics, hit.InstanceID, isInShadow ? 0.0f : 1.0f);
			}

			gOutput[threadId.xy] = color;
		}
	};
};
~~~~~~~~~~~~~~

`RayHit` has the following members:

Name                                  | Type     | Value
--------------------------------------|----------|------
IsHit                                 | bool     | True if the ray hit something. When false, every other member is zero.
IsProcedural                          | bool     | True if the ray hit a procedural primitive, false if it hit a triangle.
T                                     | float    | Distance to the hit along the ray. The hit is at `ray.Origin + ray.Direction * T`.
Barycentrics                          | float2   | Weights of the triangle's second and third vertex at the hit. Zero for a procedural primitive.
FrontFace                             | bool     | True if the ray hit the front side of the triangle. False for a procedural primitive.
InstanceIndex                         | uint     | Index of the instance that was hit, within the acceleration structure.
InstanceID                            | uint     | Identifier given to the instance when the acceleration structure was built.
InstanceContributionToHitGroupIndex   | uint     | Hit group offset given to the instance when the acceleration structure was built.
GeometryIndex                         | uint     | Index of the geometry that was hit, within the mesh of the instance.
PrimitiveIndex                        | uint     | Index of the triangle or procedural primitive that was hit, within its geometry.
ObjectToWorld                         | float3x4 | Transform of the instance, from its object space to world space.
WorldToObject                         | float3x4 | Transform of the instance, from world space to its object space.

## Inline ray tracing callbacks
Two decisions need code from the shader while the ray is traced: whether a hit on geometry that isn't marked as opaque counts, and where a ray hits a procedural primitive. A ray tracing pass makes them in any hit and intersection programs. Inline ray tracing makes them in methods of a struct that is passed as the fifth argument:
 - `RayHit RayTraceClosest(RaytracingAccelerationStructure scene, uint rayFlags, uint instanceMask, RayDesc ray, inout Callbacks callbacks)`
 - `RayHit RayTraceAny(RaytracingAccelerationStructure scene, uint rayFlags, uint instanceMask, RayDesc ray, inout Callbacks callbacks)`

`Callbacks` is a struct of your own that has one or both of these methods:

Method                                                            | Called                                                               | Return value
------------------------------------------------------------------|----------------------------------------------------------------------|-------------
`bool AnyHit(RayCandidate candidate)`                             | For every triangle the ray meets that isn't marked as opaque.        | True accepts the hit. False ignores it, and the search continues.
`bool Intersection(RayCandidate candidate, out float distance)`   | For every procedural primitive whose bounding box the ray enters.    | True reports a hit at `distance` along the ray. A distance outside the ray's range, or beyond the closest hit found so far, is ignored.

Without an `AnyHit` method, triangles that aren't marked as opaque are accepted. Without an `Intersection` method, procedural primitives are skipped.

The struct can have other members and methods. It is passed as `inout`: the callbacks read the values its members had before the call, and the changes they make to the members are visible after the call.

~~~~~~~~~~~~~~
shader InlineRayTracingCallbacks
{
	code
	{
		RaytracingAccelerationStructure gScene;
		RWTexture2D<float4> gOutput;
		StructuredBuffer<float> gInstanceOpacity;

		struct SceneCallbacks
		{
			// Set before the trace
			float opacityThreshold;

			// Read after the trace
			uint ignoredCount;

			// Accepts a triangle only if its instance is opaque enough
			bool AnyHit(RayCandidate candidate)
			{
				if (gInstanceOpacity[candidate.InstanceID] >= opacityThreshold)
					return true;

				ignoredCount++;
				return false;
			}

			// Intersects the ray with a sphere of radius one, centered on the object's origin
			bool Intersection(RayCandidate candidate, out float distance)
			{
				float3 origin = candidate.ObjectRayOrigin;
				float3 direction = candidate.ObjectRayDirection;

				float directionSquared = dot(direction, direction);
				float projection = dot(origin, direction);
				float discriminant = projection * projection - directionSquared * (dot(origin, origin) - 1.0f);

				distance = (-projection - sqrt(max(discriminant, 0.0f))) / directionSquared;
				return discriminant >= 0.0f;
			}
		};

		[numthreads(8, 8, 1)]
		void csmain(uint3 threadId : SV_DispatchThreadID)
		{
			RayDesc ray;
			ray.Origin = float3(threadId.xy, -10.0f);
			ray.TMin = 0.0f;
			ray.Direction = float3(0.0f, 0.0f, 1.0f);
			ray.TMax = 1000.0f;

			SceneCallbacks callbacks;
			callbacks.opacityThreshold = 0.5f;
			callbacks.ignoredCount = 0;

			RayHit hit = RayTraceClosest(gScene, RAY_FLAG_NONE, 0xFF, ray, callbacks);

			float4 color = 0;
			if (hit.IsHit)
				color = float4(hit.T, hit.IsProcedural ? 1.0f : 0.0f, callbacks.ignoredCount, 1.0f);

			gOutput[threadId.xy] = color;
		}
	};
};
~~~~~~~~~~~~~~

The `candidate` parameter describes the triangle or procedural primitive the callback is asked about. Callbacks are called in no particular order.

Name                                  | Type     | Value
--------------------------------------|----------|------
T                                     | float    | Distance to the triangle along the ray. Zero in `Intersection`.
Barycentrics                          | float2   | Weights of the triangle's second and third vertex at the candidate hit. Zero in `Intersection`.
FrontFace                             | bool     | True if the ray meets the front side of the triangle. False in `Intersection`.
InstanceIndex                         | uint     | Index of the candidate's instance, within the acceleration structure.
InstanceID                            | uint     | Identifier given to the instance when the acceleration structure was built.
InstanceContributionToHitGroupIndex   | uint     | Hit group offset given to the instance when the acceleration structure was built.
GeometryIndex                         | uint     | Index of the candidate's geometry, within the mesh of the instance.
PrimitiveIndex                        | uint     | Index of the triangle or procedural primitive, within its geometry.
ObjectRayOrigin                       | float3   | Origin of the ray in the object space of the instance.
ObjectRayDirection                    | float3   | Direction of the ray in the object space of the instance.

The compiler enforces these rules:
 - The callbacks argument must be a local variable or a function parameter. A global variable, a static local variable, a member of another variable and an array element are all rejected.
 - Its type must be a struct declared at global scope, and not a template.
 - The struct needs at least one of the two methods, declared with exactly the return type and parameters shown above. A misspelled method name therefore fails with an error such as `callbacks struct 'Typo' has neither an 'AnyHit' nor an 'Intersection' method`.
 - A callback can't call **RayTraceClosest**, **RayTraceAny**, `TraceRay`, `CallShader` or `clip`, and can't use `discard`. This includes the functions it calls.

## Ray flags
Ray flags change which geometry a ray can hit. `TraceRay` accepts all of them. **RayTraceClosest** and **RayTraceAny** accept only some:

Flag                                                                              | Effect                                                                                  | Inline ray tracing
----------------------------------------------------------------------------------|-----------------------------------------------------------------------------------------|-------------------
`RAY_FLAG_NONE`                                                                   | No flags.                                                                               | Yes
`RAY_FLAG_FORCE_OPAQUE`                                                           | Treats all geometry as opaque, so no any hit program or `AnyHit` callback runs.         | Yes
`RAY_FLAG_FORCE_NON_OPAQUE`                                                       | Treats all geometry as not opaque.                                                      | Yes
`RAY_FLAG_CULL_BACK_FACING_TRIANGLES`, `RAY_FLAG_CULL_FRONT_FACING_TRIANGLES`     | Skips the triangles that face away from the ray, or toward it.                          | Yes
`RAY_FLAG_CULL_OPAQUE`, `RAY_FLAG_CULL_NON_OPAQUE`                                | Skips the geometry that is marked as opaque, or the geometry that isn't.                | Yes
`RAY_FLAG_SKIP_PROCEDURAL_PRIMITIVES`                                             | Skips procedural primitives.                                                            | Yes
`RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH`                                        | Ends the search at the first hit.                                                       | No, call **RayTraceAny** instead
`RAY_FLAG_SKIP_CLOSEST_HIT_SHADER`                                                | Doesn't run the closest hit program.                                                    | No, there is no closest hit program
`RAY_FLAG_SKIP_TRIANGLES`                                                         | Skips triangles.                                                                        | No

When the flags passed to **RayTraceClosest** or **RayTraceAny** are a constant, a flag that isn't accepted is a compile error, such as `ray flag RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH can't be used with 'RayTraceClosest'; use RayTraceAny to stop at the first hit`. Flags computed while the program runs can't be checked, and the flags that aren't accepted are ignored.

## Acceleration structure parameters
A `RaytracingAccelerationStructure` parameter is declared at the top level of a **code** block, like a texture. It can be declared as an array and passed to functions.

An acceleration structure can also be read from the resource heap, by indexing `ResourceDescriptorHeap` with a `uint`. It is then not a parameter of the shader. See [Bindless resources](../../Developer_Manuals/Low_Level_rendering/bindlessResources) for how programs access the heap.

~~~~~~~~~~~~~~
shader AccelerationStructureParameters
{
	code
	{
		// Bound as a parameter
		RaytracingAccelerationStructure gScene;

		cbuffer SceneParameters
		{
			// Index of another acceleration structure in the resource heap
			uint gHeapSceneIndex;
		};

		RWStructuredBuffer<float> gDistances;

		float TraceDistance(RaytracingAccelerationStructure scene, RayDesc ray)
		{
			RayHit hit = RayTraceClosest(scene, RAY_FLAG_NONE, 0xFF, ray);
			return hit.IsHit ? hit.T : ray.TMax;
		}

		[numthreads(64, 1, 1)]
		void csmain(uint3 threadId : SV_DispatchThreadID)
		{
			RayDesc ray;
			ray.Origin = float3(threadId.x, 0.0f, 0.0f);
			ray.TMin = 0.0f;
			ray.Direction = float3(0.0f, 0.0f, 1.0f);
			ray.TMax = 100.0f;

			RaytracingAccelerationStructure heapScene = ResourceDescriptorHeap[gHeapSceneIndex];

			gDistances[threadId.x] = min(TraceDistance(gScene, ray), TraceDistance(heapScene, ray));
		}
	};
};
~~~~~~~~~~~~~~

# Default values
Ordinary constants (uniforms) of primitive types can be assigned default values. Push-constant members cannot have initializers; their values are set through the command buffer. For ordinary uniforms, default values will be used if the user doesn't assign the values explicitly. The relevant syntax is:
 - For scalars: "type name = value;"
 - For vectors/matrices: "type name = { v0, v1, ... };", where the number of values is the total number of elements in a vector/matrix
 
~~~~~~~~~~~~~~
shader MyShader
{
	code
	{
		cbuffer SomeBuffer
		{
			bool val1 = false;
			float val2 = 5.0f;
			float3 val3 = { 0.0f, 1.0f, 2.0f };
			float2x2 val4 = { 0.0f, 1.0f, 2.0f, 3.0f };
		};
		
		// ... remaining shader code
	};
};
~~~~~~~~~~~~~~

Textures can also be assigned default values, limited to a specific subset. Valid set of values is:
 - white - All white texture
 - black - All black texture
 - normal - Texture representing a normal map with normals facing in the Y (up) direction
 
~~~~~~~~~~~~~~
shader MyShader
{
	code
	{
		Texture2D albedoMap = black;
		Texture2D normalMap = normal;
		
		// ... remaining shader code
	};
};
~~~~~~~~~~~~~~

Finally, sampler states may also be assigned default values. The values are specified in a block immediately following the sampler state definition.

~~~~~~~~~~~~~~
shader MyShader
{
	code
	{
		SamplerState MySampler
		{
			Filter = MIN_MAG_MIP_LINEAR;
			AddressU = Wrap;
			AddressV = Wrap;
		};

		// ... remaining shader code
	};
};
~~~~~~~~~~~~~~

Valid values within the sampler state block are:
Name                 | Valid values				   				  | Reference
---------------------|--------------------------------------------|----------
AddressU			 | WRAP, MIRROR, CLAMP, BORDER, MIRROR_ONCE   | @b3d::SAMPLER_STATE_DESC::addressMode
AddressV			 | WRAP, MIRROR, CLAMP, BORDER, MIRROR_ONCE   | @b3d::SAMPLER_STATE_DESC::addressMode
AddressW			 | WRAP, MIRROR, CLAMP, BORDER, MIRROR_ONCE   | @b3d::SAMPLER_STATE_DESC::addressMode
BorderColor			 | float4									  | @b3d::SAMPLER_STATE_DESC::borderColor
Filter				 | See table below							  | @b3d::SAMPLER_STATE_DESC::minFilter, @b3d::SAMPLER_STATE_DESC::magFilter, @b3d::SAMPLER_STATE_DESC::mipFilter
MaxAnisotropy		 | u32										  | @b3d::SAMPLER_STATE_DESC::maxAniso
MaxLOD				 | float									  | @b3d::SAMPLER_STATE_DESC::mipMax
MinLOD				 | float									  | @b3d::SAMPLER_STATE_DESC::mipMin
MipLODBias			 | float									  | @b3d::SAMPLER_STATE_DESC::mipmapBias
ComparisonFunc		 | NEVER, LESS, EQUAL, LESS_EQUAL, GREATER, NOT_EQUAL, GREATER_EQUAL, ALWAYS | @b3d::SAMPLER_STATE_DESC::comparisonFunc

Filter valid values:
- MIN_MAG_MIP_POINT
- MIN_MAG_POINT_MIP_LINEAR
- MIN_POINT_MAG_LINEAR_MIP_POINT
- MIN_POINT_MAG_MIP_LINEAR
- MIN_LINEAR_MAG_MIP_POINT
- MIN_LINEAR_MAG_POINT_MIP_LINEAR
- MIN_MAG_LINEAR_MIP_POINT
- MIN_MAG_MIP_LINEAR
- ANISOTROPIC
- COMPARISON_MIN_MAG_MIP_POINT
- COMPARISON_MIN_MAG_POINT_MIP_LINEAR
- COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT
- COMPARISON_MIN_POINT_MAG_MIP_LINEAR
- COMPARISON_MIN_LINEAR_MAG_MIP_POINT
- COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR
- COMPARISON_MIN_MAG_LINEAR_MIP_POINT
- COMPARISON_MIN_MAG_MIP_LINEAR
- COMPARISON_ANISOTROPIC
- MINIMUM_MIN_MAG_MIP_POINT
- MINIMUM_MIN_MAG_POINT_MIP_LINEAR
- MINIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT
- MINIMUM_MIN_POINT_MAG_MIP_LINEAR
- MINIMUM_MIN_LINEAR_MAG_MIP_POINT
- MINIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR
- MINIMUM_MIN_MAG_LINEAR_MIP_POINT
- MINIMUM_MIN_MAG_MIP_LINEAR
- MINIMUM_ANISOTROPIC
- MAXIMUM_MIN_MAG_MIP_POINT
- MAXIMUM_MIN_MAG_POINT_MIP_LINEAR
- MAXIMUM_MIN_POINT_MAG_LINEAR_MIP_POINT
- MAXIMUM_MIN_POINT_MAG_MIP_LINEAR
- MAXIMUM_MIN_LINEAR_MAG_MIP_POINT
- MAXIMUM_MIN_LINEAR_MAG_POINT_MIP_LINEAR
- MAXIMUM_MIN_MAG_LINEAR_MIP_POINT
- MAXIMUM_MIN_MAG_MIP_LINEAR
- MAXIMUM_ANISOTROPIC

> Note: Sampler state default values use the same syntax as HLSL Effects framework.

# Attributes
BSL provides a couple of extension attributes that can be applied to constants (uniforms) or constant (uniform) blocks. Attributes are specified using the standard HSLS [] attribute syntax.

~~~~~~~~~~~~~~
shader MyShader
{
	code
	{
		[someAttribute]
		Texture2D someMap;
		
		[someAttributeWithParameters(p0, p1)]
		Texture2D someMap2;
	};
};
~~~~~~~~~~~~~~

Following is a complete list of supported attributes with their use cases.

## pushConstant

Marks a `cbuffer` as a small block of data set directly through the low-level command buffer. Unlike ordinary constant buffers, it does not create material parameters or a parameter-set binding. Use it for frequently changed values such as draw indices, flags, or a few scalar parameters.

~~~~~~~~~~~~~~
shader PushConstantColor
{
	code
	{
		[pushConstant]
		cbuffer DrawConstants
		{
			float gExposure;
			float2 gOffset;
		};

		float4 fsmain() : SV_Target0
		{
			return float4(gOffset, gExposure, 1.0f);
		}
	};
};
~~~~~~~~~~~~~~

The attribute takes no arguments, applies only to a `cbuffer`, and allows at most one push-constant block per GPU program. Do not specify a `register` binding. Members must not have initializers, semantics, or `packoffset` annotations.

The engine supports up to 16 bytes per block. Use 32-bit `float`, `int`, or `uint` types; vectors, matrices, and structures composed of these types are supported if they fit. Arrays, `bool`, `half`, and `double` are not supported.

Layout follows std140-compatible alignment: scalars align to 4 bytes, two-component vectors to 8, and three- or four-component vectors to 16. The block size is rounded to four bytes. In this example `gExposure` starts at byte 0, `gOffset` at byte 8, and the block is 16 bytes, including four bytes of padding between the members. Do not assume a C++ structure has matching padding.

See [Setting push constants](../../Developer_Manuals/Low_Level_rendering/gpuPrograms#push-constants) for the matching command-buffer example.

## dynamicOffset

Marks a uniform-buffer binding whose byte offset can be changed through @b3d::render::GpuCommandBuffer::SetDynamicBufferOffset without rebinding its parameter set:

~~~~~~~~~~~~~~
[dynamicOffset]
cbuffer PerDraw
{
	float4 gColor;
};
~~~~~~~~~~~~~~

The main goal it accomplishes is performance, as it's a very cheap thing to change between draw calls.

The attribute takes no arguments. Unmarked buffers use ordinary bindings. It cannot be combined with `[pushConstant]` or applied to storage buffers or arrays of buffer resources; arrays of values inside the buffer are supported. See [GPU buffers](../../Developer_Manuals/Low_Level_rendering/gpuBuffers) for selecting buffer suballocations.

## internal
Marks a constant or a constant buffer (cbuffer) so it is hidden from the material's public interface (editor UI or **Material** API). This is useful for constants that are set by the engine itself and shouldn't be touched by normal users. Additionaly internal cbuffers must be explicitly created and assigned by the low level rendering API, as they will not be created automatically.

In the example below only *position* and *tint* parameters will be accessible through the material.
~~~~~~~~~~~~~~
shader MyShader
{
	code
	{
		// Hide from public interface
		[internal]
		Texture2D someMap;
		
		cbuffer SomeBuffer
		{
			float3 position;
			float3 tint;
			
			// Hide from public interface
			[internal]
			float4x4 viewProjMat;
		};		
	};
};
~~~~~~~~~~~~~~

## color
Marks the floating point vector constant as a color. This ensures the constant is displayed as a color in the editor UI (with access to a color picker), and is represented by the **Color** structure in **Material** API. Only usable on *float3* or *float4* types.

~~~~~~~~~~~~~~
shader MyShader
{
	code
	{
		cbuffer SomeBuffer
		{
			float3 position;

			// Interpret as color, instead of a 3D vector
			[color]
			float3 tint;
		};
	};
};
~~~~~~~~~~~~~~

## hdr
Marks a color constant to support high dynamic range (HDR) values. This allows the color to have values outside the normal [0, 1] range. Must be used in combination with the **color** attribute. Only usable on *float3* or *float4* types.

~~~~~~~~~~~~~~
shader MyShader
{
	code
	{
		cbuffer MaterialParams
		{
			// Emissive color with HDR support
			[color][hdr]
			float3 emissiveColor = { 1.0f, 1.0f, 1.0f };
		};
	};
};
~~~~~~~~~~~~~~

## hideInInspector
Marks a constant or constant buffer to be hidden from the material inspector UI in the editor. Unlike the **internal** attribute, the constant is still accessible through the **Material** API, but it won't be shown in the editor's material properties panel.

~~~~~~~~~~~~~~
shader MyShader
{
	code
	{
		cbuffer Params
		{
			// Hidden from inspector but accessible via Material API
			[hideInInspector]
			float invDepthRange = 1.0f;
		};
	};
};
~~~~~~~~~~~~~~

## spriteuv
Marks a *float4* constant to be used for sprite texture animation. If a **Material** is assigned a **SpriteTexture** with animation enabled this parameter will receive the UV offset (in .xy components) and size (in .zw components) required for animating the texture. The attribute expects a texture name as a parameter, which corresponds to the texture to animate.

~~~~~~~~~~~~~~
shader MyShader
{
	code
	{
		// Texture that supports animation
		Texture2D gAlbedoTex;
		SamplerState gAlbedoSamp;
		
		cbuffer SomeBuffer
		{
			// Field to receive animated UV
			[spriteuv(gAlbedoTex)]
			float4 gSpriteUV;
		};
		
		float4 fsmain(float2 uv : TEXCOORD) : SV_Target0
		{
			// Transform the mesh UV using the offset/size to sample 
			// from the current sub-section of the texture
			float2 spriteUV = uv * gSpriteUV.zw + gSpriteUV.xy;
		
			return gAlbedoTex.Sample(gAlbedoSamp, spriteUV);
		}	
	};
};
~~~~~~~~~~~~~~

## alias
Allows you to provide an alternative name to a SamplerState. This is important when using render backends like OpenGL, which may not support separate sampler states. In these cases the sampler state will be 'merged' with the texture it is used on. This means all internal systems will report the name of this sampler to be the same as the name of the texture, rather than the given name. In this case you will want to explictly add an `alias(TextureName)` attribute so the system knows this sampler state might have an alternative name.

This does come with a restriction that a sampler state can only be used with a single texture.

~~~~~~~~~~~~~~
shader MyShader
{
	code
	{
		// Sampler used with the 'gAlbedoTex'
		[alias(gAlbedoTex)]
		SamplerState gAlbedoSamp;
		
		// Sampler used with the 'gNormalTex'
		[alias(gNormalTex)]
		SamplerState gNormalSamp;
		
		Texture2D gAlbedoTex;
		Texture2D gNormalTex;
	};
};
~~~~~~~~~~~~~~

## layout
Used primarily as compatibility with OpenGL and Vulkan code, which require read-write objects (e.g. **RWTexture**) to have an explicit layout provided in shader. This is only required when READING from a read-write object AND when you will be using either OpenGL or Vulkan render backend.

This attribute expects a parameter that lets the system know the format of the data in the buffer. Valid values are:
- rgba32f
- rgba16f
- rg32f
- rg16f
- r11f_g11f_b10f
- r32f
- r16f
- rgba16
- rgb10_a2
- rgba8
- rg16
- rg8
- r16
- r8
- rgba16_snorm
- rgba8_snorm
- rg16_snorm
- rg8_snorm
- r16_snorm
- r8_snorm
- rgba32i
- rgba16i
- rgba8i
- rg32i
- rg16i
- rg8i
- r32i
- r16i
- r8i
- rgba32ui
- rgba16ui
- rgb10_a2ui
- rgba8ui
- rg32ui
- rg16ui
- rg8ui
- r32ui
- r16ui
- r8ui

Where:
 - letters "rgba" represent the number of channels
 - numbers represent number of bits per channel
 - "f" - floating point
 - "snorm" - normalized real in [-1, 1] range (internally backed by a signed integer)
 - "i" - signed integer
 - "ui" - unsigned integer
 - no suffix - normalized real in [0, 1] range (internally backed by an unsigned integer)

~~~~~~~~~~~~~~
shader MyShader
{
	code
	{
		// Texture contains 4-channel 16-bit floating point data, and we plan on reading from it
		[layout(rgba16f))]
		RWTexture2D someMap2;
	};
};
~~~~~~~~~~~~~~

# Global options
BSL supports a few global options that control all shaders and mixins in a shader file. These options are specified in a **options** block, which must be defined at the top most level along with **shader** or **mixin** blocks.

~~~~~~~~~~~~~~
options
{
	separable = true;
	sort = backtofront;
	transparent = true;
	priority = 100;
};

shader MyShader
{
	// Shader definition
};
~~~~~~~~~~~~~~

Valid options are:
Name                 | Valid values				   | Default value			| Description
---------------------|---------------------------- |------------------------|------------
separable			 | true, false				   | false					| When true, tells the renderer that passes within the shader don't need to be renderered one straight after another. This allows the system to perform rendering more optimally, but can be unfeasible for most materials which will depend on exact rendering order. Only relevant if a shader has multiple passes.
sort				 | none, backtofront, fronttoback | fronttoback			| Determines how does the renderer sort objects with this material before rendering. Most objects should be sorted front to back in order to avoid overdraw. Transparent (see below) objects will always be sorted back to front and this option is ignored. When no sorting is active the system will try to group objects based on the material alone, reducing material switching and potentially reducing CPU overhead, at the cost of overdraw.
transparent			 | true, false				   | false					| Notifies the renderer that this object is see-through. This will force the renderer to the use back to front sorting mode, and likely employ a different rendering method. Attempting to render transparent geometry without this option set to true will likely result in graphical artifacts.
forward				 | true, false				   | false					| Notifies the renderer that this object should be rendered using the forward rendering pipeline (as opposed to a deferred one).
priority			 | integer					   | 0						| Allows you to force objects with this shader to render before others. Objects with higher priority will be rendered before those with lower priority. If sorting is enabled, objects will be sorted within their priority groups (i.e. priority takes precedence over sort mode).

# Variations
Sometimes you need a few versions of the same shader, that are mostly similar but have some minor differences between them. For example, when rendering objects you might need to support a vertex shader for static meshes, as well as those using skinned and/or morph animation. 

This is where the **variations** block comes into play. It allows you to specify a set of permutations for which the shader will be compiled. During shader import every permutation of that shader will be parsed, enabling pre-processor \#define blocks depending on the current permutation. The \#defines take on the name of their variation, and one of the user provided values.

~~~~~~~~~~~~~~
// An example shader supporting different mesh animation modes
shader VertexInput
{
	// This will be be compiled using 4 different variations:
	// - Static (no animation) (SKINNED = false, MORPH = false)
	// - Skinned animation (SKINNED = true, MORPH = false)
	// - Morph animation (SKINNED = false, MORPH = true)
	// - Skinned morph animation (SKINNED = true, MORPH = true)
	variations
	{
		SKINNED = { false, true };
		MORPH = { false, true };
	};

	code
	{
		struct VertexInput
		{
			float3 position : POSITION;
			float3 normal : NORMAL; // Note: Half-precision could be used
			float4 tangent : TANGENT; // Note: Half-precision could be used
			float2 uv0 : TEXCOORD0;
			
			#if SKINNED
				uint4 blendIndices : BLENDINDICES;
				float4 blendWeights : BLENDWEIGHT;
			#endif
			
			#if MORPH
				float3 deltaPosition : POSITION1;
				float4 deltaNormal : NORMAL1;
			#endif				
		};
		
		// Potentially more code...
	};
};
~~~~~~~~~~~~~~

The syntax within the **variations** block is as follows:

 - `IDENTIFIER = { bool/int, bool/int, ... }`
 
Each variation block can have one or multiple entries. Each must have a unique identifier. Each entry can take on two or more values, each representing a single variation. The values must be boolean (true/false), or integers. If there are multiple variation entries, a variation for each possible combination of their values will be created.

Once a shader with variations is imported, those variations are available as @b3d::Variation objects. See the [Advanced materials manual](../../01_Developer_Manuals/03_advMaterials.md) for selecting and using variations with the low-level rendering API.

Variations used with **Material** must preserve the material-visible parameter names, types and array sizes. Internal parameters may vary. **RendererMaterial** and direct low-level rendering can use different parameter interfaces in different variations. Within one variation and pass, stages share the same source declarations and bindings.
# Sub-shaders
Each BSL file can contain an optional set of sub-shaders, alongside the main shader. Sub-shaders are recognized by the renderer and are meant to allow the user to override functionality of default shaders used by the renderer. They are specified using the **subshader** keyword, followed by an unique identifier. Sub-shaders are only allowed to contain **mixin** blocks, within which the same rules as for normal mixins apply.

~~~~~~~~~~~~~~
shader MainShader
{
	// Shader definition
};

subshader SomeSubShader
{
	mixin SomeMixin
	{
		// Mixin definition
	};
};
~~~~~~~~~~~~~~

The mixins you specify in a sub-shader will be used to override some existing mixin, allowing you to change specific rendering functionality. 

Imagine for example that you wanted to implement a custom lighting model while using deferred rendering. Deferred rendering doesn't perform lighting when rendering the object, instead it does lighting in a single pass for all objects at once. The renderer will closely interact with that lighting shader, and if you wanted to change it you would need to rewrite it using the exact rules as the renderer expects. Worse yet, the renderer might support different forms of deferred rendering (e.g. standard vs. tiled), meaning you would need to write multiple shaders to fully support it. You would also need to keep your shader(s) updated whenever the renderer's internal change. For a normal user this is very difficult and requires detailed knowledge of how the renderer works.

bsf's renderer instead provides a set of "extension points". Extension points match to a specific part of the renderer pipeline that can be overriden. For example an extension point named "DeferredDirectLighting" would let you override the BRDF and direct lighting calculations during deferred rendering. Sub-shaders are how you "link" with those extension points. The name of the sub-shader corresponds to the extension point name, and the mixins in the sub-shader override specific functionality of that extension point. For example you might override the "StandardBRDF" mixin to change the BRDF model for the standard material, or "LuminanceSpot" to change how is luminance for spot lights calculated.

This way you only override small parts of the renderer shader code, without having to rewrite all the complex shader code, while all the  implementation details are transparently handled by the renderer itself.

~~~~~~~~~~~~~~
// Sub-shader overriding direct lighting used for deferred rendering
subshader DeferredDirectLighting // Name of the subshader specifies exactly what part of the rendering pipeline to override
{
	// Exact name of the mixin to override. Each subshader extension point can provide one or multiple mixins that can be overriden
	mixin StandardBRDF
	{
		// Override the PBR BRDF with a simple Lambert BRDF
		float3 evaluateStandardBRDF(float3 V, float3 L, float specLobeEnergy, SurfaceData surfaceData)
		{
			return surfaceData.albedo.rgb / 3.14f;
		}
	};
};
~~~~~~~~~~~~~~

In a later manual we will provide a list of all extension points that the default renderer provides, as well as the mixins that can be overriden, and their required method signatures.

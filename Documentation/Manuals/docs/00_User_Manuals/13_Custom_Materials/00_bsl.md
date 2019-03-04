---
title: BSL syntax
---

All shaders in bs::f are written in BSL (bs::f Shading Language). The core of the language is based on HLSL (High Level Shading Language), with various extensions to make development easier. In this manual we will not cover HLSL syntax, nor talk about shaders in general, and will instead focus on the functionality specific to BSL. If you are not familiar with the concept of a shader, or HLSL syntax, it is suggested you learn about them before continuing.

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
  
Let's now move onto more advanced functionality specific to BSL.

# Non-programmable states
Aside from the **code** block, a **shader** can also specify four blocks that allow it to control non-programmable parts of the pipeline:
 - **raster** - Allows you to set options related to rasterization, like rasterization mode (fill/wireframe), cull mode, etc.
 - **depth** - Allows you to set options related to depth buffer and depth comparison, like enabling/disabling depth reads or writes, or changing the depth comparison function
 - **stencil** - Allows you to set options related to the stencil buffer and stencil test, like enabling stencil test and setting the test operations
 - **blend** - Allows you to set blending options, like enabling/disabling blending, setting blend operations or setting color the write mask

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
fill    	  	     | wire, solid (See @bs::PolygonMode)			   | @bs::RASTERIZER_STATE_DESC::polygonMode
cull    	  	     | cw, ccw, none (See @bs::CullingMode)			   | @bs::RASTERIZER_STATE_DESC::cullMode
scissor				 | true, false				   | @bs::RASTERIZER_STATE_DESC::scissorEnable
multisample			 | true, false				   | @bs::RASTERIZER_STATE_DESC::multisampleEnable
lineaa				 | true, false				   | @bs::RASTERIZER_STATE_DESC::antialiasedLineEnable

## depth
Name                 | Valid values				   | Reference
---------------------|---------------------------- |----------
read    	  	     | true, false				   | @bs::DEPTH_STENCIL_STATE_DESC::depthReadEnable
write    	  	     | true, false				   | @bs::DEPTH_STENCIL_STATE_DESC::depthWriteEnable
compare    	  	     | never, always, lt, lte, eq, neq, gte, gt (See @bs::CompareFunction)				   | @bs::DEPTH_STENCIL_STATE_DESC::depthComparisonFunc
bias    	  	     | float				   | @bs::RASTERIZER_STATE_DESC::depthBias
scaledBias    	  	 | float				   | @bs::RASTERIZER_STATE_DESC::slopeScaledDepthBias
clip    	  	     | true, false				   | @bs::RASTERIZER_STATE_DESC::depthClipEnable

## stencil
Name                 | Valid values				   | Reference
---------------------|---------------------------- |----------
reference    	  	 | integer			           | Reference value to use for stencil compare operations.
enabled    	  	     | true, false				   | @bs::DEPTH_STENCIL_STATE_DESC::stencilEnable
readmask    	  	 | integer in [0, 255] range   | @bs::DEPTH_STENCIL_STATE_DESC::stencilReadMask
writemask    	  	 | integer in [0, 255] range   | @bs::DEPTH_STENCIL_STATE_DESC::stencilWriteMask
front				 | StencilOp block			   | Stencil operations and compare function for front facing geometry
back				 | StencilOp block			   | Stencil operations and compare function for back facing geometry
 
**front** and **back** options are blocks themselves, and they accept the following options:
Name                 | Valid values				   | Reference
---------------------|---------------------------- |----------
fail    	  	 	| keep, zero, replace, inc, dec, incwrap, decwrap, inverse (See @bs::StencilOperation)			           | @bs::DEPTH_STENCIL_STATE_DESC::frontStencilFailOp & @bs::DEPTH_STENCIL_STATE_DESC::backStencilFailOp
zfail    	  	 	| keep, zero, replace, inc, dec, incwrap, decwrap, inverse (See @bs::StencilOperation)			           | @bs::DEPTH_STENCIL_STATE_DESC::frontStencilZFailOp & @bs::DEPTH_STENCIL_STATE_DESC::backStencilZFailOp
pass    	  	 	| keep, zero, replace, inc, dec, incwrap, decwrap, inverse (See @bs::StencilOperation)			           | @bs::DEPTH_STENCIL_STATE_DESC::frontStencilPassOp & @bs::DEPTH_STENCIL_STATE_DESC::backStencilPassOp
compare				| never, always, lt, lte, eq, neq, gte, gt (See @bs::CompareFunction) | @bs::DEPTH_STENCIL_STATE_DESC::frontStencilComparisonFunc & @bs::DEPTH_STENCIL_STATE_DESC::backStencilComparisonFunc

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
dither    	  	 | true, false			           | @bs::BLEND_STATE_DESC::alphaToCoverageEnable
independant    	 | true, false			           | @bs::BLEND_STATE_DESC::independantBlendEnable
target			 | Target block					   | Blend operations for a specific render target. Multiple Target blocks can exist under a single blend block.

**Target** block accepts the following options:
Name                 | Valid values				   | Reference
---------------------|---------------------------- |----------
index    	  	     | positive integer		       | Index of the render target these options are applied to. If not specified the index is derived from the order in which Target blocks are defined.
enabled				 | true, false				   | @bs::RENDER_TARGET_BLEND_STATE_DESC::blendEnable
writemask		     | R, G, B, A or any combination (e.g. RG, RBA, RGBA). "empty" for zero mask.					   | @bs::RENDER_TARGET_BLEND_STATE_DESC::renderTargetWriteMask
color    	  	     | BlendOp block	       	   | Represents the blend operation to execute on the color channels.
alpha				 | BlendOp block			   | Represents the blend operation to execute on the alpha channel. Only relevant if independant blend is enabled.

**BlendOp** block accepts the following options:
Name                 | Valid values				   | Reference
---------------------|---------------------------- |----------
source    	  	     | one, zero, dstRGB, srcRGB, dstIRGB, srcIRGB, dstA, srcA, dstIA, srcIA (See @bs::BlendFactor)		       | @bs::RENDER_TARGET_BLEND_STATE_DESC::srcBlend, @bs::RENDER_TARGET_BLEND_STATE_DESC::srcBlendAlpha
dest    	  	     | one, zero, dstRGB, srcRGB, dstIRGB, srcIRGB, dstA, srcA, dstIA, srcIA (See @bs::BlendFactor)		       | @bs::RENDER_TARGET_BLEND_STATE_DESC::dstBlend, @bs::RENDER_TARGET_BLEND_STATE_DESC::dstBlendAlpha
op    	  	     | add, sub, rsub, min, max (See @bs::BlendOperation)		       | @bs::RENDER_TARGET_BLEND_STATE_DESC::blendOp, @bs::RENDER_TARGET_BLEND_STATE_DESC::blendOpAlpha

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

# Default values
All constants (uniforms) of primitive types can be assigned default values. These values will be used if the user doesn't assign the values explicitly. The relevant syntax is:
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
AddressU			 | WRAP, MIRROR, CLAMP, BORDER, MIRROR_ONCE   | @bs::SAMPLER_STATE_DESC::addressMode
AddressV			 | WRAP, MIRROR, CLAMP, BORDER, MIRROR_ONCE   | @bs::SAMPLER_STATE_DESC::addressMode
AddressW			 | WRAP, MIRROR, CLAMP, BORDER, MIRROR_ONCE   | @bs::SAMPLER_STATE_DESC::addressMode
BorderColor			 | float4									  | @bs::SAMPLER_STATE_DESC::borderColor
Filter				 | See table below							  | @bs::SAMPLER_STATE_DESC::minFilter, @bs::SAMPLER_STATE_DESC::magFilter, @bs::SAMPLER_STATE_DESC::mipFilter
MaxAnisotropy		 | uint										  | @bs::SAMPLER_STATE_DESC::maxAniso
MaxLOD				 | float									  | @bs::SAMPLER_STATE_DESC::mipMax
MinLOD				 | float									  | @bs::SAMPLER_STATE_DESC::mipMin
MipLODBias			 | float									  | @bs::SAMPLER_STATE_DESC::mipmapBias
ComparisonFunc		 | NEVER, LESS, EQUAL, LESS_EQUAL, GREATER, NOT_EQUAL, GREATER_EQUAL, ALWAYS | @bs::SAMPLER_STATE_DESC::comparisonFunc

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

This is where the **variation** block comes into play. It allows you to specify a set of permutations for which the shader will be compiled. During shader import every permutation of that shader will be parsed, enabling pre-processor \#define blocks depending on the current permutation. The \#defines take on the name of their variation, and one of the user provided values.

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

The syntax within the **variation** block is as follows:
 - `IDENTIFIER = { bool/int, bool/int, ... }`
 
Each variation block can have one or multiple entries. Each must have a unique identifier. Each entry can take on two or more values, each representing a single variation. The values must be boolean (true/false), or integers. If there are multiple variation entries, a variation for each possible combination of their values will be created.

Once a shader with variations is imported, those variations will be available on the shader in the form of **Technique** objects. In general variations are only required when working with the low level rendering API, therefore we discuss techniques in more details in the developer manuals.

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

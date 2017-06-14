BSL syntax			{#bsl}
===============
[TOC]

All shaders in Banshee are written in BSL (Banshee Shading Language). The core of the language is based on HLSL (High Level Shading Language), with various extensions to make development easier. In this manual we will not cover HLSL syntax, nor talk about shaders in general, and will instead focus on the functionality specific to BSL.

# Basics 

A simple BSL program that renders a mesh all in white looks like this:
~~~~~~~~~~~~~~
technique MyShader
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
			return float4(1.0f, 1.0f, 1.0f 1.0f); 
		}	
	};
};
~~~~~~~~~~~~~~

As you can see, aside from the **technique** and **code** blocks, the shader code looks identical to HLSL. Each BSL shader must contain at least one **technique**. Inside the technique you can use a variety of options, but the minimum required is the **code** block, which allows you to specify programmable shader code. Using just this syntax you get the full power of HLSL, as if you were using it directly.

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
Aside from the **code** block, a **technique** can also specify four blocks that allow it to control non-programmable parts of the pipeline:
 - **raster** - Allows you to set options related to rasterization, like rasterization mode (fill/wireframe), cull mode, etc.
 - **depth** - Allows you to set options related to depth buffer and depth comparison, like enabling/disabling depth reads or writes, or changing the depth comparison function
 - **stencil** - Allows you to set options related to the stencil buffer and stencil test, like enabling stencil test and setting the test operations
 - **blend** - Allows you to set blending options, like enabling/disabling blending, setting blend operations or setting color the write mask

An example shader using a variety of these blocks is shown:
~~~~~~~~~~~~~~
technique MyShader
{
	// No depth reads of writes
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
---------------------|---------------------------- |-----------------------
fill    	  	     | wire, solid (See @ref bs::PolygonMode "PolygonMode")			   | @ref bs::RASTERIZER_STATE_DESC::polygonMode "RASTERIZER_STATE_DESC::polygonMode"
cull    	  	     | cw, ccw, none (See @ref bs::CullingMode "CullingMode")			   | @ref bs::RASTERIZER_STATE_DESC::cullMode "RASTERIZER_STATE_DESC::cullMode"
scissor				 | true, false				   | @ref bs::RASTERIZER_STATE_DESC::scissorEnable "RASTERIZER_STATE_DESC::scissorEnable"
multisample			 | true, false				   | @ref bs::RASTERIZER_STATE_DESC::multisampleEnable "RASTERIZER_STATE_DESC::multisampleEnable"
lineaa				 | true, false				   | @ref bs::RASTERIZER_STATE_DESC::antialiasedLineEnable "RASTERIZER_STATE_DESC::antialiasedLineEnable"

## depth
Name                 | Valid values				   | Reference
---------------------|---------------------------- |-----------------------
read    	  	     | true, false				   | @ref bs::DEPTH_STENCIL_STATE_DESC::depthReadEnable "DEPTH_STENCIL_STATE_DESC::depthReadEnable"
write    	  	     | true, false				   | @ref bs::DEPTH_STENCIL_STATE_DESC::depthWriteEnable "DEPTH_STENCIL_STATE_DESC::depthWriteEnable"
compare    	  	     | never, always, lt, lte, eq, neq, gte, gt (See @ref bs::CompareFunction "CompareFunction")				   | @ref bs::DEPTH_STENCIL_STATE_DESC::depthComparisonFunc "DEPTH_STENCIL_STATE_DESC::depthComparisonFunc"
bias    	  	     | float				   | @ref bs::RASTERIZER_STATE_DESC::depthBias "RASTERIZER_STATE_DESC::depthBias"
scaledBias    	  	 | float				   | @ref bs::RASTERIZER_STATE_DESC::slopeScaledDepthBias "RASTERIZER_STATE_DESC::slopeScaledDepthBias"
clip    	  	     | true, false				   | @ref bs::RASTERIZER_STATE_DESC::depthClipEnable "RASTERIZER_STATE_DESC::depthClipEnable"

## stencil
Name                 | Valid values				   | Reference
---------------------|---------------------------- |-----------------------
reference    	  	 | integer			           | Reference value to use for stencil compare operations.
enabled    	  	     | true, false				   | @ref bs::DEPTH_STENCIL_STATE_DESC::stencilEnable "DEPTH_STENCIL_STATE_DESC::stencilEnable"
readmask    	  	 | R, G, B, A or any combination (e.g. RG, RBA, RGBA). "empty" for zero mask.				   | @ref bs::DEPTH_STENCIL_STATE_DESC::stencilReadMask "DEPTH_STENCIL_STATE_DESC::stencilReadMask"
writemask    	  	 | R, G, B, A or any combination (e.g. RG, RBA, RGBA). "empty" for zero mask.				   | @ref bs::DEPTH_STENCIL_STATE_DESC::stencilWriteMask "DEPTH_STENCIL_STATE_DESC::stencilWriteMask"
front				 | StencilOp block			   | Stencil operations and compare function for front facing geometry
back				 | StencilOp block			   | Stencil operations and compare function for back facing geometry
 
**front** and **back** options are blocks themselves, and they accept the following options:
Name                 | Valid values				   | Reference
---------------------|---------------------------- |-----------------------
fail    	  	 	| keep, zero, replace, inc, dec, incwrap, decwrap, inverse (See @ref bs::StencilOperation "StencilOperation")			           | @ref bs::DEPTH_STENCIL_STATE_DESC::frontStencilFailOp "DEPTH_STENCIL_STATE_DESC::frontStencilFailOp" & @ref bs::DEPTH_STENCIL_STATE_DESC::backStencilFailOp "DEPTH_STENCIL_STATE_DESC::backStencilFailOp"
zfail    	  	 	| keep, zero, replace, inc, dec, incwrap, decwrap, inverse (See @ref bs::StencilOperation "StencilOperation")			           | @ref bs::DEPTH_STENCIL_STATE_DESC::frontStencilZFailOp "DEPTH_STENCIL_STATE_DESC::frontStencilZFailOp" & @ref bs::DEPTH_STENCIL_STATE_DESC::backStencilZFailOp "DEPTH_STENCIL_STATE_DESC::backStencilZFailOp"
pass    	  	 	| keep, zero, replace, inc, dec, incwrap, decwrap, inverse (See @ref bs::StencilOperation "StencilOperation")			           | @ref bs::DEPTH_STENCIL_STATE_DESC::frontStencilPassOp "DEPTH_STENCIL_STATE_DESC::frontStencilPassOp" & @ref bs::DEPTH_STENCIL_STATE_DESC::backStencilPassOp "DEPTH_STENCIL_STATE_DESC::backStencilPassOp"
compare				| never, always, lt, lte, eq, neq, gte, gt (See @ref bs::CompareFunction "CompareFunction") | @ref bs::DEPTH_STENCIL_STATE_DESC::frontStencilComparisonFunc "DEPTH_STENCIL_STATE_DESC::frontStencilComparisonFunc" & @ref bs::DEPTH_STENCIL_STATE_DESC::backStencilComparisonFunc "DEPTH_STENCIL_STATE_DESC::backStencilComparisonFunc"

An example of a stencil block:
~~~~~~~~~~~~~~
technique MyShader
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
technique MyShader
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
---------------------|---------------------------- |-----------------------
dither    	  	 | true, false			           | @ref bs::BLEND_STATE_DESC::alphaToCoverageEnable "BLEND_STATE_DESC::alphaToCoverageEnable"
independant    	 | true, false			           | @ref bs::BLEND_STATE_DESC::independantBlendEnable "BLEND_STATE_DESC::independantBlendEnable"
target			 | Target block					   | Blend operations for a specific render target. Multiple Target blocks can exist under a single blend block.

**Target** block accepts the following options:
Name                 | Valid values				   | Reference
---------------------|---------------------------- |-----------------------
index    	  	     | positive integer		       | Index of the render target these options are applied to. If not specified the index is derived from the order in which Target blocks are defined.
enabled				 | true, false				   | @ref bs::RENDER_TARGET_BLEND_STATE_DESC::blendEnable "RENDER_TARGET_BLEND_STATE_DESC::blendEnable"
writemask		     | R, G, B, A or any combination (e.g. RG, RBA, RGBA). "empty" for zero mask.					   | @ref bs::RENDER_TARGET_BLEND_STATE_DESC::renderTargetWriteMask "RENDER_TARGET_BLEND_STATE_DESC::renderTargetWriteMask"
color    	  	     | BlendOp block	       	   | Represents the blend operation to execute on the color channels.
alpha				 | BlendOp block			   | Represents the blend operation to execute on the alpha channel. Only relevant if independant blend is enabled.

**BlendOp** block accepts the following options:
Name                 | Valid values				   | Reference
---------------------|---------------------------- |-----------------------
source    	  	     | one, zero, dstRGB, srcRGB, dstIRGB, srcIRGB, dstA, srcA, dstIA, srcIA (See @ref bs::BlendFactor "BlendFactor")		       | @ref bs::RENDER_TARGET_BLEND_STATE_DESC::srcBlend "RENDER_TARGET_BLEND_STATE_DESC::srcBlend", @ref bs::RENDER_TARGET_BLEND_STATE_DESC::srcBlendAlpha "RENDER_TARGET_BLEND_STATE_DESC::srcBlendAlpha"
dest    	  	     | one, zero, dstRGB, srcRGB, dstIRGB, srcIRGB, dstA, srcA, dstIA, srcIA (See @ref bs::BlendFactor "BlendFactor")		       | @ref bs::RENDER_TARGET_BLEND_STATE_DESC::dstBlend "RENDER_TARGET_BLEND_STATE_DESC::dstBlend", @ref bs::RENDER_TARGET_BLEND_STATE_DESC::dstBlendAlpha "RENDER_TARGET_BLEND_STATE_DESC::dstBlendAlpha"
op    	  	     | add, sub, rsub, min, max (See @ref bs::BlendOperation "BlendOperation")		       | @ref bs::RENDER_TARGET_BLEND_STATE_DESC::blendOp "RENDER_TARGET_BLEND_STATE_DESC::blendOp", @ref bs::RENDER_TARGET_BLEND_STATE_DESC::blendOpAlpha "RENDER_TARGET_BLEND_STATE_DESC::blendOpAlpha"

An example of a **blend** block:
~~~~~~~~~~~~~~
technique MyShader
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
technique MyShader
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
When writing complex shaders is it is often useful to break them up into components. This is where the concept of a **mixin** comes in. Any shader code or programmable states defined in a **mixin** can be included in any **technique**. Syntax within a **mixin** block is identical to syntax in a **technique** block, meaning you can define code and non-programmable state blocks as shown above.

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

When a technique wishes to use a mixin, simply add it to the technique using the same **mixin** keyword, followed by its name.
~~~~~~~~~~~~~~
technique MyShader
{
	mixin MyMixin;

	code
	{
		// ...
	
		float4 fsmain(in VStoFS input) : SV_Target0
		{
			uint2 pixelPos = ...;
			
			// Technique can now call methods from the mixin
			SurfaceData surfaceData = getGBufferData(pixelPos);
		
			return surfaceData.albedo; 
		}	
	};
};
~~~~~~~~~~~~~~

Included mixins will append their shader code and states to the technique they are included in. If mixin and technique define the same states, the value of the states present on the technique will be used. If multiple included mixins use the same state then the state from the last included mixin will be used. Shader code is included in the order in which mixins are defined, followed by shader code from the technique itself.

Often you will want to define mixins in separate files. BSL files are normally stored with the ".bsl" extension, but when writing include files you should use the ".bslinc" extension instead, in order to prevent the system trying to compile the shader code on its own.

In order to include other files in BSL, use the \#include command. The paths are relative to the working folder, or if you are working in Banshee 3D editor, the paths are relative to your project folder. You can use the special variables $ENGINE$ and $EDITOR$ to access paths to the builtin engine and editor shaders.

~~~~~~~~~~~~~~
// Include the code for accessing Banshee's GBuffer
#include "$ENGINE$/GBufferInput.bslinc"

technique MyShader
{
	mixin GBufferInput;

	// ...
};
~~~~~~~~~~~~~~

## Mixin overrides
Mixins can override each other if another mixin is defined with the same name. The last defined mixin is considered the override and will be used in the technique. Techniques can also reference mixins that haven't been declared yet. 

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
Passes can be used when a technique needs to perform multiple complex operations in a sequence. Each pass can be thought of as its own fully functional shader. By default techniques have one pass, which doesn't have to be explicitly defined, as was the case in all examples above. To explicitly define a pass, use the **pass** block and define the relevant code/state blocks within it, same as it was shown for techniques above. Passes will be executed sequentially one after another in the order they are defined. 

~~~~~~~~~~~~~~
technique MyShader
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
technique MyShader
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
technique MyShader
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
technique MyShader
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
---------------------|--------------------------------------------|------------------------
AddressU			 | WRAP, MIRROR, CLAMP, BORDER, MIRROR_ONCE   | @ref bs::SAMPLER_STATE_DESC::addressMode "SAMPLER_STATE_DESC::addressMode"
AddressV			 | WRAP, MIRROR, CLAMP, BORDER, MIRROR_ONCE   | @ref bs::SAMPLER_STATE_DESC::addressMode "SAMPLER_STATE_DESC::addressMode"
AddressW			 | WRAP, MIRROR, CLAMP, BORDER, MIRROR_ONCE   | @ref bs::SAMPLER_STATE_DESC::addressMode "SAMPLER_STATE_DESC::addressMode"
BorderColor			 | float4									  | @ref bs::SAMPLER_STATE_DESC::borderColor "SAMPLER_STATE_DESC::borderColor"
Filter				 | See table below							  | @ref bs::SAMPLER_STATE_DESC::minFilter "SAMPLER_STATE_DESC::minFilter", @ref bs::SAMPLER_STATE_DESC::magFilter "SAMPLER_STATE_DESC::magFilter", @ref bs::SAMPLER_STATE_DESC::mipFilter "SAMPLER_STATE_DESC::mipFilter"
MaxAnisotropy		 | uint										  | @ref bs::SAMPLER_STATE_DESC::maxAniso "SAMPLER_STATE_DESC::maxAniso"
MaxLOD				 | float									  | @ref bs::SAMPLER_STATE_DESC::mipMax "SAMPLER_STATE_DESC::mipMax"
MinLOD				 | float									  | @ref bs::SAMPLER_STATE_DESC::mipMin "SAMPLER_STATE_DESC::mipMin"
MipLODBias			 | float									  | @ref bs::SAMPLER_STATE_DESC::mipmapBias "SAMPLER_STATE_DESC::mipmapBias"
ComparisonFunc		 | NEVER, LESS, EQUAL, LESS_EQUAL, GREATER, NOT_EQUAL, GREATER_EQUAL, ALWAYS | @ref bs::SAMPLER_STATE_DESC::comparisonFunc "SAMPLER_STATE_DESC::comparisonFunc"

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
technique MyShader
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

Supported attribute types are:
Name                 | Parameters			   | Usable on              | Description
---------------------|------------------------ |------------------------|------------------
internal    	  	 | none					   | constants and cbuffers | Forces the constant (or all the constants in a buffer if applied to cbuffer) to be hidden from the materials public interface (editor UI or **Material** API). This is useful for constants that are set by the engine itself and shouldn't be touched by normal users. Additionaly internal cbuffers must be explicitly created and assigned by the low level rendering API, as they will not be created automatically.
color				 | none					   | float3 or float4 constants	| Marks the floating point vector as a color. This ensures the constant is displayed as a color in the editor UI (with access to a color picker), and is represented by the **Color** structure in **Material** API.
layout				 | See table below		   | RW texture or buffer constant | Used primarily as compatibility with OpenGL and Vulkan code, which require read-write objects (e.g. **RWTexture**) to have an explicit layout provided in shader. This is only required when READING from a read-write object AND when you will be using either OpenGL or Vulkan render backend.

**layout** valid values:
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

Example of attributes in action:
~~~~~~~~~~~~~~
technique MyShader
{
	code
	{
		// Hide from public interface
		[internal]
		Texture2D someMap;
		
		cbuffer SomeBuffer
		{
			float3 position;
			
			// Interpret as color, instead of a 3D vector
			[color]
			float3 tint;
		};		
		
		// Texture contains 4-channel 16-bit floating point data, and we plan on reading from it
		[layout(rgba16f))]
		RWTexture2D someMap2;
	};
};
~~~~~~~~~~~~~~

# Global options
BSL supports a few global options that control all techniques and mixins in a shader file. These options are specified in a **options** block, which must be defined at the top most level along with **technique** or **mixin** blocks.

~~~~~~~~~~~~~~
options
{
	separable = true;
	sort = backtofront;
	transparent = true;
	priority = 100;
};

technique MyShader
{
	// Technique definition
};
~~~~~~~~~~~~~~

Valid options are:
Name                 | Valid values				   | Default value			| Description
---------------------|---------------------------- |------------------------|------------------
separable			 | true, false				   | false					| When true, tells the renderer that passes within the shader don't need to be renderered one straight after another. This allows the system to perform rendering more optimally, but can be unfeasible for most materials which will depend on exact rendering order. Only relevant if a technique has multiple passes.
sort				 | none, backtofront, fronttoback | fronttoback			| Determines how does the renderer sort objects with this material before rendering. Most objects should be sorted front to back in order to avoid overdraw. Transparent (see below) objects will always be sorted back to front and this option is ignored. When no sorting is active the system will try to group objects based on the material alone, reducing material switching and potentially reducing CPU overhead, at the cost of overdraw.
transparent			 | true, false				   | false					| Notifies the renderer that this object is see-through. This will force the renderer to the use back to front sorting mode, and likely employ a different rendering method. Attempting to render transparent geometry without this option set to true will likely result in graphical artifacts.
priority			 | integer					   | 0						| Allows you to force objects with this shader to render before others. Objects with higher priority will be rendered before those with lower priority. If sorting is enabled, objects will be sorted within their priority groups (i.e. priority takes precedence over sort mode).
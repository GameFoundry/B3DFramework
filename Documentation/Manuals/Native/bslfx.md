Banshee Shading Language FX			{#bslfx}
===============
[TOC]

BSLFX is a material definition language that allows you to specify non-programmable render states together with programmable ones. For example BSLFX will allow you to define a set of input parameters, rasterizer, depth-stencil and blend states along with actual vertex/fragment GPU program code.

Actual GPU program code itself is written in any of the standard languages: HLSL or GLSL. A unified shading language is in the works so you will don't have to write separate code for DirectX and OpenGL renderers.

Before continuing it is highly suggested you read the [material](@ref materials) manual, as it describes how to create shaders without the use of BSLFX which can provide a good background of where BSLFX is useful.

BSLFX is a language with simple syntax that handles configuration but no actual logic. All syntax can be represented using this primitive:
 - Type [Name] [= Value|Block] [: Modifier];

Where:
 - **Type** is one of the built-in language constructs. Most types are only allowed to be used in a certain context.
 - **Name** is a user-defined name of the object. Certain **Types** require no names.
 - **Value** is an integer, floating point value (including vectors and matrices), boolean, string (surrounded by quotes, “value”), or one of the built-in values. Which type of value is accepted depends on **Type**. 
 - **Block** is a part of code that contains more primitives. As a special case blocks can also contain HLSL or GLSL code.

A simple program that renders a mesh all in white looks like this:
~~~~~~~~~~~~~~
Parameters =
{
	mat4x4		gMatWorldViewProj;
	mat4x4		gMatWorld;
	mat4x4		gMatInvWorld;
};

Blocks =
{
	Block PerObject;
};

Technique =
{
	Language = "HLSL11";
	
	Pass =
	{
		Common = 
		{
			struct VStoFS
			{
				float4 position : SV_Position;
			};
			
			cbuffer PerObject
			{
				float4x4 gMatWorldViewProj;
				float4x4 gMatWorld;
				float4x4 gMatInvWorld;
			}					
		};

		Vertex =
		{			
			struct VertexInput
			{
				float3 position : POSITION;
			};
			
			VStoFS main(VertexInput input)
			{
				VStoFS output;
			
				output.position = mul(gMatWorldViewProj, input.position);			
				return output;
			}
		};
		
		Fragment =
		{
			float4 main(in VStoFS input) : SV_Target0
			{
				return float4(1.0f, 1.0f, 1.0f 1.0f); 
			}	
		};
	};
};
~~~~~~~~~~~~~~
	
On the top-most level the program consists out of three blocks:
 - Parameters - Contains a list of input parameters into the program (constants/uniforms).
 - Blocks - Contains a list of blocks that group input parameters (constant buffers/uniform buffers).
 - Technique - Contains one or multiple passes that contain the pipeline state for rendering. This is the meat of the program.

# Parameters {#bslfx_a}
All parameters specified in this block will be exposed to the @ref BansheeEngine::Shader "Shader" object. These parameters will be accessible from the @ref BansheeEngine::Material "Material" object. You are still allowed to define uniforms/constants within shader code, without defining them in the parameter list, but they will not be visible to high level code. This can be useful if you are working on a lower level, like directly with the renderer.

Types supported in this block are:
 - int - signed integer
 - int2 - 2D vector of signed integers
 - int3 - 3D vector of signed integers
 - int4 - 4D vector of signed integers
 - float - floating point value
 - float2 - 2D vector of floating point values
 - float3 - 3D vector of floating point values
 - float4 - 4D vector of floating point values
 - color - 4D vector of floating point values (the same as float4)
 - mat2x2 - 2x2 matrix of floating point values
 - mat2x3 - 2x3 matrix of floating point values
 - mat2x4 - 2x4 matrix of floating point values
 - mat3x2 - 3x2 matrix of floating point values
 - mat3x3 - 3x3 matrix of floating point values
 - mat3x4 - 3x4 matrix of floating point values
 - mat4x2 - 4x2 matrix of floating point values
 - mat4x3 - 4x3 matrix of floating point values
 - mat4x4 - 4x4 matrix of floating point values
 - Sampler1D - Sampler state for a 1D texture
 - Sampler2D - Sampler state for a 2D texture
 - Sampler3D - Sampler state for a 3D texture
 - SamplerCUBE - Sampler state for a cube texture
 - Sampler2DMS - Sampler state for a multi-sampled 2D texture
 - Texture1D - 1D texture
 - Texture2D - 2D texture
 - Texture3D - 3D texture
 - TextureCUBE - Cube texture
 - Texture2DMS - Multi-sampled 2D texture
 - RWTexture1D - 1D texture (UAV/load-store texture)
 - RWTexture2D - 2D texture (UAV/load-store texture)
 - RWTexture3D - 3D texture (UAV/load-store texture)
 - RWTexture2DMS - Multi-sampled 2D texture (UAV/load-store texture)
 - ByteBuffer - Readable buffer of raw bytes
 - StructBuffer - Readable buffer of structs
 - RWByteBuffer - Read/write buffer of raw bytes (UAV/load-store buffer)
 - RWStructBuffer  - Read/write buffer of structs (UAV/load-store buffer)
 - AppendBuffer - Buffer that is used for appending data in a stack-like fashion
 - ConsumeBuffer - Buffer that is used for consuming data in a stack-like fashion
 
Each parameter must have at least a type followed by a unique name. The name must match the name of the parameter in actual shader code:
~~~~~~~~~~~~~~
mat4x4 gMatWorldViewProj;
~~~~~~~~~~~~~~

You are also allowed to specify a default value for primitive types:
~~~~~~~~~~~~~~
float gMyFloat = 5.0f;
~~~~~~~~~~~~~~
	
Or:
~~~~~~~~~~~~~~
mat2x2 gMyMat = { 1.0f, 0.0f, 0.0f, 1.0f };
~~~~~~~~~~~~~~

Textures can also have default values. Currently the accepted ones are "White", "Black" and "Normal". "Normal" will provide a normal texture with all normals pointing upwards, and "White" and "Black" will provide a white and black texture, respectively. For example:
~~~~~~~~~~~~~~
Texture2D gMyTexture = "White";
~~~~~~~~~~~~~~

Sampler states support more complex default values in the form of their own block. For example:
~~~~~~~~~~~~~~
Sampler2D gMySampler = { ... };
~~~~~~~~~~~~~~

Actual values in the sampler state will be explained later.

Final element that parameters can have are modifiers. Modifiers are in the format of ": Modifier(Value)". Supported modifiers are:
 - auto - Accepts a string that contains a semantic name. For example: `mat4x4 gMatWorldViewProj : auto("WVP");`. If the semantic "WVP" is recognized by the active renderer this value will be automatically assigned by the renderer. Automatic values cannot be manually set through the @ref BansheeEngine::Material "Material" interface. The default renderer doesn't support any parameter semantics, it instead works using block semantics (see below).
 - alias - Accepts a string that contains an alternative name for the element. This can only be used for samplers, and is used for interfacing with render APIs that do not have separate objects for textures and samplers (e.g. OpenGL). You can use this to give your sampler the same name as the texture so such API will recognize it. For example:
~~~~~~~~~~~~~~
Texture2D gMyTexture;
Sampler2D gMySampler : alias("gMyTexture"); // Ensures that render APIs that don't have separate sampler names use the same name as the texture
~~~~~~~~~~~~~~

# Blocks {#bslfx_b}
Blocks are containers for parameters. In HLSL/GLSL they're usually called constant/uniform buffers. Actual layout of the blocks is therefore defined in their source language, and using the Block command we just make them available to the high level interface. The name of the block must be the same as the corresponding constant/uniform buffer.

Parameters don't have to belong to a block but if they do you can share parameter blocks with multiple instances of @ref BansheeEngine::Material "Material", which is more efficient. Normally this is not something you need to worry about unless working low level with the renderer.

Blocks all begin with the Block keyword, followed by a name. The name must match the name of the constant/uniform block in actual shader code:
~~~~~~~~~~~~~~
	Block MyBlock;
~~~~~~~~~~~~~~

# Technique {#bslfx_c}

This is the meat of your shader. A technique contains code for your vertex/fragment/geometry/hull/domain/compute programs, as well as blend/rasterizer/depth-stencil states. A shader can contain multiple techniques but only a single technique is ever used at once. Different techniques can be specified for each shading language (e.g. HLSL, GLSL) and different renderer (in case you're using something other than the default).

## Properties {#bslfx_c_a}

Technique block should therefore always contain a "Language" property like so:
~~~~~~~~~~~~~~
	Language = "HLSL"
~~~~~~~~~~~~~~

Supported values are "HLSL" (DirectX 11 HLSL), "HLSL9" (DirectX 9 HLSL), and "GLSL".

You can also specify a renderer using the "Renderer" property:
~~~~~~~~~~~~~~
	Renderer = "Default"
~~~~~~~~~~~~~~

Supported values are "Any", or "Default". More values could be available if you are using a custom renderer, but otherwise you don't need to set this property.

Once the base properties are defined you can start defining code blocks and states.

## Code blocks {#bslfx_c_b}
Code blocks are elements in the technique that will contain HLSL/GLSL (or other supported language) code. There are six supported code blocks:
 - Vertex
 - Fragment
 - Geometry
 - Hull
 - Domain
 - Compute
 
Each corresponding to the programmable GPU pipline stage of the same name. A code block looks like this:
~~~~~~~~~~~~~~
Vertex =
{
	...raw HLSL/GLSL code...
};
~~~~~~~~~~~~~~

Within a code block BSL parsing rules do not work, anything inside them is treated as native GPU program code in the language specified. BSL expects that main entry methods for each programmable stage are named "main" (similar to the OpenGL requirement). You can use any name for non-entry methods.

Aside from the mentioned code blocks you can also use a "Common" code block. All code in the "Common" code block will be available in all other code blocks.

## States {#bslfx_c_c}
Each technique can define properties for blend, depth-stencil and rasterizer states. Read the [render API](@ref renderAPI) manual for more information about render states. See a list below of all supported properties and their accepted values:

**Rasterizer**
 - Fill = [WIRE/SOLID];
 - Cull = [CW/CCW/NOCULL];
 - DepthBias = float;
 - ScaledDepthBias = float;
 - DepthClip = [true/false];
 - Scissor = [true/false];
 - Multisample = [true/false];
 - AALine = [true/false];
 
For example:
~~~~~~~~~~~~~~
...
// Enable wireframe rendering with no culling
Fill = WIRE;
Cull = NOCULL;
...
~~~~~~~~~~~~~~
 
**Depth-stencil**
 - DepthRead = [true/false];
 - DepthWrite = [true/false];
 - CompareFunc = [FAIL/PASS/LT/GT/LTE/GTE/EQ/NEQ];
 - Stencil = [true/false];
 - StencilReadMask = int;
 - StencilWriteMask = int;
 - StencilOpFront = StencilOpBlock;
 - StencilOpBack = StencilOpBlock;
 - StencilRef = int;
 
Where StencilOpBlock has properties:
 - Fail = [KEEP/ZERO/REPLACE/INC/DEC/INCWRAP/DECWRAP/INV];
 - ZFail = [KEEP/ZERO/REPLACE/INC/DEC/INCWRAP/DECWRAP/INV];
 - Pass = [KEEP/ZERO/REPLACE/INC/DEC/INCWRAP/DECWRAP/INV];
 - CompareFunc = [FAIL/PASS/LT/GT/LTE/GTE/EQ/NEQ];
 
StencilOpBlock can also be defined in short form without having to specify property names. Parameters are ordered as listed here. For example:
 - StencilOpFront = { KEEP, KEEP, REPLACE, PASS };

For example:
~~~~~~~~~~~~~~
...
// Disable depth writes and set up a stencil test for front faces
DepthWrite = false;
Stencil = true;
StencilOpFront =
	{
		Fail = KEEP;
		ZFail = KEEP;
		Pass = INC; // Increment stencil on pass
		CompareFunc = GT; // Only pass if greater than stencil reference value
	};
// OR
// StencilOpFront = { KEEP, KEEP, INC; GT }; // Sort form
StencilRef = 2; // Reference value to perform comparison against
...
~~~~~~~~~~~~~~ 
 
**Blend state**
 - AlphaToCoverage = [true/false];
 - IndependantBlend = [true/false];
 - Target = BlendTargetBlock;
 
Where BlendTargetBlock has properties:
 - Index = int;
 - Blend = [true/false];
 - Color = BlendDefinitionBlock;
 - Alpha = BlendDefinitionBlock;
 - WriteMask = [NOCOLOR/R/G/B/A/RG/RB/RA/GB/GA/BA/RGB/RGA/RBA/GBA/RGBA];
 
Where BlendDefinitionBlock has properties:
 - Source = [ONE/ZERO/DSTRGB/SRCRGB/DSTIRGB/SRCIRGB/DSTA/SRCA/DSTIA/SRCIA];
 - Dest = [ONE/ZERO/DSTRGB/SRCRGB/DSTIRGB/SRCIRGB/DSTA/SRCA/DSTIA/SRCIA];
 - Op = [ADD/SUB/RSUB/MIN/MAX];
 
BlendDefinitionBlock can also be specified in short form similar as StencilOpBlock (parameters in order as listed here).
 
For example:
~~~~~~~~~~~~~~
...
// Set up blending so we can render transparent objects
Target = 
	{
		Index = 0; // First render target, can be ignored in this case
		Blend = true; // Enable blending
		Color = // Blend operation for color values
			{
				Source = SRCA; // Multiply source with source alpha
				Dest = SRCIA; // Multiply destination with inverse source alpha
				Op = ADD; // Add the result together
			};
		// OR
		// Color = { SRCA, SRCIA, ADD}; // Sort form
	};
...
~~~~~~~~~~~~~~  
 
Entries in brackets represent the supported keywords for the specified properties, while the other values represent data types.

## Passes {#bslfx_c_d}
Each technique can support one or multiple passes. By default you do not have to specify any passes in technique if your shader only requires a single pass. If multiple passes are required use the "Pass" block.

A "Pass" block supports all the code-block and state properties the Technique supports. It also supports an additional "Index" property that accepts an integer and allows you to specify an order in which the passes are executed. It also allows the pass to be uniquely identified if merging passes (see later sections for information about merging). By default index is not needed and pass order of execution is assumed to be sequential.

If you specify code blocks and/or states in both a Technique and a Pass block, then the values under the Technique block will be inherited by all Pass blocks of that technique.

Pass example:
~~~~~~~~~~~~~~
// Technique with two passes
Technique 
{
	Pass
	{
		Index = 0; // Optional, assumed zero since it's first in technique
		
		// Uses same states and code blocks we described for Technique
	}
	
	Pass
	{
		Index = 1; // Optional, assumed one since it's second in technique
		
		// Uses same states and code blocks we described for Technique
	}
};
~~~~~~~~~~~~~~

# Advanced {#bslfx_d}

## Block merging {#bslfx_d_a}
If multiple Techniques using the same "Language" and "Renderer" are found in the file, they will internally be merged into a single larger technique. You may also use value of "Any" for both "Language" or "Renderer" properties to allow the technique to be merged with techniques that don't match it exactly.

When merging passes within techniques, pass "Index" properties are compared and passes with the same index are merged. If no index is specified, passes are merged sequentially according to their order in the techniques.

Code blocks are also merged, in the order they are defined.

## Pre-processor {#bslfx_d_b}
Use \#include "Path/To/File.bslinc" to share code by including other BSLFX files. Included files must end with a .bslinc extension but otherwise their syntax is the same as a normal BSLFX file. The provided path is a path to the shader relative to the project library if running in editor, or relative to the working directory otherwise. You can also use built-in $ENGINE$ and $EDITOR$ folders to access builtin shaders. e.g.
~~~~~~~~~~~~~~
#include "$ENGINE$/SpriteImage.bslinc"
~~~~~~~~~~~~~~

These values correspond to "Data/Engine/Includes" and "Data/Editor/Includes" folders, respectively. Be aware that $EDITOR$ folder will not be available in your standalone game.

Use standard pre-processor commands \#define/\#undef/\#ifdef/\#ifndef/\#else/\#elif/\#endif to conditionally compile parts of the BSLFX file.

Macros using \#defines are not supported in BSLFX code, but are within code-blocks. So while you are allowed to write:
~~~~~~~~~~~~~~
#define PI 3.14
~~~~~~~~~~~~~~

Any references to PI outside of code-blocks will not be replaced with 3.14 and will likely result in an error due to an unrecognized identifier.

## Global shader properties {#bslfx_d_c}
On the top-most level you may also specify additional parameters along with "Parameters", "Blocks" and "Technique":
 - Separable = [true/false]; - Specifies if passes within the shader need to be executed sequentially, or could some other shader be executed in-between them. This is an optimization as it can allow the system to render geometry sharing the same passes all at once. False by default.
 - Sort - [FRONTTOBACK/BACKTOFRONT/NOSORT]; - Specifies in what order are the objects using this shader sorted before rendering.
 - Priority - int; - Specifies when will objects with this shader be rendered compared to other objects. Higher value means the objects will be rendered sooner. Priority has higher importance than sorting.
 - Transparent - [true/false]; - Determines whether the shader renders transparent surfaces. Allows the renderer to better handle the shader.
 
## Sampler state default values {#bslfx_d_d}
Earlier we mentioned that sampler states can be provided a set of default values in a form of their own block, but didn't specify their properties. Sampler state properties are:
 - AddressMode = AddressModeBlock;
 - MinFilter = [NOFILTER/POINT/LINEAR/ANISO/POINTC/LINEARC/ANISOC];
 - MaxFilter = [NOFILTER/POINT/LINEAR/ANISO/POINTC/LINEARC/ANISOC];
 - MipFiler = [NOFILTER/POINT/LINEAR/ANISO/POINTC/LINEARC/ANISOC];
 - MaxAniso = int;
 - MipmapBias = float;
 - MipMin = float;
 - MipMax = float;
 - BorderColor = { float, float, float, float };
 - CompareFunc = [FAIL/PASS/LT/GT/LTE/GTE/EQ/NEQ];
 
Where AddressModeBlock has the following properties:
 - U = [WRAP/MIRROR/CLAMP/BORDER];
 - V = [WRAP/MIRROR/CLAMP/BORDER];
 - W = [WRAP/MIRROR/CLAMP/BORDER];
 
It can also be specified in short form, where parameters are in order as above. For example:
 - AddressMode = { WRAP, WRAP, WRAP };
 
See @ref BansheeEngine::SamplerState "SamplerState" documentation about the meaning of these properties. 

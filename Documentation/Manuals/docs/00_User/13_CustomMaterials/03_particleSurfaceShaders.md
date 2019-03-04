Particle surface shaders					{#particleSurfaceShaders}
===============
[TOC]

Particle system supports the use of custom surface shaders with mostly the same approach as with normal surface shaders. The major difference is that instead of using the **BasePass** mixin for handling vertex shaders you use the **ParticleVertex** mixin.

# Deferred pipeline {#particleSurfaceShaders_a}
When creating particle shaders for the deferred pipeline the process is nearly identical to the normal deferred surface shader approach. The end goal is still to populate the **SurfaceData** structure.

~~~~~~~~~~~~~
#define LIGHTING_DATA
#include "$ENGINE$\ParticleVertex.bslinc"
#include "$ENGINE$\GBufferOutput.bslinc"

shader Surface
{
	mixin ParticleVertex;
	mixin GBufferOutput;

	code
	{
		SamplerState gAlbedoSamp;
		Texture2D gAlbedoTex;
	
		void fsmain(
			in VStoFS input, 
			out float4 OutSceneColor : SV_Target0,
			out float4 OutGBufferA : SV_Target1,
			out float4 OutGBufferB : SV_Target2,
			out float2 OutGBufferC : SV_Target3,
			out float OutGBufferD : SV_Target4)
		{
			SurfaceData surfaceData;
			surfaceData.albedo = gAlbedoTex.Sample(gAlbedoSamp, input.uv0);
			surfaceData.worldNormal.xyz = float3(0, 1, 0);
			surfaceData.roughness = 1.0f;
			surfaceData.metalness = 0.0f;
			surfaceData.mask = gLayer;
			
			encodeGBuffer(surfaceData, OutGBufferA, OutGBufferB, OutGBufferC, OutGBufferD);
			
			OutSceneColor = 0.0f;
		}	
	};
};
~~~~~~~~~~~~~

Note the **LIGHTING_DATA** preprocessor define and the use of **ParticleVertex** mixin. Those are the only two notable differences between normal and particle surface shaders when it comes to the deferred pipeline. **LIGHTING_DATA** tells the **ParticleVertex** mixin to provide additional information required for calculating lighting (such as normals and tangents). This is required as its possible to create particle shaders that don't account for lighting.

# Forward pipeline {#particleSurfaceShaders_b}
Once again, the forward pipeline approach is the same as with standard surface shaders except for the **LIGHTING_DATA** define and **ParticleVertex** mixin. In the example below we also add support for transparency, but this is not specific to particle surface shaders.

~~~~~~~~~~~~~
#define LIGHTING_DATA
#include "$ENGINE$\ParticleVertex.bslinc"
#include "$ENGINE$\ForwardLighting.bslinc"

// Notify the renderer this shader supports transparency
options
{
	transparent = true;
};

shader Surface
{
	mixin ParticleVertex;
	mixin ForwardLighting;
	
	// Enable blending (i.e. transparency)
	blend
	{
		target	
		{
			enabled = true;
			color = { srcA, srcIA, add };
		};
	};	
	
	// Transparent objects shouldnt write to depth
	depth
	{
		write = false;
	};
	
	code
	{
		SamplerState gAlbedoSamp;
		Texture2D gAlbedoTex;
		
		float4 fsmain(in VStoFS input) : SV_Target0
		{
			// For simplicity we only read the albedo from the texture, and assume other properties are constant
			SurfaceData surfaceData;
			surfaceData.albedo = gAlbedoTex.Sample(gAlbedoSamp, input.uv0);
			surfaceData.worldNormal.xyz = float3(0, 1, 0);
			surfaceData.roughness = 1.0f;
			surfaceData.metalness = 0.0f;
			
			float3 lighting = calcLighting(input.worldPosition.xyz, input.position, input.uv0, surfaceData);
			float opacity = surfaceData.albedo.a;
			return float4(lighting, opacity);
		}	
	};
};
~~~~~~~~~~~~~

# Unlit forward {#particleSurfaceShaders_c}
And finally the most common way particles will be renderered is with support for transparency but ignoring any lighting. This can be done simply by including **ParticleVertex** on its own with no other mixins and writing the color in the fragment shader.

~~~~~~~~~~~~~
#include "$ENGINE$\ParticleVertex.bslinc"

options
{
	transparent = true;
};

shader Surface
{
	mixin ParticleVertex;

	blend
	{
		target	
		{
			enabled = true;
			color = { srcA, srcIA, add };
		};
	};	

	depth
	{
		write = false;
	};
	
	code
	{
		SamplerState gSampler;
		Texture2D gTexture;
		
		float4 fsmain(in VStoFS input) : SV_Target0
		{
			return gTexture.Sample(gSampler, input.uv0);
		}
	};
};
~~~~~~~~~~~~~
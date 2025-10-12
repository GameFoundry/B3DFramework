---
title: Particle surface shaders
---

Particle system supports the use of custom surface shaders with mostly the same approach as with normal surface shaders. The major difference is that instead of using the **BasePass** mixin for handling vertex shaders you use the **ParticleVertex** mixin.

# Deferred pipeline
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
		[alias(gAlbedoTex)]
		SamplerState gAlbedoSamp;

		Texture2D gAlbedoTex = white;

		void fsmain(
			in VStoFS input,
			out float4 OutSceneColor : SV_Target0,
			out GBufferData OutGBuffer)
		{
			SurfaceData surfaceData;
			surfaceData.albedo = gAlbedoTex.Sample(gAlbedoSamp, input.uv0);
			surfaceData.worldNormal.xyz = input.tangentToWorldZ;
			surfaceData.roughness = 1.0f;
			surfaceData.metalness = 0.0f;
			surfaceData.mask = gLayer;
			surfaceData.velocity = 0.0f;

			OutSceneColor = 0.0f;
			OutGBuffer = encodeGBuffer(surfaceData);
		}
	};
};
~~~~~~~~~~~~~

Note the **LIGHTING_DATA** preprocessor define and the use of **ParticleVertex** mixin. Those are the only two notable differences between normal and particle surface shaders when it comes to the deferred pipeline. **LIGHTING_DATA** tells the **ParticleVertex** mixin to provide additional information required for calculating lighting (such as normals and tangents). This is required as its possible to create particle shaders that don't account for lighting.

# Forward pipeline
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

	// Transparent objects shouldn't write to depth
	depth
	{
		write = false;
	};

	code
	{
		[alias(gAlbedoTex)]
		SamplerState gAlbedoSamp;

		[alias(gNormalTex)]
		SamplerState gNormalSamp;

		Texture2D gAlbedoTex = white;
		Texture2D gNormalTex = normal;

		cbuffer MaterialParams
		{
			float gOpacity = 1.0f;
		}

		float4 fsmain(in VStoFS input) : SV_Target0
		{
			float3 normal = normalize(gNormalTex.Sample(gNormalSamp, input.uv0).xyz * 2.0f - float3(1, 1, 1));
			float3 worldNormal = calcWorldNormal(input, normal);

			SurfaceData surfaceData;
			surfaceData.albedo = gAlbedoTex.Sample(gAlbedoSamp, input.uv0) * input.color;
			surfaceData.worldNormal.xyz = worldNormal;
			surfaceData.worldNormal.w = 1.0f;
			surfaceData.roughness = 1.0f;
			surfaceData.metalness = 0.0f;

			float3 lighting = calcLighting(input.worldPosition.xyz, input.position, input.uv0, surfaceData);
			return float4(lighting, surfaceData.albedo.a * gOpacity);
		}
	};
};
~~~~~~~~~~~~~

# Unlit forward
And finally the most common way particles will be rendered is with support for transparency but ignoring any lighting. This can be done simply by including **ParticleVertex** on its own with no other mixins and writing the color in the fragment shader.

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
		[alias(gTexture)]
		SamplerState gSampler;

		Texture2D gTexture = white;

		float4 fsmain(in VStoFS input) : SV_Target0
		{
			return gTexture.Sample(gSampler, input.uv0) * input.color;
		}
	};
};
~~~~~~~~~~~~~

Note that in the particle shaders, the **input.color** field contains the per-particle color that can be set via the particle system. This is often multiplied with the texture sample to tint particles dynamically.

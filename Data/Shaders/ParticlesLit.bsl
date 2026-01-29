#define LIGHTING_DATA
#include "$ENGINE$\ParticleVertex.bslinc"
#include "$ENGINE$\ForwardLighting.bslinc"

options
{
	transparent = true;
};

shader Surface
{
	mixin ParticleVertex;
	mixin ForwardLighting;

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
		[alias(gAlbedoTex)]
		SamplerState gAlbedoSamp;
		
		[alias(gNormalTex)]
		SamplerState gNormalSamp;
		
		[alias(gRoughnessTex)]
		SamplerState gRoughnessSamp;
		
		[alias(gMetalnessTex)]
		SamplerState gMetalnessSamp;
	
		[alias(gEmissiveMaskTex)]
		SamplerState gEmissiveMaskSamp;	
		
		Texture2D gAlbedoTex = white;
		Texture2D gNormalTex = normal;
		Texture2D gRoughnessTex = white;
		Texture2D gMetalnessTex = black;
		Texture2D gEmissiveMaskTex = black;
		
		cbuffer MaterialParams
		{
			float gOpacity = 1.0f;
			[color][hdr]
			float3 gEmissiveColor = { 1.0f, 1.0f, 1.0f };
			float2 gUVOffset = { 0.0f, 0.0f };
			float2 gUVTile = { 1.0f, 1.0f };
		}
		
		float4 fsmain(in VStoFS input) : SV_Target0
		{
			float2 uv = input.uv0 * gUVTile + gUVOffset;
		
			float3 normal = normalize(gNormalTex.Sample(gNormalSamp, uv).xyz * 2.0f - float3(1, 1, 1));
			float3 worldNormal = calcWorldNormal(input, normal);
		
			SurfaceData surfaceData;
			surfaceData.albedo = gAlbedoTex.Sample(gAlbedoSamp, uv) * input.color;
			surfaceData.worldNormal.xyz = worldNormal;
			surfaceData.worldNormal.w = 1.0f;
			surfaceData.roughness = gRoughnessTex.Sample(gRoughnessSamp, uv).x;
			surfaceData.metalness = gMetalnessTex.Sample(gMetalnessSamp, uv).x;
			
			float3 lighting = calcLighting(input.worldPosition.xyz, input.position, uv, surfaceData);
			float3 emissive = gEmissiveColor * gEmissiveMaskTex.Sample(gEmissiveMaskSamp, uv).x;
			return float4(emissive + lighting, surfaceData.albedo.a * gOpacity);
		}	
	};
};

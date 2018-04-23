#include "$ENGINE$\BasePass.bslinc"
#include "$ENGINE$\ForwardLighting.bslinc"

options
{
	transparent = true;
};

shader Surface
{
	mixin BasePass;
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
		SamplerState gAlbedoSamp;
		SamplerState gNormalSamp;
		SamplerState gRoughnessSamp;
		SamplerState gMetalnessSamp;
		
		Texture2D gAlbedoTex = white;
		Texture2D gNormalTex = normal;
		Texture2D gRoughnessTex = white;
		Texture2D gMetalnessTex = black;
		
		cbuffer MaterialParams
		{
			float gOpacity = 1.0f;
		}
		
		float4 fsmain(in VStoFS input) : SV_Target0
		{
			float3 normal = normalize(gNormalTex.Sample(gNormalSamp, input.uv0).xyz * 2.0f - float3(1, 1, 1));
			float3 worldNormal = calcWorldNormal(input, normal);
		
			SurfaceData surfaceData;
			surfaceData.albedo = gAlbedoTex.Sample(gAlbedoSamp, input.uv0);
			surfaceData.worldNormal.xyz = worldNormal;
			surfaceData.worldNormal.w = 1.0f;
			surfaceData.roughness = gRoughnessTex.Sample(gRoughnessSamp, input.uv0).x;
			surfaceData.metalness = gMetalnessTex.Sample(gMetalnessSamp, input.uv0).x;
			
			float3 lighting = calcLighting(input.worldPosition.xyz, input.position, input.uv0, surfaceData);
			return float4(lighting, surfaceData.albedo.a * gOpacity);
		}	
	};
};
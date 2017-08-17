#include "$ENGINE$\BasePass.bslinc"
#include "$ENGINE$\LightGridCommon.bslinc"
#include "$ENGINE$\ReflectionCubemapCommon.bslinc"
#define USE_LIGHT_GRID_INDICES 1
#include "$ENGINE$\LightingCommon.bslinc"
#include "$ENGINE$\ImageBasedLighting.bslinc"
#include "$ENGINE$\Surface.bslinc"

options
{
	transparent = true;
};

mixin Surface
{
	mixin LightingCommon;
	mixin LightGridCommon;
	mixin ReflectionCubemapCommon;
	mixin ImageBasedLighting;

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
		SamplerState gAlbedoSamp : register(s0);
		SamplerState gNormalSamp : register(s1);
		SamplerState gRoughnessSamp : register(s2);
		SamplerState gMetalnessSamp : register(s3);
		
		Texture2D gAlbedoTex : register(t0);
		Texture2D gNormalTex : register(t1);
		Texture2D gRoughnessTex : register(t2);
		Texture2D gMetalnessTex : register(t3);
		
		Buffer<uint4> gGridLightOffsetsAndSize;
		Buffer<uint2> gGridProbeOffsetsAndSize;

		cbuffer MaterialParams : register(b5)
		{
			float gOpacity;
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
			
			uint2 pixelPos = (uint2)input.position.xy;
			uint cellIdx = calcCellIdx(pixelPos, input.position.z);
			uint3 lightOffsetAndSize = gGridLightOffsetsAndSize[cellIdx].rgb;
			
			uint4 lightOffsets;
			lightOffsets.x = gLightCounts.x;
			lightOffsets.y = lightOffsetAndSize.x;
			lightOffsets.z = lightOffsets.y + lightOffsetAndSize.y;
			lightOffsets.w = lightOffsets.z + lightOffsetAndSize.z;
			
			uint2 reflProbeOffsetAndSize = gGridProbeOffsetsAndSize[cellIdx];
			
			float3 V = normalize(gViewOrigin - input.worldPosition);
			float3 N = surfaceData.worldNormal.xyz;
			float3 R = 2 * dot(V, N) * N - V;
			float3 specR = getSpecularDominantDir(N, R, surfaceData.roughness);
			
			float4 directLighting = getDirectLighting(input.worldPosition, V, specR, surfaceData, lightOffsets);
			float ao = gAmbientOcclusionTex.Sample(gAlbedoSamp, input.uv0);
			float4 ssr = gSSRTex.Load(int3(pixelPos.xy, 0));
			float3 imageBasedSpecular = getImageBasedSpecular(input.worldPosition, V, specR, surfaceData, ao, ssr,
				reflProbeOffsetAndSize.x, reflProbeOffsetAndSize.y);

			float3 totalLighting = directLighting.rgb;
			totalLighting.rgb += imageBasedSpecular;

			return float4(totalLighting, gOpacity);
		}	
	};
};
#if MSAA
#define MSAA_COUNT 2
#else
#define MSAA_COUNT 1
#endif

#include "$ENGINE$\GBufferInput.bslinc"
#include "$ENGINE$\PPBase.bslinc"
#include "$ENGINE$\PerCameraData.bslinc"
#include "$ENGINE$\ImageBasedLighting.bslinc"
#include "$ENGINE$\DirectLighting.bslinc"

shader DeferredIBLFinalize
{
	mixin PPBase;
	mixin GBufferInput;
	mixin PerCameraData;
	mixin ImageBasedLighting;
	mixin DirectLighting;

	blend
	{
		target
		{
			enabled = true;
			color = { dstA, one, add };		
		};
	};
	
	variations
	{
		MSAA = { true, false };
		MSAA_RESOLVE_0TH = { true, false };
	};		
	
	#if MSAA
	stencil
	{
		enabled = true;
		readmask = 0x80;
		
		#if INSIDE_GEOMETRY
		back = { keep, keep, keep, eq };
		#else
		front = { keep, keep, keep, eq };
		#endif
		
		#if MSAA_RESOLVE_0TH
		reference = 0;
		#else
		reference = 0x80;
		#endif
	};
	#endif
	
	code
	{
		float4 fsmain(VStoFS input, float4 pixelPos : SV_Position
			#if MSAA_COUNT > 1 && !MSAA_RESOLVE_0TH
			, uint sampleIdx : SV_SampleIndex
			#endif
			) : SV_Target0
		{		
			#if MSAA_COUNT > 1
				#if MSAA_RESOLVE_0TH
					SurfaceData surfaceData = getGBufferData((uint2)pixelPos.xy, 0);
				#else
					SurfaceData surfaceData = getGBufferData((uint2)pixelPos.xy, sampleIdx);
				#endif
			#else
				SurfaceData surfaceData = getGBufferData((uint2)pixelPos.xy);
			#endif	

			if(surfaceData.worldNormal.w > 0.0f && gSkyCubemapAvailable > 0 && gUseReflectionMaps != 0)
			{
				float3 worldPosition = NDCToWorld(input.screenPos, surfaceData.depth);
				
				float3 V = normalize(gViewOrigin - worldPosition);
				float3 N = surfaceData.worldNormal.xyz;
				float3 R = 2 * dot(V, N) * N - V;
				float3 specR = getSpecularDominantDir(N, R, surfaceData.roughness);
			
				float skyMipLevel = mapRoughnessToMipLevel(surfaceData.roughness, gSkyCubemapNumMips);
				float4 skySample = gSkyReflectionTex.SampleLevel(gSkyReflectionSamp, specR, skyMipLevel) * gSkyBrightness;
				
				return float4(skySample.rgb, 1.0f); 
			}
			else
				return float4(0.0f, 0.0f, 0.0f, 0.0f);
		}
	};
};

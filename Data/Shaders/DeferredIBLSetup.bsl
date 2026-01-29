#if MSAA
#define MSAA_COUNT 2
#else
#define MSAA_COUNT 1
#endif

#include "$ENGINE$\GBufferInput.bslinc"
#include "$ENGINE$\PPBase.bslinc"
#include "$ENGINE$\PerCameraData.bslinc"
#include "$ENGINE$\ImageBasedLighting.bslinc"

shader DeferredIBLSetup
{
	mixin PPBase;
	mixin GBufferInput;
	mixin PerCameraData;
	mixin ImageBasedLighting;
	
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

			if(surfaceData.worldNormal.w > 0.0f)
			{
				float3 worldPosition = NDCToWorld(input.screenPos, surfaceData.depth);

				float ao = gAmbientOcclusionTex.SampleLevel(gAmbientOcclusionSamp, input.uv0, 0.0f).r;
				float4 ssr = gSSRTex.SampleLevel(gSSRSamp, input.uv0, 0.0f);
				
				float3 V = normalize(gViewOrigin - worldPosition);
				float3 N = surfaceData.worldNormal.xyz;
				float NoV = saturate(dot(N, V));
				
				float3 radiance = ssr.rgb;
				float alpha = 1.0f - ssr.a; // Determines how much to blend in reflection probes & skybox
				
				// Generate an approximate spec. occlusion value from AO. This doesn't need to be applied to SSR since it accounts
				// for occlusion by tracing rays.
				float specOcclusion = getSpecularOcclusion(NoV, surfaceData.roughness * surfaceData.roughness, ao);
				alpha *= specOcclusion;
				
				if(gUseReflectionMaps == 0)
				{
					// Note: Using a fixed F0 value of 0.04 (plastic) for dielectrics, and using albedo as specular for conductors.
					// For more customizability allow the user to provide separate albedo/specular colors for both types.
					float3 specularColor = lerp(float3(0.04f, 0.04f, 0.04f), surfaceData.albedo.rgb, surfaceData.metalness);								
					radiance += specularColor;
				}

				return float4(radiance, alpha);
			}
			else
				return float4(0.0f, 0.0f, 0.0f, 0.0f);
		}
	};
};

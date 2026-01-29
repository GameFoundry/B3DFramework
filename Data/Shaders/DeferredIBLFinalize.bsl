#if MSAA
#define MSAA_COUNT 2
#else
#define MSAA_COUNT 1
#endif

#include "$ENGINE$\GBufferInput.bslinc"
#include "$ENGINE$\PPBase.bslinc"
#include "$ENGINE$\PerCameraData.bslinc"
#include "$ENGINE$\ImageBasedLighting.bslinc"

shader DeferredIBLFinalize
{
	mixin PPBase;
	mixin GBufferInput;
	mixin PerCameraData;
	mixin ImageBasedLighting;

	blend
	{
		target
		{
			enabled = true;
			color = { one, one, add };		
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
		#if MSAA_COUNT > 1
			Texture2DMS<float4>	gIBLRadianceTex;
		#else
			Texture2D gIBLRadianceTex;
		#endif
	
		float4 fsmain(VStoFS input, float4 pixelPos : SV_Position
			#if MSAA_COUNT > 1 && !MSAA_RESOLVE_0TH
			, uint sampleIdx : SV_SampleIndex
			#endif
			) : SV_Target0
		{		
			#if MSAA_COUNT > 1
				#if MSAA_RESOLVE_0TH
					SurfaceData surfaceData = getGBufferData((uint2)pixelPos.xy, 0);
					float3 radiance = gIBLRadianceTex.Load((uint2)pixelPos.xy, 0).rgb;
				#else
					SurfaceData surfaceData = getGBufferData((uint2)pixelPos.xy, sampleIdx);
					float3 radiance = gIBLRadianceTex.Load((uint2)pixelPos.xy, sampleIdx).rgb;
				#endif
			#else
				SurfaceData surfaceData = getGBufferData((uint2)pixelPos.xy);
				float3 radiance = gIBLRadianceTex.Load(int3((int2)pixelPos.xy, 0)).rgb;
			#endif	

			if(surfaceData.worldNormal.w > 0.0f)
			{
				// See C++ code for generation of gPreintegratedEnvBRDF to see why this code works as is
				float3 worldPosition = NDCToWorld(input.screenPos, surfaceData.depth);
				
				float3 V = normalize(gViewOrigin - worldPosition);
				float3 N = surfaceData.worldNormal.xyz;
				float NoV = saturate(dot(N, V));
				
				// Note: Using a fixed F0 value of 0.04 (plastic) for dielectrics, and using albedo as specular for conductors.
				// For more customizability allow the user to provide separate albedo/specular colors for both types.
				float3 specularColor = lerp(float3(0.04f, 0.04f, 0.04f), surfaceData.albedo.rgb, surfaceData.metalness);
								
				float2 envBRDF = gPreintegratedEnvBRDF.SampleLevel(gPreintegratedEnvBRDFSamp, float2(NoV, surfaceData.roughness), 0).rg;
				
				return float4(radiance * (specularColor * envBRDF.x + envBRDF.y), 0.0f);
			}
			else
				return float4(0.0f, 0.0f, 0.0f, 0.0f);
		}
	};
};

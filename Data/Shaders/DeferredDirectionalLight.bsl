#if MSAA
#define MSAA_COUNT 2
#else
#define MSAA_COUNT 1
#endif

#include "$ENGINE$\DeferredLightCommon.bslinc"

shader DeferredDirectionalLight
{
	mixin DeferredLightCommon;

	variations
	{
		MSAA = { true, false };
		MSAA_RESOLVE_0TH = { true, false };
	};	
	
	depth
	{
		read = false;
	};
	
	#if MSAA
	stencil
	{
		enabled = true;
		front = { keep, keep, keep, eq };
		readmask = 0x80;
		
		#if MSAA_RESOLVE_0TH
		reference = 0;
		#else
		reference = 0x80;
		#endif
	};
	#endif
	
	code 
	{
		struct VStoFS
		{
			float4 position : SV_POSITION;
			float2 uv0 : TEXCOORD0;
			float4 screenPos : TEXCOORD1;
		};

		struct VertexInput
		{
			float2 screenPos : POSITION;
			float2 uv0 : TEXCOORD0;
		};
		
		VStoFS vsmain(VertexInput input)
		{
			VStoFS output;
		
			output.position = float4(input.screenPos, 0, 1);
			output.screenPos = float4(input.screenPos, 0, 1);
			output.uv0 = input.uv0;
		
			return output;
		}

		float4 fsmain(VStoFS input
			#if MSAA_COUNT > 1 && !MSAA_RESOLVE_0TH
			, uint sampleIdx : SV_SampleIndex
			#endif
			) : SV_Target0
		{
			uint2 pixelPos = UVToScreen(input.uv0);
			
			#if MSAA_COUNT > 1
				#if MSAA_RESOLVE_0TH
					SurfaceData surfaceData = getGBufferData(pixelPos, 0);
				#else
					SurfaceData surfaceData = getGBufferData(pixelPos, sampleIdx);
				#endif
			#else
			SurfaceData surfaceData = getGBufferData(pixelPos);
			#endif			
		
			if(surfaceData.worldNormal.w > 0.0f)
			{
				float2 ndcPos = input.screenPos.xy / input.screenPos.w;
				float3 worldPosition = NDCToWorld(ndcPos, surfaceData.depth);

				float3 V = normalize(gViewOrigin - worldPosition);
				float3 N = surfaceData.worldNormal.xyz;
				float3 R = 2 * dot(V, N) * N - V;
				float3 specR = getSpecularDominantDir(N, R, surfaceData.roughness);
								
				LightData lightData = getLightData();
				
				#if MSAA_COUNT > 1
					#if MSAA_RESOLVE_0TH
						float occlusion = gLightOcclusionTex.Load(pixelPos, 0).r;
					#else
						float occlusion = gLightOcclusionTex.Load(pixelPos, sampleIdx).r;
					#endif
				#else
				float occlusion = gLightOcclusionTex.Load(int3(pixelPos, 0)).r;
				#endif
				
				// Reverse the sqrt we did when storing it
				occlusion *= occlusion;
				
				occlusion = 1.0f - occlusion;
				
				return float4(getLuminanceDirectional(lightData, worldPosition, V, specR, surfaceData) * occlusion, 1.0f);
			}
			else
				return float4(0.0f, 0.0f, 0.0f, 0.0f);
		}
	};
};

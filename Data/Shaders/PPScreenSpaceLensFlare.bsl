#include "$ENGINE$\PPBase.bslinc"
#include "$ENGINE$\ColorSpace.bslinc"

shader PPScreenSpaceLensFlare
{
	mixin PPBase;
	mixin ColorSpace;
	
	variations
	{
		// 0 - No halo, 1 - Halo with aspect, 2 - Halo without aspect
		HALO_MODE = { 0, 1, 2 };
		CHROMATIC_ABERRATION = { true, false };
	};
	
	code
	{
		[internal]
		cbuffer Input
		{
			float gThreshold;
			float gGhostSpacing;
			uint gGhostCount;
			
			float gHaloRadius;
			float gHaloThickness;
			float gHaloThreshold;
			float gHaloAspectRatio;
			
			float gChromaticAberration;
		}		

		Texture2D gInputTex;
		
		[alias(gInputTex)]
		SamplerState gInputSamp;
		
		Texture2D gGradientTex;
		
		[alias(gGradientTex)]
		SamplerState gGradientSamp
		{
			AddressU = CLAMP;
			AddressV = CLAMP;
			AddressW = CLAMP;
		};
		
		float3 sampleSceneColor(float2 uv)
		{
			#if CHROMATIC_ABERRATION
				float2 chromaticOffset = normalize(0.5f - uv) * gChromaticAberration;
				return float3(
						gInputTex.SampleLevel(gInputSamp, uv + chromaticOffset, 0.0f).r,
						gInputTex.SampleLevel(gInputSamp, uv, 0.0f).g,
						gInputTex.SampleLevel(gInputSamp, uv - chromaticOffset, 0.0f).b
					);
			#else
				return gInputTex.SampleLevel(gInputSamp, uv, 0.0f).rgb;
			#endif
		}
		
		float windowCubic(float x, float center, float radius)
		{
			x = min(abs(x - center) / radius, 1.0f);
			return 1.0f - x * x * (3.0f - 2.0f * x);
		}
		
		float4 fsmain(VStoFS input) : SV_Target0
		{
			float4 output = 0;
			float2 uv = 1.0f - input.uv0; // Flip UV
			
			float2 ghostVec = (0.5f - uv) * gGhostSpacing;
			for (uint i = 0; i < gGhostCount; ++i) 
			{
				float2 ghostUV = frac(uv + ghostVec * i);
				float3 sceneColor = sampleSceneColor(ghostUV);
				
				sceneColor = max(sceneColor - gThreshold, 0.0f);
				
				float distanceToCenter = distance(ghostUV, 0.5f);
				sceneColor *= gGradientTex.Sample(gGradientSamp, float2(distanceToCenter, 0.5)).rgb;
				
				output += float4(sceneColor, 0.0f);
			}
			
			// Halo
			#if HALO_MODE != 0
				float2 haloVec = 0.5f - uv;
				#if HALO_MODE == 2
					haloVec = normalize(haloVec);
					float haloWeight = distance(uv, 0.5f);
				#else
					haloVec.x /= gHaloAspectRatio;
					haloVec = normalize(haloVec);
					haloVec.x *= gHaloAspectRatio;
					float2 wuv = (uv - float2(0.5f, 0.0f)) / float2(gHaloAspectRatio, 1.0f) + float2(0.5f, 0.0f);
					float haloWeight = distance(wuv, 0.5f);
				#endif
				
				haloVec *= gHaloRadius;
				haloWeight = windowCubic(haloWeight, gHaloRadius, gHaloThickness);
				
				float3 sceneColor = sampleSceneColor(uv + haloVec);
				sceneColor = max(sceneColor - gHaloThreshold, 0.0f);
				output += float4(sceneColor * haloWeight, 0.0f);				
			#endif
			
			return output;
		}	
	};
};

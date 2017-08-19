#include "$ENGINE$\PPBase.bslinc"
#include "$ENGINE$\GBufferInput.bslinc"
#include "$ENGINE$\PerCameraData.bslinc"

#define TEMPORAL_LOCAL_VELOCITY 0
#define TEMPORAL_SEARCH_NEAREST 0
#define TEMPORAL_BLEND_FACTOR 8
#define TEMPORAL_SMOOTH_NEIGHBORHOOD 0
#include "$ENGINE$\TemporalResolve.bslinc"

technique PPSSRStencil
{
	mixin PPBase;
	mixin PerCameraData;
	mixin TemporalResolve;

	code
	{
		[internal]
		cbuffer Input
		{
			float2 gSceneDepthTexelSize;
			float2 gSceneColorTexelSize;
		
			float gManualExposure;
		}
		
		#if MSAA
			Texture2DMS gSceneDepth;
			Texture2DMS gSceneColor;
			Texture2DMS gPrevColor;
		#else
			Texture2D gSceneDepth;
			Texture2D gSceneColor;
			Texture2D gPrevColor;

			SamplerState gPointSampler;
			SamplerState gLinearSampler;
		#endif		
		
		#if EYE_ADAPTATION
			Texture2D gEyeAdaptationTex;
		#endif
		
		float3 fsmain(VStoFS input) : SV_Target0
		{
			float exposureScale;
			#if EYE_ADAPTATION
				exposureScale = gEyeAdaptationTex.Load(int3(0, 0, 0)).r;
			#else
				exposureScale = gManualExposure;
			#endif
		
			#if MSAA
				return temporalResolve(gSceneDepth, gSceneColor, gPrevColor, 
					exposureScale, input.uv0, input.screenPos, 0);
			#else
				return temporalResolve(
					gSceneDepth, gPointSampler, gSceneDepthTexelSize,
					gSceneColor, gLinearSampler, gSceneColorTexelSize, 
					gPrevColor, gLinearSampler, gSceneColorTexelSize,
					exposureScale, input.uv0, input.screenPos, 0);
			#endif
		}	
	};
};
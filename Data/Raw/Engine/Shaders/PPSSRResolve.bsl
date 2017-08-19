#include "$ENGINE$\PPBase.bslinc"
#include "$ENGINE$\GBufferInput.bslinc"
#include "$ENGINE$\PerCameraData.bslinc"

#define TEMPORAL_LOCAL_VELOCITY 0
#define TEMPORAL_SEARCH_NEAREST 0
#define TEMPORAL_BLEND_FACTOR 8
#define TEMPORAL_SMOOTH_NEIGHBORHOOD 0
#define MSAA_COLOR 0
#include "$ENGINE$\TemporalResolve.bslinc"

technique PPSSRResolve
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
		#else
			Texture2D gSceneDepth;
		#endif	

		Texture2D gSceneColor;
		Texture2D gPrevColor;

		SamplerState gPointSampler;
		SamplerState gLinearSampler;		
		
		float3 fsmain(VStoFS input) : SV_Target0
		{
			#if MSAA
				return temporalResolve(
					gSceneDepth, 
					gSceneColor, gLinearSampler, gSceneColorTexelSize, 
					gPrevColor, gLinearSampler, gSceneColorTexelSize,
					gManualExposure, input.position.xy, input.screenPos, 0);
			#else
				return temporalResolve(
					gSceneDepth, gPointSampler, gSceneDepthTexelSize,
					gSceneColor, gLinearSampler, gSceneColorTexelSize, 
					gPrevColor, gLinearSampler, gSceneColorTexelSize,
					gManualExposure, input.uv0, input.screenPos, 0);
			#endif
		}	
	};
};
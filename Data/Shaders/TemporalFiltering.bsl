#include "$ENGINE$\PPBase.bslinc"
#include "$ENGINE$\GBufferInput.bslinc"
#include "$ENGINE$\PerCameraData.bslinc"

#if TYPE == 0
	#define TEMPORAL_SEARCH_NEAREST 1
	#define TEMPORAL_BICUBIC_HISTORY 0
	#define TEMPORAL_YCOCG 1
	#define TEMPORAL_LOWPASS 0
	#define TEMPORAL_SMOOTH_NEIGHBORHOOD 0
	#define TEMPORAL_FILTER_COLOR 0
	#define TEMPORAL_BLEND_FACTOR 25
	#define MSAA_COLOR 0
#elif TYPE == 1
	#define TEMPORAL_SEARCH_NEAREST 0
	#define TEMPORAL_BLEND_FACTOR 8
	#define TEMPORAL_SMOOTH_NEIGHBORHOOD 0
	#define MSAA_COLOR 0
#endif

#if PER_PIXEL_VELOCITY
	#define TEMPORAL_LOCAL_VELOCITY 1
#else
	#define TEMPORAL_LOCAL_VELOCITY 0
#endif

#include "$ENGINE$\TemporalResolve.bslinc"

shader TemporalFiltering
{
	mixin PPBase;
	mixin PerCameraData;
	mixin TemporalResolve;

	variations
	{
		MSAA = { true, false };
		PER_PIXEL_VELOCITY = { true, false };
		TYPE = 
		{
			0, // Full screen AA
			1, // SSR
		};
	};
	
	code
	{
		[internal]
		cbuffer Input
		{
			float4 gSceneDepthTexelSize;
			float4 gSceneColorTexelSize;
			float4 gVelocityTexelSize;
		
			float gManualExposure;
		}
		
		#if MSAA
			Texture2DMS<float> gSceneDepth;
			
			#if PER_PIXEL_VELOCITY
				Texture2DMS<float> gVelocity;
			#endif
		#else
			Texture2D gSceneDepth;
			
			#if PER_PIXEL_VELOCITY
				Texture2D gVelocity;
			#endif
		#endif	

		Texture2D gSceneColor;
		Texture2D gPrevColor;

		SamplerState gPointSampler;
		SamplerState gLinearSampler;		
		
		float4 fsmain(VStoFS input) : SV_Target0
		{
			#if MSAA
				return temporalResolve(
					gSceneDepth, 
					gSceneColor, gLinearSampler, gSceneColorTexelSize, 
					gPrevColor, gLinearSampler, gSceneColorTexelSize,
				#if PER_PIXEL_VELOCITY
					gVelocity,
				#endif
					gManualExposure, input.uv0, input.screenPos, 0);
			#else
				return temporalResolve(
					gSceneDepth, gPointSampler, gSceneDepthTexelSize,
					gSceneColor, gLinearSampler, gSceneColorTexelSize, 
					gPrevColor, gLinearSampler, gSceneColorTexelSize,
				#if PER_PIXEL_VELOCITY
					gVelocity, gPointSampler, gVelocityTexelSize,
				#endif
					gManualExposure, input.uv0, input.screenPos, 0);
			#endif
		}	
	};
};

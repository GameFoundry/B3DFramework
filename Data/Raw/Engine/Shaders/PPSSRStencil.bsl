#include "$ENGINE$\PPBase.bslinc"
#include "$ENGINE$\GBufferInput.bslinc"
#include "$ENGINE$\PerCameraData.bslinc"

technique PPSSRStencil
{
	mixin PPBase;
	mixin PerCameraData;
	mixin GBufferInput;

	variations
	{
		MSAA_COUNT = { 1, 2 };
		MSAA_RESOLVE_0TH = { true, false };
	};
	
	stencil
	{
		enabled = true;
		reference = 1;
		front = { replace, replace, replace, always };
		writemask = 0x7F;
	};		
	
	code
	{
		[internal]
		cbuffer Input
		{
			float2 gRoughnessScaleBias;
		}
		
		float fsmain(VStoFS input			
		#if MSAA_COUNT > 1 && !MSAA_RESOLVE_0TH
			, uint sampleIdx : SV_SampleIndex
		#endif
		) : SV_Target0
		{
			#if MSAA_COUNT > 1 
				#if MSAA_RESOLVE_0TH
					SurfaceData surfData = getGBufferData(trunc(input.uv0.xy), 0);
				#else
					SurfaceData surfData = getGBufferData(trunc(input.uv0.xy), sampleIdx);
				#endif
			#else
				SurfaceData surfData = getGBufferData(input.uv0);
			#endif
			
			// Surfaces that are too rough fall back to refl. probes
			float fadeValue = 1.0f - saturate(surfData.roughness * gRoughnessScaleBias.x + gRoughnessScaleBias.y);
			
			// Reflection contribution is too low for dieletrics to waste performance on high quality reflections
			// 0 if metalness <= 0.4
			// [0, 1] if metalness > 0.4 && < 0.6
			// 1 if metalness >= 0.6
			fadeValue *= saturate(surfData.metalness * 5 - 2);
			
			if(fadeValue > 0.0f)
				discard;
			
			return 0.0f;
		}	
	};
};
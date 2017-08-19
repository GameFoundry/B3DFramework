#include "$ENGINE$\PPBase.bslinc"
#include "$ENGINE$\GBufferInput.bslinc"
#include "$ENGINE$\PerCameraData.bslinc"

technique PPSSRStencil
{
	mixin PPBase;
	mixin PerCameraData;
	mixin GBufferInput;

	stencil
	{
		enabled = true;
		reference = 1;
		front = { replace, replace, replace, always };
	};		
	
	code
	{
		[internal]
		cbuffer Input
		{
			float2 gRoughnessScaleBias;
		}
		
		float fsmain(VStoFS input			
		#if MSAA_COUNT > 1 
			, uint sampleIdx : SV_SampleIndex
		#endif
		) : SV_Target0
		{
			#if MSAA_COUNT > 1 
			SurfaceData surfData = getGBufferData(trunc(input.position.xy), sampleIdx);
			#else
			SurfaceData surfData = getGBufferData(input.uv0);
			#endif
			
			// Surfaces that are too rough fall back to refl. probes
			float fadeValue = 1.0f - saturate(surfData.roughness * gRoughnessScaleBias.x + gRoughnessScaleBias.y);
			
			if(fadeValue > 0.0f)
				discard;
			
			return 0.0f;
		}	
	};
};
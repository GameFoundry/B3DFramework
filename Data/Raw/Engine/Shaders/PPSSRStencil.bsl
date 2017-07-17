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
		
		float fsmain(VStoFS input) : SV_Target0
		{
			// TODO - Support MSAA?
		
			SurfaceData surfData = getGBufferData(input.uv0);
			
			// Surfaces that are too rough fall back to refl. probes
			float fadeValue = 1.0f - saturate(surfData.roughness * gRoughnessScaleBias.x + gRoughnessScaleBias.y);
			
			if(fadeValue > 0.0f)
				discard;
			
			return 0.0f;
		}	
	};
};
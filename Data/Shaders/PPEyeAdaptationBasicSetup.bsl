#include "$ENGINE$\PPBase.bslinc"
#include "$ENGINE$\PPEyeAdaptationCommon.bslinc"

shader PPEyeAdaptationBasicSetup
{
	mixin PPBase;
	mixin PPEyeAdaptationParams;

	code
	{
		Texture2D gInputTex;
		
		[alias(gInputTex)]
		SamplerState gInputSamp;
		
		float4 fsmain(VStoFS input) : SV_Target0
		{
			float4 value = gInputTex.Sample(gInputSamp, input.uv0);
			float luminance = dot(value.xyz, float3(0.2126, 0.7152, 0.0722));
			
			float maxIntensity = gEyeAdaptationParams[2].z;
			luminance = max(maxIntensity, luminance);
			
			// Store intensity as log, and scale to [0, 1] range
			value.w = gEyeAdaptationParams[0].x * log2(luminance) + gEyeAdaptationParams[0].y;
			
			return value;
		}	
	};
};

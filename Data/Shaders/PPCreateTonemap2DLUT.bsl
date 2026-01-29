#include "$ENGINE$\PPCreateTonemapLUTCommon.bslinc"
#include "$ENGINE$\PPBase.bslinc"

shader PPCreateTonemap2DLUT
{
	mixin PPCreateTonemapLUTCommon;
	mixin PPBase;
	
	code
	{
		float4 fsmain(VStoFS input) : SV_Target0
		{
			float3 logColor;
			
			float2 uv = input.uv0;
			// Make sure uv maps to [0,1], as it's currently specified for pixel centers
			// (This way we get non-extrapolated color values for 0 and 1)
			uv -= float2(0.5f / (LUT_SIZE * LUT_SIZE), 0.5f / LUT_SIZE);
			uv *= LUT_SIZE / (LUT_SIZE - 1);
			
			// Red goes from 0 to 1, in each slice along X (LUT_SIZE number of slices)
			logColor.r = frac(uv.x * LUT_SIZE);
			
			// Blue value is constant within each slice, and increases by 1/LUT_SIZE with each slice along X
			logColor.b = uv.x - logColor.r / LUT_SIZE;
			
			// Green increases linearly with y
			logColor.g = uv.y;
			
			float3 gammaColor = TonemapColor(logColor);
							
			// TODO - Divide by 1.05f here and then re-apply it when decoding from the texture?
			return float4(gammaColor, 1.0f);	
		}	
	};
};

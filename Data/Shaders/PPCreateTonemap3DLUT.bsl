#include "$ENGINE$\PPCreateTonemapLUTCommon.bslinc"

shader PPCreateTonemap3DLUT
{
	mixin PPCreateTonemapLUTCommon;
	
	featureset = HighEnd;
		
	code
	{
		[layout(rgba8)]
		RWTexture3D<unorm float4> gOutputTex;
		
		[numthreads(8, 8, 1)]
		void csmain(
			uint3 dispatchThreadId : SV_DispatchThreadID,
			uint threadIndex : SV_GroupIndex)
		{
			float3 logColor = float3(dispatchThreadId.xyz / (float)(LUT_SIZE - 1));
			float3 gammaColor = TonemapColor(logColor);
							
			// TODO - Divide by 1.05f here and then re-apply it when decoding from the texture?
			gOutputTex[dispatchThreadId] = float4(gammaColor, 1.0f);	
		}
	};
};

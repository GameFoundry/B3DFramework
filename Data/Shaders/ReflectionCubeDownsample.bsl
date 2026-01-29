#include "$ENGINE$\PPBase.bslinc"
#include "$ENGINE$\ReflectionCubemapCommon.bslinc"

shader ReflectionCubeDownsample
{
	mixin PPBase;
	mixin ReflectionCubemapCommon;

	code
	{
		[internal]
		cbuffer Input
		{
			int gCubeFace;
			int gMipLevel;
		}	
	
		SamplerState gInputSamp;
		TextureCube gInputTex;

		float4 fsmain(VStoFS input) : SV_Target0
		{
			float2 scaledUV = input.uv0 * 2.0f - 1.0f;
			float3 dir = getDirFromCubeFace(gCubeFace, scaledUV);
			
			return gInputTex.SampleLevel(gInputSamp, dir, gMipLevel);
		}	
	};
};

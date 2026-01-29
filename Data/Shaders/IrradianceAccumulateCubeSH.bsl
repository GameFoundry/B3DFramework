#include "$ENGINE$\ReflectionCubemapCommon.bslinc"
#include "$ENGINE$\PPBase.bslinc"

shader IrradianceAccumulateCubeSH
{
	mixin PPBase;
	mixin ReflectionCubemapCommon;

	code
	{
		#define PI 3.1415926
	
		SamplerState gInputSamp;
		TextureCube gInputTex;
	
		[internal]
		cbuffer Params
		{
			uint gCubeFace;
			uint gCubeMip;
			float2 gHalfPixel;
		}			
				
		float4 fsmain(VStoFS input) : SV_Target0
		{		
			float2 offset[4];
			offset[0] = float2(gHalfPixel.x, gHalfPixel.y);
			offset[1] = float2(-gHalfPixel.x, gHalfPixel.y);
			offset[2] = float2(gHalfPixel.x, -gHalfPixel.y);
			offset[3] = float2(-gHalfPixel.x, -gHalfPixel.y);
			
			float4 sum = gInputTex.SampleLevel(gInputSamp, float3(1, 0, 0), gCubeMip);
			sum += gInputTex.SampleLevel(gInputSamp, float3(-1, 0, 0), gCubeMip);
			sum += gInputTex.SampleLevel(gInputSamp, float3(0, 1, 0), gCubeMip);
			sum += gInputTex.SampleLevel(gInputSamp, float3(0, -1, 0), gCubeMip);
			sum += gInputTex.SampleLevel(gInputSamp, float3(0, 0, 1), gCubeMip);
			sum += gInputTex.SampleLevel(gInputSamp, float3(0, 0, -1), gCubeMip);
			
			return float4(4 * PI * sum.rgb / max(sum.a, 0.0001f), 0.0f);
		}
	};
};

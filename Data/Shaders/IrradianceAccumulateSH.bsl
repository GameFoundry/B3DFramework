#include "$ENGINE$\ReflectionCubemapCommon.bslinc"
#include "$ENGINE$\PPBase.bslinc"

shader IrradianceAccumulateSH
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
			
			float4 sum = 0;
			[unroll]
			for(int i = 0; i < 4; ++i)
			{
				float2 uv = saturate(input.uv0 + offset[i]) * 2.0f - 1.0f;
				
				float3 dir = getDirFromCubeFace(gCubeFace, uv);
				dir = normalize(dir);
				
				float4 value = gInputTex.SampleLevel(gInputSamp, dir, gCubeMip);
				sum += value;
			}
			
			return sum;
		}
	};
};

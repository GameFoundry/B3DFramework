#include "$ENGINE$\PPBase.bslinc"

technique PPGaussianBlur
{
	mixin PPBase;

	code
	{
		[internal]
		cbuffer Input
		{
			float4 gSampleOffsets[(MAX_NUM_SAMPLES + 1) / 2];
			float4 gSampleWeights[(MAX_NUM_SAMPLES + 3) / 4];
			int gNumSamples;
		}		

		SamplerState gInputSamp;
		Texture2D gInputTex;
		
		float4 fsmain(VStoFS input) : SV_Target0
		{
			// Note: Consider adding a version of this shader with unrolled loop for small number of samples
			float4 output = 0;
			
			int idx = 0;
			for(; idx < (gNumSamples / 4); idx++)
			{
				{
					float2 uv = input.uv0 + gSampleOffsets[idx * 2 + 0].xy;
					output += gInputTex.Sample(gInputSamp, uv) * gSampleWeights[idx].x;
				}
				
				{
					float2 uv = input.uv0 + gSampleOffsets[idx * 2 + 0].zw;
					output += gInputTex.Sample(gInputSamp, uv) * gSampleWeights[idx].y;
				}
				
				{
					float2 uv = input.uv0 + gSampleOffsets[idx * 2 + 1].xy;
					output += gInputTex.Sample(gInputSamp, uv) * gSampleWeights[idx].z;
				}
				
				{
					float2 uv = input.uv0 + gSampleOffsets[idx * 2 + 1].zw;
					output += gInputTex.Sample(gInputSamp, uv) * gSampleWeights[idx].w;
				}
			}
			
			int extraSamples = gNumSamples - idx * 4;
			[branch]
			if(extraSamples >= 1)
			{
				float2 uv = input.uv0 + gSampleOffsets[idx * 2 + 0].xy;
				output += gInputTex.Sample(gInputSamp, uv) * gSampleWeights[idx].x;
				
				[branch]
				if(extraSamples >= 2)
				{
					float2 uv = input.uv0 + gSampleOffsets[idx * 2 + 0].zw;
					output += gInputTex.Sample(gInputSamp, uv) * gSampleWeights[idx].y;
					
					[branch]
					if(extraSamples >= 3)
					{
						float2 uv = input.uv0 + gSampleOffsets[idx * 2 + 1].xy;
						output += gInputTex.Sample(gInputSamp, uv) * gSampleWeights[idx].z;
					}
				}				
			}
			
			return output;
		}	
	};
};
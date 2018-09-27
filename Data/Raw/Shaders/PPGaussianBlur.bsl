#include "$ENGINE$\PPBase.bslinc"

shader PPGaussianBlur
{
	mixin PPBase;
	
	variations
	{
		ADDITIVE = { false, true };
	};

	code
	{
		[internal]
		cbuffer Input
		{
			float4 gSampleOffsets[(MAX_NUM_SAMPLES + 1) / 2];
			float4 gSampleWeights[MAX_NUM_SAMPLES];
			int gNumSamples;
		}		

		Texture2D gInputTex;
		
		[alias(gInputTex)]
		SamplerState gInputSamp
		{
			AddressU = BORDER;
			AddressV = BORDER;
			BorderColor = 0;
		};
		
		#if ADDITIVE
		Texture2D gAdditiveTex;
		
		[alias(gAdditiveTex)]
		SamplerState gAdditiveSamp
		{
			AddressU = BORDER;
			AddressV = BORDER;
			BorderColor = 0;
		};
		#endif
		
		float4 fsmain(VStoFS input) : SV_Target0
		{
			// Note: Consider adding a version of this shader with unrolled loop for small number of samples
			float4 output = 0;
			
			int idx = 0;
			for(; idx < (gNumSamples / 4); idx++)
			{
				{
					float2 uv = input.uv0 + gSampleOffsets[idx * 2 + 0].xy;
					output += gInputTex.SampleLevel(gInputSamp, uv, 0) * gSampleWeights[idx * 4 + 0];
				}
				
				{
					float2 uv = input.uv0 + gSampleOffsets[idx * 2 + 0].zw;
					output += gInputTex.SampleLevel(gInputSamp, uv, 0) * gSampleWeights[idx * 4 + 1];
				}
				
				{
					float2 uv = input.uv0 + gSampleOffsets[idx * 2 + 1].xy;
					output += gInputTex.SampleLevel(gInputSamp, uv, 0) * gSampleWeights[idx * 4 + 2];
				}
				
				{
					float2 uv = input.uv0 + gSampleOffsets[idx * 2 + 1].zw;
					output += gInputTex.SampleLevel(gInputSamp, uv, 0) * gSampleWeights[idx * 4 + 3];
				}
			}
			
			int extraSamples = gNumSamples - idx * 4;
			[branch]
			if(extraSamples >= 1)
			{
				float2 uv = input.uv0 + gSampleOffsets[idx * 2 + 0].xy;
				output += gInputTex.SampleLevel(gInputSamp, uv, 0) * gSampleWeights[idx * 4 + 0];
				
				[branch]
				if(extraSamples >= 2)
				{
					float2 uv = input.uv0 + gSampleOffsets[idx * 2 + 0].zw;
					output += gInputTex.SampleLevel(gInputSamp, uv, 0) * gSampleWeights[idx * 4 + 1];
					
					[branch]
					if(extraSamples >= 3)
					{
						float2 uv = input.uv0 + gSampleOffsets[idx * 2 + 1].xy;
						output += gInputTex.SampleLevel(gInputSamp, uv, 0) * gSampleWeights[idx * 4 + 2];
					}
				}				
			}
			
			#if ADDITIVE
				output += gAdditiveTex.SampleLevel(gAdditiveSamp, input.uv0, 0);
			#endif
			
			return output;
		}	
	};
};
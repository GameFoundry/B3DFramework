#include "$ENGINE$\PPBase.bslinc"

shader PPDownsample
{
	mixin PPBase;

	variations
	{
		QUALITY = { 0, 1 };
		MSAA = { true, false };
	};
	
	code
	{
		[internal]
		cbuffer Input
		{
			float2 gOffsets[4];
		}		

		#if MSAA
			Texture2DMS<float4> gInputTex;
			
			// position is expected to be at the center of 2x2 pixel tile, in pixels
			float4 bilinearFilter(float2 position)
			{
				float4 sampleSum;

				sampleSum = gInputTex.Load(trunc(position + float2(-0.5f, -0.5f)), 0);
				sampleSum += gInputTex.Load(trunc(position + float2(0.5f, -0.5f)), 0);
				sampleSum += gInputTex.Load(trunc(position + float2(-0.5f, 0.5f)), 0);
				sampleSum += gInputTex.Load(trunc(position + float2(0.5f, 0.5f)), 0);
				
				return sampleSum * 0.25f;
			}
		#else
			SamplerState gInputSamp
			{
				AddressU = CLAMP;
				AddressV = CLAMP;
				AddressW = CLAMP;
			};
			Texture2D gInputTex;
			
			// position is expected to be at the center of 2x2 pixel tile, in UV
			float4 bilinearFilter(float2 position)
			{
				return gInputTex.Sample(gInputSamp, position);
			}			
		#endif
		
		float4 fsmain(VStoFS input) : SV_Target0
		{
			// input.uv0 is in the center of 2x2 block of pixels. If MSAA is enabled the value is
			// in pixels, otherwise normal UV range.
			
			#if QUALITY == 0 
				// Single bilinearly filtered sample (2x2 block average)
				return bilinearFilter(input.uv0);
			#else // QUALITY == 1
				// Four bilinearly filtered samples (4x4 block average)
				float4 samples[4];

				[unroll]
				for(uint i = 0; i < 4; i++)
					samples[i] = bilinearFilter(input.uv0 + gOffsets[i]);

				return (samples[0] + samples[1] + samples[2] + samples[3]) * 0.25f;
				
			#endif // QUALITY
		}	
	};
};

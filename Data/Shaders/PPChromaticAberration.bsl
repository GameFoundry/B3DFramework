#include "$ENGINE$\PPBase.bslinc"

shader PPChromaticAberration
{
	mixin PPBase;

	variations
	{
		SIMPLE = { false, true };
	};
	
	code
	{
		[internal]
		cbuffer Params
		{
			float2 gInputSize;
			float gShiftAmount;
		}	
	
		SamplerState gInputSamp
		{
			Filter = MIN_MAG_MIP_POINT;
		};
		Texture2D gInputTex;
		
		SamplerState gFringeSamp;
		Texture2D gFringeTex;
		
		float4 fsmain(VStoFS input) : SV_Target0
		{
            float2 ndc = 2.0f * input.uv0 - 1.0f;
            float2 end = input.uv0 - ndc * dot(ndc, ndc) * gShiftAmount;
		
		    #if SIMPLE
            {
                float2 delta = (end - input.uv0) / 3.0f;

                float4 filterA = float4(gFringeTex.Sample(gFringeSamp, float2(0.5f / 3.0f, 0.0f)).rgb, 1.0f);
                float4 filterB = float4(gFringeTex.Sample(gFringeSamp, float2(1.5f / 3.0f, 0.0f)).rgb, 1.0f);
                float4 filterC = float4(gFringeTex.Sample(gFringeSamp, float2(2.5f / 3.0f, 0.0f)).rgb, 1.0f);

                float4 texelA = gInputTex.Sample(gInputSamp, input.uv0);
                float4 texelB = gInputTex.Sample(gInputSamp, input.uv0 + delta);
                float4 texelC = gInputTex.Sample(gInputSamp, input.uv0 + delta * 2.0f);

                float4 sum = texelA * filterA + texelB * filterB + texelC * filterC;
                float4 filterSum = filterA + filterB + filterC;
                return sum / filterSum;
            }
            #else
            {
                float2 diff = end - input.uv0;
                int numSamples = clamp((int)length(gInputSize * diff / 2.0f), 3, MAX_SAMPLES);
				
                float2 delta = diff / numSamples;
                float2 pos = input.uv0;
                float4 sum = 0.0f;
				float4 filterSum = 0.0f;

                for (int i = 0; i < numSamples; i++)
                {
                    float t = (i + 0.5f) / numSamples;
                    float4 s = gInputTex.Sample(gInputSamp, pos);
                    float4 filter = float4(gFringeTex.Sample(gFringeSamp, float2(t, 0.0f)).rgb, 1.0f);

                    sum += s * filter;
                    filterSum += filter;
                    pos += delta;
                }

                return sum / filterSum;
            }
            #endif
		}
	};
};

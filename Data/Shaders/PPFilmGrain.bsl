#include "$ENGINE$\PPBase.bslinc"

shader PPFilmGrain
{
	mixin PPBase;

	code
	{
		[internal]
		cbuffer Params
		{
			float gIntensity;
			float gTime;
		}	
	
		SamplerState gInputSamp
		{
			Filter = MIN_MAG_MIP_POINT;
		};
		Texture2D gInputTex;
		
		float4 fsmain(VStoFS input) : SV_Target0
		{
			float2 uv = input.uv0;
            float x = (uv.x + 4.0f) * (uv.y + 4.0f) * gTime;
			float4 grain = (fmod((fmod(x, 13.0f) + 1.0f) * (fmod(x, 123.0f) + 1.0f), 0.01f) - 0.005f) * gIntensity;
			
			float4 color = gInputTex.Sample(gInputSamp, uv);
			return color + grain;
		}
	};
};

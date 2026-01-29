#include "$ENGINE$\PPBase.bslinc"
#include "$ENGINE$\PPGaussianBlurCommon.bslinc"

shader PPGaussianBlur
{
	mixin PPBase;
	mixin PPGaussianBlurCommon;
	
	variations
	{
		ADDITIVE = { false, true };
	};

	code
	{
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
			float4 output = gaussianBlur(gInputTex, gInputSamp, input.uv0);
		
			#if ADDITIVE
				output += gAdditiveTex.SampleLevel(gAdditiveSamp, input.uv0, 0);
			#endif
			
			return output;
		}	
	};
};

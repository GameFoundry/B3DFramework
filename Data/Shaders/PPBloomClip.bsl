#include "$ENGINE$\ColorSpace.bslinc"

shader PPBloomClip
{
	mixin ColorSpace;

	variations
	{
		AUTO_EXPOSURE = { true, false };
	};
	
	depth
	{
		read = false;
		write = false;
	};

	code
	{
		struct VStoFS
		{
			float4 position : SV_POSITION;
			float2 uv0 : TEXCOORD0;
			float exposureScale : TEXCOORD1;
		};

		struct VertexInput
		{
			float2 screenPos : POSITION;
			float2 uv0 : TEXCOORD0;
		};
		
		Texture2D gEyeAdaptationTex;
		
		VStoFS vsmain(VertexInput input)
		{
			VStoFS output;
		
			output.position = float4(input.screenPos, 0, 1);
			output.uv0 = input.uv0;
			output.exposureScale = gEyeAdaptationTex.Load(int3(0, 0, 0)).r;

			return output;
		}			

		SamplerState gInputSamp;
		Texture2D gInputTex;
				
		cbuffer Input
		{
			float gThreshold;
			float gManualExposureScale;
		}				
		
		float4 fsmain(VStoFS input) : SV_Target0
		{
			float3 sceneColor = gInputTex.Sample(gInputSamp, input.uv0).rgb;		
			float luminance = LuminanceRGB(sceneColor);
			
			#if AUTO_EXPOSURE
				luminance = luminance * input.exposureScale;
			#else
				luminance = luminance * gManualExposureScale;
			#endif
			
			float clippedLuminance = luminance - gThreshold;
			float bloomMask = saturate(clippedLuminance / 2.0f);
			
			return float4(sceneColor * bloomMask, 0.0f);
		}	
	};
};

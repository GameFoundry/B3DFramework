#include "$ENGINE$\PPTonemapCommon.bslinc"

technique PPTonemapping
{
	mixin PPTonemapCommon;

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

		#if MSAA
			Texture2DMS<float4> gInputTex;
		#else
			SamplerState gInputSamp;
			Texture2D gInputTex;
		#endif
		
		SamplerState gColorLUTSamp;
		Texture3D gColorLUT;
		
		cbuffer Input
		{
			float gRawGamma;
			float gManualExposureScale;
			uint gNumSamples;
		}				
		
		float3 ColorLookupTable(float3 linearColor)
		{
			float3 logColor = LinearToLogColor(linearColor);
			float3 UVW = logColor * ((LUT_SIZE - 1) / (float)LUT_SIZE) + (0.5f / LUT_SIZE);
			
			float3 gradedColor = gColorLUT.Sample(gColorLUTSamp, UVW).rgb;
			return gradedColor;
		}
		
		float3 tonemapSample(float3 samp, float exposureScale)
		{
			#if AUTO_EXPOSURE
				samp = samp * exposureScale;
			#else
				samp = samp * gManualExposureScale;
			#endif
			
			#if GAMMA_ONLY
				return pow(samp, gRawGamma);				
			#else
				return ColorLookupTable(samp);
			#endif
		}
		
		float4 fsmain(VStoFS input) : SV_Target0
		{
			float4 sceneColor = 0;
			#if MSAA
				for(uint i = 0; i < gNumSamples; ++i)
					sceneColor.rgb += tonemapSample(gInputTex.Load(trunc(input.uv0), i).rgb, input.exposureScale);
			
				sceneColor.rgb /= gNumSamples;
			#else
				sceneColor.rgb = tonemapSample(gInputTex.Sample(gInputSamp, input.uv0).rgb, input.exposureScale);
			#endif
						
			// Output luma in gamma-space, for FXAA
			// Note: This can be avoided if FXAA is not used
			sceneColor.a = dot(sceneColor.rgb, float3(0.299, 0.587, 0.114));

			return sceneColor;
		}	
	};
};
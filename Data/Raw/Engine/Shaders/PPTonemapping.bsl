#include "$ENGINE$\PPTonemapCommon.bslinc"

Parameters =
{
	Sampler2D 	gInputSamp : alias("gInputTex");
	Texture2D 	gInputTex;
	Sampler3D 	gColorLUTSamp : alias("gColorLUT");
	Texture3D 	gColorLUT;	
	Texture2D	gEyeAdaptationTex;
};

Technique =
{
	Language = "HLSL11";
	
	Pass =
	{
		DepthWrite = false;
		DepthRead = false;
	
		Common = 
		{
			struct VStoFS
			{
				float4 position : SV_POSITION;
				float2 uv0 : TEXCOORD0;
				float exposureScale : TEXCOORD1;
			};
		};

		Vertex =
		{
			struct VertexInput
			{
				float2 screenPos : POSITION;
				float2 uv0 : TEXCOORD0;
			};
			
			Texture2D gEyeAdaptationTex;
			
			VStoFS main(VertexInput input)
			{
				VStoFS output;
			
				output.position = float4(input.screenPos, 0, 1);
				output.uv0 = input.uv0;
				output.exposureScale = gEyeAdaptationTex.Load(int3(0, 0, 0)).r;

				return output;
			}			
		};	
	
		Fragment =
		{
			SamplerState gInputSamp;
			Texture2D gInputTex;
			
			SamplerState gColorLUTSamp;
			Texture3D gColorLUT;
			
			cbuffer Input
			{
				float gRawGamma;
				float gManualExposureScale;
			}				
			
			float3 ColorLookupTable(float3 linearColor)
			{
				float3 logColor = LinearToLogColor(linearColor);
				float3 UVW = logColor * ((LUT_SIZE - 1) / (float)LUT_SIZE) + (0.5f / LUT_SIZE);
				
				float3 gradedColor = gColorLUT.Sample(gColorLUTSamp, UVW).rgb;
				return gradedColor;
			}
						
			float4 main(VStoFS input) : SV_Target0
			{
				float4 sceneColor = gInputTex.Sample(gInputSamp, input.uv0);
				
				#if AUTO_EXPOSURE
					sceneColor.rgb = sceneColor.rgb * input.exposureScale;
				#else
					sceneColor.rgb = sceneColor.rgb * gManualExposureScale;
				#endif
				
				#if GAMMA_ONLY
					sceneColor.rgb = pow(sceneColor.rgb, gRawGamma);				
				#else
					sceneColor.rgb = ColorLookupTable(sceneColor.rgb);
				#endif

				return sceneColor;
			}	
		};
	};
};

Technique =
{
	Language = "GLSL";
	
	Pass =
	{
		Fragment =
		{
			// TODO
		};
	};
};
#include "$ENGINE$\PPTonemapCommon.bslinc"

Parameters =
{
	Sampler2D 	gInputSamp : alias("gInputTex");
	Texture2D 	gInputTex;
	Sampler3D 	gColorLUTSamp : alias("gColorLUT");
	Texture3D 	gColorLUT;	
	Texture2D	gEyeAdaptationTex;
};

Blocks =
{
	Block Input;
};

Technique : inherits("PPTonemapCommon") =
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

Technique : inherits("PPTonemapCommon") =
{
	Language = "GLSL";
	
	Pass =
	{
		DepthWrite = false;
		DepthRead = false;
	
		Vertex =
		{
			layout(location = 0) in vec2 bs_position;
			layout(location = 1) in vec2 bs_texcoord0;
			
			out VStoFS
			{
				layout(location = 0) vec2 uv0;
				layout(location = 1) float exposureScale;
			} VSOutput;
			
			layout(binding = 0) uniform sampler2D gEyeAdaptationTex;
			
			out gl_PerVertex
			{
				vec4 gl_Position;
			};			
			
			void main()
			{
				gl_Position = vec4(bs_position, 0, 1);
				VSOutput.uv0 = bs_texcoord0;
				VSOutput.exposureScale = texelFetch(gEyeAdaptationTex, ivec2(0, 0), 0).r;
			}			
		};	
	
		Fragment =
		{
			in VStoFS
			{
				layout(location = 0) vec2 uv0;
				layout(location = 1) float exposureScale;
			} FSInput;
		
			layout(location = 0) out vec4 fragColor;
		
			layout(binding = 1) uniform sampler2D gInputTex;
			layout(binding = 2) uniform sampler3D gColorLUT;
			
			layout(binding = 3) uniform Input
			{
				float gRawGamma;
				float gManualExposureScale;
			};

			void ColorLookupTable(vec3 linearColor, out vec3 result)
			{
				vec3 logColor;
				LinearToLogColor(linearColor, logColor);
				
				vec3 UVW = logColor * ((LUT_SIZE - 1) / float(LUT_SIZE)) + (0.5f / LUT_SIZE);
				
				vec3 gradedColor = texture(gColorLUT, UVW).rgb;
				result = gradedColor;
			}
						
			void main()
			{
				vec4 sceneColor = texture2D(gInputTex, FSInput.uv0);
				
				#if AUTO_EXPOSURE
					sceneColor.rgb = sceneColor.rgb * FSInput.exposureScale;
				#else
					sceneColor.rgb = sceneColor.rgb * gManualExposureScale;
				#endif
				
				#if GAMMA_ONLY
					sceneColor.rgb = pow(sceneColor.rgb, vec3(gRawGamma));				
				#else
					vec3 lookupColor;
					ColorLookupTable(sceneColor.rgb, lookupColor);
					
					sceneColor.rgb = lookupColor;
				#endif

				fragColor = sceneColor;
			}	
		};
	};
};
#include "$ENGINE$\PPTonemapCommon.bslinc"

shader PPTonemapping
{
	mixin PPTonemapCommon;

	variations
	{
		VOLUME_LUT = { true, false };
		GAMMA_ONLY = { true, false };
		AUTO_EXPOSURE = { true, false };
		MSAA = { true, false };
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

		#if MSAA
			Texture2DMS<float4> gInputTex;
		#else
			SamplerState gInputSamp;
			Texture2D gInputTex;
		#endif
		
		SamplerState gColorLUTSamp;
		
		#if VOLUME_LUT
		Texture3D gColorLUT;
		#else
		Texture2D gColorLUT;
		#endif
		
		Texture2D gBloomTex;
		SamplerState gBloomSamp;
		
		cbuffer Input
		{
			float gRawGamma;
			float gManualExposureScale;
			float2 gTexSize;
			float4 gBloomTint;
			uint gNumSamples;
		}				
		
		float3 ColorLookupTable(float3 linearColor)
		{
			float3 logColor = LinearToLogColor(linearColor);
			float3 UVW = logColor * ((LUT_SIZE - 1) / (float)LUT_SIZE) + (0.5f / LUT_SIZE);
			
			#if VOLUME_LUT
			
			float3 gradedColor = gColorLUT.Sample(gColorLUTSamp, UVW).rgb;
			
			#else
			
			float slice = floor(UVW.z * LUT_SIZE - 0.5f);
			float sliceFrac = UVW.z * LUT_SIZE - 0.5f - slice;

			float U = (UVW.x + slice) / LUT_SIZE;
			float V = UVW.y;

			// Blend between two slices (emulating a 3D texture sample)
			float3 v0 = gColorLUT.Sample(gColorLUTSamp, float2(U, V)).rgb;
			float3 v1 = gColorLUT.Sample(gColorLUTSamp, float2(U + 1.0f / LUT_SIZE, V)).rgb;

			float3 gradedColor = lerp(v0, v1, sliceFrac);
			
			#endif
			
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
			float3 bloomColor = gBloomTex.Sample(gBloomSamp, input.uv0) * gBloomTint;
			#if MSAA
				// Note: Ideally I'd want to use the MSAA coverage texture here, so I can only average samples for pixels
				// that really need it. But because forward rendering doesn't write to MSAA coverage I can't do it as I
				// don't have up-to-date coverage information. It might be good to find a way around this.
				int2 pixelXY = trunc(input.uv0 * gTexSize);
				for(uint i = 0; i < gNumSamples; ++i)
					sceneColor.rgb += tonemapSample(gInputTex.Load(pixelXY, i).rgb + bloomColor, input.exposureScale);
			
				sceneColor.rgb /= gNumSamples;
			#else
				sceneColor.rgb = tonemapSample(gInputTex.Sample(gInputSamp, input.uv0).rgb + bloomColor, input.exposureScale);
			#endif
					
			// Output luma in gamma-space, for FXAA
			// Note: This can be avoided if FXAA is not used
			sceneColor.a = dot(sceneColor.rgb, float3(0.299, 0.587, 0.114));

			return sceneColor;
		}	
	};
};

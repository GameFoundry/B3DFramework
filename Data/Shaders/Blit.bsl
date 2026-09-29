#include "$ENGINE$\ColorSpace.bslinc"

shader Blit
{
	mixin ColorSpace;

	variations
	{
		MODE = 
			{ 
				0, // Color (point filtering) 
				1, // Color (bilinear filtering)
				2, // Depth
			};
		MSAA_COUNT = { 1, 2, 4, 8 };
        BLEND =
			{
				0, // Overwrite
				1, // Blend, straight alpha source
				2, // Blend, premultiplied alpha source
			};
        WRITE_ALPHA = { false, true };
        SRGB_ENCODE = { false, true };
        OUTPUT_32BIT = { false, true };
	};

	#if OUTPUT_32BIT
	output
	{
		target
		{
			index = 0;
			format = RGBA32F;
		};
	};
	#endif

	depth
	{	
		#if MODE != 2
		read = false;
		write = false;	
		#else
		// Cannot use read = false because that disables gl_FragDepth writes on OpenGL
		compare = always;	
		#endif
	};

	#if BLEND
	blend
	{
		target
        {
			enabled = true;

			#if BLEND == 1
				color = { srcA, srcIA, add };
			#else
				color = { one, srcIA, add };
				alpha = { one, srcIA, add };
			#endif

			#if WRITE_ALPHA
				writemask = RGBA;
			#else
				writemask = RGB;
			#endif
		};
	};
	#endif

	code
	{
		struct VStoFS
		{
			noperspective float4 position : SV_POSITION;
			noperspective float2 uv0 : TEXCOORD0;
		};

		struct VertexInput
		{
			float2 screenPos : POSITION;
			float2 uv0 : TEXCOORD0;
		};
		
		VStoFS vsmain(VertexInput input)
		{
			VStoFS output;
		
			output.position = float4(input.screenPos, 0, 1);
			output.uv0 = input.uv0;

			return output;
		}			

		#if MSAA_COUNT > 1
		
		#if MODE != 2
		Texture2DMS<float4> gSource;
		
		float4 fsmain(VStoFS input) : SV_Target0
		#else // MODE
		// Depth
		Texture2DMS<float> gSource;
		
		float fsmain(VStoFS input, out float depth : SV_Depth) : SV_Target0
		#endif // MODE
		{
			int2 iUV = trunc(input.uv0);
		
			#if MODE != 2
				float4 sum = float4(0, 0, 0, 0);
				
				[unroll]
				for(int i = 0; i < MSAA_COUNT; i++)
					sum += gSource.Load(iUV, i);
					
				return sum / MSAA_COUNT;
			#else // MODE 
				// Depth
				float minVal = gSource.Load(iUV, 0);
				
				[unroll]
				for(int i = 1; i < MSAA_COUNT; i++)
					minVal = min(minVal, gSource.Load(iUV, i));
					
				depth = minVal;
				return 0.0f;
			#endif // MODE
		}
		
		#else // MSAA_COUNT
		
		Texture2D<float4> gSource;

		#if MODE == 1
			SamplerState gSampler;
		#endif

		float4 fsmain(VStoFS input) : SV_Target0
		{
			#if MODE == 1
				float4 color = gSource.Sample(gSampler, input.uv0);
			#else // MODE
				int2 iUV = trunc(input.uv0);
				float4 color = gSource.Load(int3(iUV.xy, 0));
			#endif // MODE

			#if SRGB_ENCODE
				// Source is hardware-sRGB (decoded to linear on sample); re-encode to sRGB so it can be written to a non-sRGB target.
				#if BLEND == 2
					// Encoding applies to the straight color, so undo the premultiplication around it
					if(color.a > 0.0f)
						color.rgb = LinearToGammasRGB(color.rgb / color.a) * color.a;
				#else
					color.rgb = LinearToGammasRGB(color.rgb);
				#endif
			#endif

			return color;
		}
		
		#endif // MSAA_COUNT
	};
};

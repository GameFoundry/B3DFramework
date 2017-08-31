technique Blit
{
	depth
	{	
		#ifndef DEPTH
		read = false;
		write = false;
		#else
		// Cannot use read = false because that disables gl_FragDepth writes on OpenGL
		compare = always;		
		#endif
	};

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
		
		#if COLOR
		Texture2DMS<float4> gSource;
		
		float4 fsmain(VStoFS input) : SV_Target0
		#else // Assuming depth
		Texture2DMS<float> gSource;
		
		float fsmain(VStoFS input, out float depth : SV_Depth) : SV_Target0
		#endif
		{
			int2 iUV = trunc(input.uv0);
		
			#if COLOR
				float4 sum = float4(0, 0, 0, 0);
				
				[unroll]
				for(int i = 0; i < MSAA_COUNT; i++)
					sum += gSource.Load(iUV, i);
					
				return sum / MSAA_COUNT;
			#else // Assuming depth
				float minVal = gSource.Load(iUV, 0);
				
				[unroll]
				for(int i = 1; i < MSAA_COUNT; i++)
					minVal = min(minVal, gSource.Load(iUV, i));
					
				depth = minVal;
				return 0.0f;
			#endif
		}
		
		#else
		
		Texture2D<float4> gSource;
	
		float4 fsmain(VStoFS input) : SV_Target0
		{
			int2 iUV = trunc(input.uv0);
			return gSource.Load(int3(iUV.xy, 0));
		}
		
		#endif
	};
};
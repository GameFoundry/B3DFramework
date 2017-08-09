technique TetrahedraRender
{
	raster
	{
		cull = cw;
	};
	
	depth
	{
		compare = lte;
	};

	code
	{
		struct VertexInput
		{
			float3 position : POSITION;
			uint index : TEXCOORD0;			
		};		
	
		struct VStoFS
		{
			float4 position : SV_Position;
			uint index : TEXCOORD0;
		};
		
		cbuffer Params
		{
			float4x4 gMatViewProj;
			float4 gNDCToUV;
			float2 gNDCToDeviceZ;
		};
		
		VStoFS vsmain(VertexInput input)
		{
			VStoFS output;
		
			output.position = mul(gMatViewProj, float4(input.position, 1.0f));
			output.index = input.index;
			
			return output;
		}
		
		#ifndef MSAA
			#define MSAA 0
		#endif

		#if MSAA
		Texture2DMS<float> gDepthBufferTex;
		#else
		Texture2D gDepthBufferTex;
		SamplerState gDepthBufferSamp;
		#endif		
		
		uint fsmain(VStoFS input
		#if MSAA
			,uint sampleIdx : SV_SampleIndex
		#endif
		) : SV_Target0
		{
			float2 uv = input.position.xy * gNDCToUV.xy + gNDCToUV.zw;
			
			float sceneDepth;
			#if MSAA
				sceneDepth = gDepthBufferTex.Load(trunc(uv), sampleIdx);
			#else
				sceneDepth = gDepthBufferTex.Sample(gDepthBufferSamp, uv);
			#endif
			
			float currentDepth = input.position.z * gNDCToDeviceZ.x + gNDCToDeviceZ.y;
			if(currentDepth < sceneDepth)
				discard;
				
			return input.index;
		}
	};
};
#include "$ENGINE$\PerCameraData.bslinc"

shader TetrahedraRender
{
	mixin PerCameraData;

	variations
	{
		MSAA = { true, false };
		MSAA_RESOLVE_0TH = { true, false };
	};
	
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
			float4 clipPos : TEXCOORD0;
			uint index : TEXCOORD1;
		};
		
		VStoFS vsmain(VertexInput input)
		{
			VStoFS output;
		
			output.position = mul(gMatViewProj, float4(input.position, 1.0f));
			output.clipPos = output.position;
			output.index = input.index;
			
			return output;
		}
		
		#if MSAA
		Texture2DMS<float> gDepthBufferTex;
		#else
		Texture2D gDepthBufferTex;
		SamplerState gDepthBufferSamp;
		#endif
		
		cbuffer Params
		{
			int2 gDepthTexSize;
		};
		
		uint fsmain(VStoFS input
		#if MSAA && !MSAA_RESOLVE_0TH
			,uint sampleIdx : SV_SampleIndex
		#endif
		) : SV_Target0
		{
			float2 ndcPos = input.clipPos.xy / input.clipPos.w;
			float2 uv = NDCToUV(ndcPos);
		
			float sceneDepth;
			#if MSAA
				int2 pixelPos = uv * gDepthTexSize;
			
				#if MSAA_RESOLVE_0TH
					sceneDepth = gDepthBufferTex.Load(pixelPos, 0);
				#else
					sceneDepth = gDepthBufferTex.Load(pixelPos, sampleIdx);
				#endif
			#else
				sceneDepth = gDepthBufferTex.Sample(gDepthBufferSamp, uv);
			#endif
			
			float currentDepth = input.position.z;
			if(currentDepth < sceneDepth)
				discard;
				
			return input.index;
		}
	};
};

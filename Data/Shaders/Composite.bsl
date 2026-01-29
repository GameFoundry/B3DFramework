shader Composite
{
	depth
	{	
		read = false;
		write = false;	
	};
	
	blend
	{
		target	
		{
			enabled = true;
			color = { one, one, add };
		};
	};

	code
	{
		[internal]
		cbuffer Input
		{
			[color]
			float4 gTint;
		}		

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

		Texture2D<float4> gSource;
		
		[alias(gSource)]
		SamplerState gSampler;
	
		float4 fsmain(VStoFS input) : SV_Target0
		{
			return gSource.Sample(gSampler, input.uv0) * gTint;
		}
	};
};

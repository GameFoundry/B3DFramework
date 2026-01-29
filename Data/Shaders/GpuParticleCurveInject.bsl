shader GpuParticleCurveInject
{
	depth
	{
		write = false;
		read = false;
	};

	code
	{	
		struct VertexInput
		{
			float4 color : TEXCOORD0;
			float2 dataUV : TEXCOORD1;
			float2 uv0 : TEXCOORD2;
		};	
	
		struct VStoFS
		{
			float4 position : SV_POSITION;
			float4 color : TEXCOORD0;
		};

		cbuffer Input
		{
			float4 gUVToNDC;
		};
		
		VStoFS vsmain(VertexInput input)
		{
			float2 uv = input.uv0 + input.dataUV;
		
			VStoFS output;
			output.position = float4(uv * gUVToNDC.xy + gUVToNDC.zw, 0.0f, 1.0f);
			output.color = input.color;
		
			return output;
		}			
	
		float4 fsmain(VStoFS input) : SV_Target0
		{
			return input.color;
		}	
	};
};

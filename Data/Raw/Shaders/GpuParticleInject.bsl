shader GpuParticleInject
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
			float4 posAndTime : TEXCOORD0;
			float4 velocity : TEXCOORD1;
			float2 dataUV : TEXCOORD2;
			float2 uv0 : TEXCOORD3;
		};	
	
		struct VStoFS
		{
			float4 position : SV_POSITION;
			float4 posAndTime : TEXCOORD0;
			float4 velocity : TEXCOORD1;
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
			output.posAndTime = input.posAndTime;
			output.velocity = input.velocity;
		
			return output;
		}			
	
		void fsmain(VStoFS input, 
			out float4 outPosAndTime : SV_Target0, 
			out float4 outVel : SV_Target1)
		{
			outPosAndTime = input.posAndTime;
			outVel = input.velocity;	
		}	
	};
};
shader TextureArrayToMSAATexture
{
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

		Texture2DArray<float4> gInput;

		float4 fsmain(VStoFS input, uint sampleIndex : SV_SampleIndex) : SV_Target0
		{
			int2 pixelPos = trunc(input.uv0);
			return gInput.Load(uint4(pixelPos, sampleIndex, 0));
		}
	};
};

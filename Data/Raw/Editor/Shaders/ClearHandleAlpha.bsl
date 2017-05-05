Technique =
{
	Pass =
	{
		Target = 
		{
			WriteMask = A;
		};
		
		DepthRead = false;
		DepthWrite = false;
		
		Stencil = true;
		StencilOpFront = { KEEP, KEEP, KEEP, LTE };
		StencilRef = 1;
		
		Vertex =
		{
			float4 main(float2 screenPos : POSITION) : SV_POSITION
			{
				return float4(screenPos, 0, 1);
			}
		};	
		
		Fragment = 
		{
			float4 main(in float4 inPos : SV_Position) : SV_Target
			{
				return float4(0, 0, 0, 1);
			}	
		};	
	};
};
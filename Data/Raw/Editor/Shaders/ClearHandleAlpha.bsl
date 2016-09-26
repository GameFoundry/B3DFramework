Technique =
{
	Language = "HLSL11";
	
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

Technique =
{
	Language = "GLSL";
	
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
			in vec3 bs_position;

			out gl_PerVertex
			{
				vec4 gl_Position;
			};
			
			void main()
			{
				gl_Position = vec4(bs_position.xy, 0, 1);
			}
		};
		
		Fragment =
		{
			out vec4 fragColor;

			void main()
			{
				fragColor = vec4(0, 0, 0, 1);
			}
		};
	};
};
Parameters =
{
	mat4x4		matWorldViewProj;
	float4		selColor;
};

Technique =
{
	Language = "HLSL11";
	
	Pass =
	{
		Fill = WIRE;
		DepthBias = 0.00001f;
		
		Target =
		{
			Blend = true;
			Color = { SRCA, SRCIA, ADD };
		};

		Vertex =
		{
			float4x4 matWorldViewProj;

			void main(
			in float3 inPos : POSITION,
			out float4 oPosition : SV_Position)
			{
				oPosition = mul(matWorldViewProj, float4(inPos.xyz, 1));
			}
		};
		
		Fragment =
		{
			float4 selColor;

			float4 main(in float4 inPos : SV_Position) : SV_Target
			{
				return selColor;
			}
		};
	};
};

Technique =
{
	Language = "HLSL9";
	
	Pass =
	{
		Fill = WIRE;
		DepthBias = 0.00001f;
		
		Target =
		{
			Blend = true;
			Color = { SRCA, SRCIA, ADD };
		};

		Vertex =
		{
			float4x4 matWorldViewProj;

			void main(
			in float3 inPos : POSITION,	
			out float4 oPosition : POSITION)
			{
				oPosition = mul(matWorldViewProj, float4(inPos.xyz, 1));
			}
		};
		
		Fragment =
		{
			float4 selColor;

			float4 main() : COLOR0
			{
				return selColor;
			}
		};
	};
};

Technique =
{
	Language = "GLSL";
	
	Pass =
	{
		Fill = WIRE;
		DepthBias = 0.00001f;
		
		Target =
		{
			Blend = true;
			Color = { SRCA, SRCIA, ADD };
		};

		Vertex =
		{
			uniform mat4 matWorldViewProj;

			in vec3 bs_position;

			out gl_PerVertex
			{
				vec4 gl_Position;
			};
			
			void main()
			{
				gl_Position = matWorldViewProj * vec4(bs_position.xyz, 1);
			}
		};
		
		Fragment =
		{
			uniform vec4 selColor;
			out vec4 fragColor;

			void main()
			{
				fragColor = selColor;
			}
		};
	};
};


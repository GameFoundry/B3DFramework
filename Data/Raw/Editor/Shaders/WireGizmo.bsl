Parameters =
{
	mat4x4		matViewProj;
};

Technique =
{
	Language = "HLSL11";
	
	Pass =
	{
		Fill = WIRE;
	
		Vertex =
		{
			float4x4 matViewProj;

			void main(
				in float3 inPos : POSITION,
				in float4 color : COLOR0,
				out float4 oPosition : SV_Position,
				out float4 oColor : COLOR0)
			{
				oPosition = mul(matViewProj, float4(inPos.xyz, 1));
				oColor = color;
			}
		};
		
		Fragment =
		{
			float4 main(in float4 inPos : SV_Position, in float4 color : COLOR0) : SV_Target
			{
				return color;
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
	
		Vertex =
		{
			float4x4 matViewProj;

			void main(
				in float3 inPos : POSITION,	
				in float4 inColor : COLOR0,	
				out float4 oPosition : POSITION,
				out float4 oColor : COLOR0)
			{
				oPosition = mul(matViewProj, float4(inPos.xyz, 1));
				oColor = inColor;
			}
		};
		
		Fragment =
		{
			float4 main(float4 color : COLOR0) : COLOR0
			{
				return color;
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
	
		Vertex =
		{
			uniform mat4 matViewProj;

			in vec3 bs_position;
			in vec4 bs_color0;
			out vec4 color0;

			out gl_PerVertex
			{
				vec4 gl_Position;
			};
			
			void main()
			{
				gl_Position = matViewProj * vec4(bs_position.xyz, 1);
				color0 = bs_color0;
			}
		};
		
		Fragment =
		{
			in vec4 color0;
			out vec4 fragColor;

			void main()
			{
				fragColor = color0;
			}
		};
	};
};

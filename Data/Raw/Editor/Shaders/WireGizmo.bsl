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
	Language = "GLSL";
	
	Pass =
	{
		Fill = WIRE;
	
		Vertex =
		{
			layout(location = 0) in vec3 bs_position;
			layout(location = 1) in vec4 bs_color0;
			layout(location = 0) out vec4 color0;

			out gl_PerVertex
			{
				vec4 gl_Position;
			};		
		
			layout(binding = 0) uniform VertUBO
			{
				mat4 matViewProj;
			};
			
			void main()
			{
				gl_Position = matViewProj * vec4(bs_position.xyz, 1);
				color0 = bs_color0;
			}
		};
		
		Fragment =
		{
			layout(location = 0) in vec4 color0;
			layout(location = 0) out vec4 fragColor;

			void main()
			{
				fragColor = color0;
			}
		};
	};
};

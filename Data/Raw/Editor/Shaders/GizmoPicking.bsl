Parameters =
{
	mat4x4		gMatViewProj;
	float		gAlphaCutoff;
};

Blocks =
{
	Block Uniforms : auto("GizmoUniforms");
};

Technique =
{
	Language = "HLSL11";
	
	Pass =
	{
		Scissor = true;

		Vertex =
		{
			cbuffer Uniforms
			{
				float4x4 	gMatViewProj;
				float		gAlphaCutoff;
			}

			void main(
				in float3 inPos : POSITION,
				in float4 inColor : COLOR0,
				out float4 oPosition : SV_Position,
				out float4 oColor : COLOR0)
			{
				oPosition = mul(gMatViewProj, float4(inPos.xyz, 1));
				oColor = inColor;
			}
		};
		
		Fragment =
		{
			float4 main(in float4 inPos : SV_Position, in float4 inColor : COLOR0) : SV_Target
			{
				return inColor;
			}
		};
	};
};

Technique =
{
	Language = "GLSL";
	
	Pass =
	{
		Scissor = true;

		Vertex =
		{
			layout(location = 0) in vec3 bs_position;
			layout(location = 1) in vec4 bs_color0;
			
			layout(location = 0) out vec4 color0;

			out gl_PerVertex
			{
				vec4 gl_Position;
			};
			
			layout(binding = 0, std140) uniform Uniforms
			{
				mat4 	gMatViewProj;
				float	gAlphaCutoff;
			};
			
			void main()
			{
				gl_Position = gMatViewProj * vec4(bs_position.xyz, 1);
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
Parameters =
{
	mat4x4		gMatViewProj;
	float4		gViewDir;
};

Blocks =
{
	Block Uniforms : auto("GizmoUniforms");
};

Technique =
{
	Pass =
	{
		Fill = WIRE;
	
		Vertex =
		{
			cbuffer Uniforms
			{
				float4x4 	gMatViewProj;
				float4		gViewDir;
			}
			
			void main(
				in float3 inPos : POSITION,
				in float4 color : COLOR0,
				out float4 oPosition : SV_Position,
				out float4 oColor : COLOR0)
			{
				oPosition = mul(gMatViewProj, float4(inPos.xyz, 1));
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
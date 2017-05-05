Parameters =
{
	mat4x4		gMatViewProj;
	float4		gViewDir;
	
	Sampler2D 	gMainTexSamp : alias("gMainTexture");
	Texture2D 	gMainTexture;
};

Blocks =
{
	Block Uniforms : auto("GizmoUniforms");
};

Technique =
{
	Pass =
	{
		Target = 
		{
			Blend = true;
			Color = { SRCA, SRCIA, ADD };
		};
		
		DepthWrite = false;
		
		Vertex =
		{
			cbuffer Uniforms
			{
				float4x4 	gMatViewProj;
				float4		gViewDir;
			}

			void main(
				in float3 inPos : POSITION,
				in float2 uv : TEXCOORD0,
				in float4 color : COLOR0,
				out float4 oPosition : SV_Position,
				out float2 oUv : TEXCOORD0,
				out float4 oColor : COLOR0)
			{
				oPosition = mul(gMatViewProj, float4(inPos.xyz, 1));
				oUv = uv;
				oColor = color;
			}		
		};
		
		Fragment =
		{
			SamplerState gMainTexSamp : register(s0);
			Texture2D gMainTexture : register(t0);

			float4 main(
				in float4 inPos : SV_Position, 
				float2 uv : TEXCOORD0, 
				float4 color : COLOR0) : SV_Target
			{
				return float4(color.rgb, gMainTexture.Sample(gMainTexSamp, uv).r * color.a);
			}		
		};
	};
};
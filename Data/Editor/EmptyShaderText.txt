Parameters =
{
	mat4x4		matWorldViewProj : auto("WVP");

	Sampler2D 	samp : alias("tex");
	Texture2D 	tex;
};

Blocks =
{
	Block PerObject : auto("PerObject");
};

Technique =
{
	Language = "HLSL11";
	
	Pass =
	{

		Vertex =
		{
			cbuffer PerObject
			{
				float4x4 matWorldViewProj;
			}

			void main(
			  in float4 inPos : POSITION,
			  in float2 uv : TEXCOORD0,
			  out float4 oPosition : SV_Position,
			  out float2 oUv : TEXCOORD0)
			{
			   oPosition = mul(matWorldViewProj, inPos);
			   oUv = uv;
			}
		};
		
		Fragment =
		{
			SamplerState samp : register(s0);
			Texture2D tex : register(t0);

			float4 main(in float4 inPos : SV_Position, float2 uv : TEXCOORD0) : SV_Target
			{
			   return tex.Sample(samp, uv);
			}
		};
	};
};

Technique =
{
	Language = "GLSL";
	
	Pass =
	{

		Vertex =
		{
			uniform PerObject
			{
				mat4 matWorldViewProj;
			};

			in vec4 bs_position;
			in vec2 bs_texcoord0;
			out vec2 texcoord0;

			out gl_PerVertex
			{
				vec4 gl_Position;
			};
			
			void main()
			{
			   texcoord0 = bs_texcoord0;
			   gl_Position = matWorldViewProj * bs_position;
			}
		};
		
		Fragment =
		{
			uniform sampler2D tex;

			in vec2 texcoord0;
			out vec4 fragColor;

			void main()
			{
			   fragColor = texture2D(tex, texcoord0.st);
			}
		};
	};
};

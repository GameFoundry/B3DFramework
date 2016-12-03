Parameters =
{
	mat4x4		gMatViewProj;
	float		gAlphaCutoff;
	
	Sampler2D 	gMainTexSamp : alias("gMainTexture");
	Texture2D 	gMainTexture;	
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

		Common =
		{
			cbuffer Uniforms
			{
				float4x4 	gMatViewProj;
				float		gAlphaCutoff;
			}
		};		
		
		Vertex =
		{
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

			float4 main(in float4 inPos : SV_Position, 
						   in float2 uv : TEXCOORD0,
						   in float4 inColor : COLOR0) : SV_Target
			{
				float4 color = gMainTexture.Sample(gMainTexSamp, uv);
				if(color.a < gAlphaCutoff)
					discard;
				
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

		Common =
		{
			layout(binding = 0, std140) uniform Uniforms
			{
				mat4 	gMatViewProj;
				float	gAlphaCutoff;
			};
		};
		
		Vertex =
		{
			layout(location = 0) in vec3 bs_position;
			layout(location = 1) in vec4 bs_color0;
			layout(location = 2) in vec2 bs_texcoord0;
			
			layout(location = 0) out vec2 texcoord0;
			layout(location = 1) out vec4 color0;

			out gl_PerVertex
			{
				vec4 gl_Position;
			};
			
			void main()
			{
				gl_Position = gMatViewProj * vec4(bs_position.xyz, 1);
				texcoord0 = bs_texcoord0;
				color0 = bs_color0;
			}
		};
		
		Fragment =
		{
			layout(location = 0) in vec2 texcoord0;
			layout(location = 1) in vec4 color0;
			
			layout(location = 0) out vec4 fragColor;
			
			layout(binding = 1) uniform sampler2D gMainTexture;
			
			void main()
			{
				vec4 texColor = texture2D(gMainTexture, texcoord0);
				if(texColor.a < gAlphaCutoff)
					discard;
				
				fragColor = color0;
			}
		};
	};
};
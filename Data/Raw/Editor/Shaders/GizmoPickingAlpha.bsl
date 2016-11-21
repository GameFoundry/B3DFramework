Parameters =
{
	mat4x4		matViewProj;
	float		alphaCutoff;
	
	Sampler2D 	mainTexSamp : alias("mainTexture");
	Texture2D 	mainTexture;	
};

Technique =
{
	Language = "HLSL11";
	
	Pass =
	{
		Scissor = true;

		Vertex =
		{
			float4x4 matViewProj;

			void main(
				in float3 inPos : POSITION,
				in float2 uv : TEXCOORD0,
				in float4 color : COLOR0,
				out float4 oPosition : SV_Position,
				out float2 oUv : TEXCOORD0,
				out float4 oColor : COLOR0)
			{
				oPosition = mul(matViewProj, float4(inPos.xyz, 1));
				oUv = uv;
				oColor = color;
			}
		};
		
		Fragment =
		{
			SamplerState mainTexSamp : register(s0);
			Texture2D mainTexture : register(t0);

			float alphaCutoff;

			float4 main(in float4 inPos : SV_Position, 
						   in float2 uv : TEXCOORD0,
						   in float4 inColor : COLOR0) : SV_Target
			{
				float4 color = mainTexture.Sample(mainTexSamp, uv);
				if(color.a < alphaCutoff)
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
			
			layout(binding = 0) uniform VertUBO
			{
				mat4 matViewProj;
			};
			
			void main()
			{
				gl_Position = matViewProj * vec4(bs_position.xyz, 1);
				texcoord0 = bs_texcoord0;
				color0 = bs_color0;
			}
		};
		
		Fragment =
		{
			layout(location = 0) in vec2 texcoord0;
			layout(location = 1) in vec4 color0;
			
			layout(location = 0) out vec4 fragColor;

			layout(binding = 1) uniform FragUBO
			{
				float alphaCutoff;
			};
			
			layout(binding = 2) uniform sampler2D mainTexture;
			
			void main()
			{
				vec4 texColor = texture2D(mainTexture, texcoord0);
				if(texColor.a < alphaCutoff)
					discard;
				
				fragColor = color0;
			}
		};
	};
};
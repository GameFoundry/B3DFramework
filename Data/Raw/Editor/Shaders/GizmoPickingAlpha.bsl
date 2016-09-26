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
	Language = "HLSL9";
	
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
				out float4 oPosition : POSITION,
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
			sampler2D mainTexture;
			float alphaCutoff;

			float4 main(in float4 inPos : POSITION, 
					in float2 inUv : TEXCOORD0,
					in float4 inColor : COLOR0) : COLOR0
			{
				float4 color = tex2D(mainTexture, inUv);
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
			uniform mat4 matViewProj;
			in vec3 bs_position;
			in vec4 bs_color0;
			in vec2 bs_texcoord0;
			out vec2 texcoord0;
			out vec4 color0;

			out gl_PerVertex
			{
				vec4 gl_Position;
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
			uniform sampler2D mainTexture;
			uniform float alphaCutoff;
			in vec2 texcoord0;
			in vec4 color0;
			out vec4 fragColor;

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
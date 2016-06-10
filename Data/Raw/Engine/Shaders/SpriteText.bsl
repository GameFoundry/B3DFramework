Parameters =
{
	mat4x4 	worldTransform;
	float	invViewportWidth;
	float	invViewportHeight;
	
	Sampler2D	mainTexSamp : alias("mainTexture");
	Texture2D	mainTexture;
	
	float4		tint;
};

Technique =
{
	Language = "HLSL11";
	
	Pass =
	{
		Target = 
		{
			Blend = true;
			Color = { SRCA, SRCIA, ADD };
			WriteMask = RGB;
		};	
	
		DepthRead = false;
		DepthWrite = false;
		
		Vertex =
		{
			float invViewportWidth;
			float invViewportHeight;
			float4x4 worldTransform;

			void main(
				in float3 inPos : POSITION,
				in float2 uv : TEXCOORD0,
				out float4 oPosition : SV_Position,
				out float2 oUv : TEXCOORD0)
			{
				float4 tfrmdPos = mul(worldTransform, float4(inPos.xy, 0, 1));

				float tfrmdX = -1.0f + (tfrmdPos.x * invViewportWidth);
				float tfrmdY = 1.0f - (tfrmdPos.y * invViewportHeight);

				oPosition = float4(tfrmdX, tfrmdY, 0, 1);
				oUv = uv;
			}
		};
		
		Fragment =
		{
			SamplerState mainTexSamp : register(s0);
			Texture2D mainTexture : register(t0);
			float4 tint;

			float4 main(in float4 inPos : SV_Position, float2 uv : TEXCOORD0) : SV_Target
			{
				float4 color = float4(tint.rgb, mainTexture.Sample(mainTexSamp, uv).r * tint.a);
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
		Target = 
		{
			Blend = true;
			Color = { SRCA, SRCIA, ADD };
			WriteMask = RGB;
		};
		
		DepthRead = false;
		DepthWrite = false;
		
		Vertex =
		{
			float invViewportWidth;
			float invViewportHeight;
			float4x4 worldTransform;

			void main(
			in float3 inPos : POSITION,
			in float2 uv : TEXCOORD0,
			out float4 oPosition : POSITION,
			out float2 oUv : TEXCOORD0)
			{
				float4 tfrmdPos = mul(worldTransform, float4(inPos.xy, 0, 1));

				float tfrmdX = -1.0f + ((tfrmdPos.x - 0.5f) * invViewportWidth);
				float tfrmdY = 1.0f - ((tfrmdPos.y - 0.5f) * invViewportHeight);

				oPosition = float4(tfrmdX, tfrmdY, 0, 1);
				oUv = uv;
			}		
		};
		
		Fragment =
		{
			sampler2D mainTexture;
			float4 tint;

			float4 main(float2 uv : TEXCOORD0) : COLOR0
			{
				float4 color = float4(tint.rgb, tex2D(mainTexture, uv).r * tint.a);
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
		Target = 
		{
			Blend = true;
			Color = { SRCA, SRCIA, ADD };
			WriteMask = RGB;
		};
		
		DepthRead = false;
		DepthWrite = false;
		
		Vertex =
		{
			uniform float invViewportWidth;
			uniform float invViewportHeight;
			uniform mat4 worldTransform;

			in vec3 bs_position;
			in vec2 bs_texcoord0;
			out vec2 texcoord0;

			out gl_PerVertex
			{
				vec4 gl_Position;
			};
			
			void main()
			{
				vec4 tfrmdPos = worldTransform * vec4(bs_position.xy, 0, 1);

				float tfrmdX = -1.0f + (tfrmdPos.x * invViewportWidth);
				float tfrmdY = 1.0f - (tfrmdPos.y * invViewportHeight);

				gl_Position = vec4(tfrmdX, tfrmdY, 0, 1);
				texcoord0 = bs_texcoord0;
			}
		};
		
		Fragment =
		{
			uniform sampler2D mainTexture;
			uniform vec4 tint;

			in vec2 texcoord0;
			out vec4 fragColor;

			void main()
			{
				vec4 color = vec4(tint.rgb, texture2D(mainTexture, texcoord0.st).r * tint.a);
				fragColor = color;
			}
		};
	};
};
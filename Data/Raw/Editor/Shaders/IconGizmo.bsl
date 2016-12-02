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
	Language = "HLSL11";
	
	Pass =
	{
		Target = 
		{
			Blend = true;
			Color = { SRCA, SRCIA, ADD };
		};
		
		DepthWrite = false;
		CompareFunc = LTE;
		
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
				return color * gMainTexture.Sample(gMainTexSamp, uv);
			}		
		};
	};
	
	Pass =
	{
		Target = 
		{
			Blend = true;
			Color = { SRCA, SRCIA, ADD };
		};
		
		DepthWrite = false;
		CompareFunc = GT;
		
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
				in float4 color : COLOR1,
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
				return color * gMainTexture.Sample(gMainTexSamp, uv);
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
		};
		
		DepthWrite = false;
		CompareFunc = LTE;
		
		Vertex =
		{
			layout(location = 0) in vec3 bs_position;
			layout(location = 1) in vec4 bs_color0;
			layout(location = 2) in vec2 bs_texcoord0;
			
			layout(location = 0) out vec4 color0;
			layout(location = 1) out vec2 texcoord0;

			out gl_PerVertex
			{
				vec4 gl_Position;
			};		
		
			layout(binding = 0, std140) uniform Uniforms
			{
				mat4 	gMatViewProj;
				vec4	gViewDir;
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
			layout(location = 0) in vec4 color0;
			layout(location = 1) in vec2 texcoord0;
			layout(location = 0) out vec4 fragColor;

			layout(binding = 1) uniform sampler2D gMainTexture;
			
			void main()
			{
				vec4 texColor = texture2D(gMainTexture, texcoord0.st);
				fragColor = color0 * texColor;
			}		
		};
	};
	
	Pass =
	{
		Target = 
		{
			Blend = true;
			Color = { SRCA, SRCIA, ADD };
		};
		
		DepthWrite = false;
		CompareFunc = GT;
		
		Vertex =
		{
			layout(location = 0) in vec3 bs_position;
			layout(location = 1) in vec4 bs_color1;
			layout(location = 2) in vec2 bs_texcoord0;
			
			layout(location = 0) out vec4 color0;
			layout(location = 1) out vec2 texcoord0;

			out gl_PerVertex
			{
				vec4 gl_Position;
			};

			layout(binding = 0, std140) uniform Uniforms
			{
				mat4 	gMatViewProj;
				vec4	gViewDir;
			};
			
			void main()
			{
				gl_Position = gMatViewProj * vec4(bs_position.xyz, 1);
				texcoord0 = bs_texcoord0;
				color0 = bs_color1;
			}		
		};
		
		Fragment =
		{
			layout(location = 0) in vec4 color0;
			layout(location = 1) in vec2 texcoord0;
			layout(location = 0) out vec4 fragColor;

			layout(binding = 1) uniform sampler2D gMainTexture;
			
			void main()
			{
				vec4 texColor = texture2D(gMainTexture, texcoord0.st);
				fragColor = color0 * texColor;
			}		
		};
	};	
};	
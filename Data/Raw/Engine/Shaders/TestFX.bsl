Separable = false;

Parameters = 
{
	float test1 = 1.0f;
	float2 test2 = { 1.0f, 1.0f };
	float3 test3 = { 1.0f, 1.0f, 1.0f };
	float4 test4 = { 1.0f, 1.0f, 1.0f, 1.0f };
	mat2x2 test5 = 
		{ 1.0f, 0.0f,
		  0.0f,  1.0f };
	mat2x3 test6 =
		{ 1.0f, 0.0f, 0.0f,
		  0.0f, 1.0f, 0.0f };
	mat2x4 test7 = 
		{ 1.0f, 0.0f, 0.0f, 0.0f,
		  0.0f, 1.0f, 0.0f, 0.0f };
	mat3x2 test8 = 
		{ 1.0f, 0.0f,
		  0.0f, 1.0f,
		  0.0f, 0.0f };
	mat3x3 test9 =
		{ 1.0f, 0.0f, 0.0f,
		  0.0f, 1.0f, 0.0f,
		  0.0f, 0.0f, 1.0f };
	mat3x4 test10 =
		{ 1.0f, 0.0f, 0.0f, 0.0f,
		  0.0f, 1.0f, 0.0f, 0.0f,
		  0.0f, 0.0f, 1.0f, 0.0f };
	mat4x2 test11 =
		{ 1.0f, 0.0f,
		  0.0f, 1.0f,
		  0.0f, 0.0f,
		  0.0f, 0.0f };
	mat4x3 test12 =
		{ 1.0f, 0.0f, 0.0f,
		  0.0f, 1.0f, 0.0f,
		  0.0f, 0.0f, 1.0f,
		  0.0f, 0.0f, 0.0f };
	mat4x4 test13 =
		{ 1.0f, 0.0f, 0.0f, 0.0f,
		  0.0f, 1.0f, 0.0f, 0.0f,
		  0.0f, 0.0f, 1.0f, 0.0f,
		  0.0f, 0.0f, 0.0f, 1.0f };
	Texture2D tex0;
	Sampler2D samp0 : alias("tex0") =
	{
		AddressMode =
		{
			U = WRAP;
			V = WRAP;
			W = WRAP;
		};
		
		MinFilter = LINEAR;
		MagFilter = LINEAR;
		MipFilter = LINEAR;
		MaxAniso = 0;
		MipmapBias = 0.0f;
		MipMin = 0.0f;
		MipMax = 0.0f;
		BorderColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		CompareFunc = PASS;
	};
	Texture1D tex1;
	Sampler1D samp1;
	Texture3D tex2;
	Sampler3D samp2;
	TextureCUBE tex3;
	SamplerCUBE samp3;
	Texture2DMS tex4;
	Sampler2DMS samp4;
	ByteBuffer buffer1;
	StructBuffer buffer2;
	RWByteBuffer buffer3;
	RWStructBuffer buffer4;
	AppendBuffer buffer5;
	ConsumeBuffer buffer6;
	RWTypedBuffer buffer7;			  
};

Blocks = 
{
	Block global : auto("GLOBAL") : shared(false) : usage(STATIC);
	Block perFrame : auto("PERFRAME") : shared(false) : usage(DYNAMIC);
};

Technique = 
{
	Renderer = "Any";
	Tags = { "Animated", "OtherTag" };

	Pass =
	{
		Fill = SOLID;
		Cull = CCW;
		DepthBias = 0.0f;
		ScaledDepthBias = 0.0f;
		DepthClip = true;
		Scissor = false;
		Multisample = false;
		AALine = false;
		
		DepthRead = true;
		DepthWrite = true;
		CompareFunc = GTE;
		Stencil = false;
		StencilReadMask = 0xff;
		StencilWriteMask = 0xff;
		StencilOpFront = 
		{
			Fail = KEEP;
			ZFail = KEEP;
			Pass = KEEP;
			CompareFunc = FAIL;
		};
		StencilOpBack =
		{
			Fail = KEEP;
			ZFail = KEEP;
			Pass = KEEP;
			CompareFunc = FAIL;
		};
		
		AlphaToCoverage = false;
		IndependantBlend = false;
		Target =
		{
			Index = 0;
			Blend = false;
			Color =
			{
				Source = ONE;
				Dest = ONE;
				Op =  ADD;
			};
			Alpha =
			{
				Source = ONE;
				Dest = ONE;
				Op =  ADD;
			};
			WriteMask = RGBA;
		};
		
		Vertex = 
		{
			cbuffer VertParams
			{
				float invViewportWidth;
				float invViewportHeight;
				float4x4 worldTransform;
			};
			
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
			
			cbuffer FragParams
			{
				float4 tint;
			};
			
			float4 main(in float4 inPos : SV_Position, float2 uv : TEXCOORD0) : SV_Target
			{
				float4 color = mainTexture.Sample(mainTexSamp, uv);
				return color * tint;
			}
		};	
	};	
};
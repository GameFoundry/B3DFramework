shader SpriteImage
{
	variations
	{
		TRANSPARENT = { false, true };
		ANIMATED = { false, true };
	};

	depth
	{
		read = false;
		write = false;
	};
	
	#if TRANSPARENT
	blend
	{
		target	
		{
			enabled = true;
			color = { srcA, srcIA, add };
			writemask = RGB;
		};
	};
	#endif

	code
	{
		cbuffer GUIParams
		{
			float4x4 gWorldTransform;
			float gInvViewportWidth;
			float gInvViewportHeight;
			float gViewportYFlip;
			float4 gTint;
			float4 gUVSizeOffset;
		}	

		void vsmain(
			in float3 inPos : POSITION,
			in float2 uv : TEXCOORD0,
			out float4 oPosition : SV_Position,
			out float2 oUv : TEXCOORD0)
		{
			float4 tfrmdPos = mul(gWorldTransform, float4(inPos.xy, 0, 1));

			float tfrmdX = -1.0f + (tfrmdPos.x * gInvViewportWidth);
			float tfrmdY = (1.0f - (tfrmdPos.y * gInvViewportHeight)) * gViewportYFlip;

			oPosition = float4(tfrmdX, tfrmdY, 0, 1);
			
			#if ANIMATED
			oUv = uv * gUVSizeOffset.xy + gUVSizeOffset.zw;
			#else
			oUv = uv;
			#endif
		}

		[alias(gMainTexture)]
		SamplerState gMainTexSamp;
		Texture2D gMainTexture;
		
		float4 fsmain(in float4 inPos : SV_Position, float2 uv : TEXCOORD0) : SV_Target
		{
			float4 color = gMainTexture.Sample(gMainTexSamp, uv);
			return color * gTint;
		}
	};
};
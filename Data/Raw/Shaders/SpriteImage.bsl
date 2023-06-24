#define UV 1
#include "$ENGINE$\SpriteCommon.bslinc"

shader SpriteImage
{
	mixin SpriteCommon;

	variations
	{
		TRANSPARENCY = { 
			0, // None
			1, // Normal
			2, // Premultiplied
		};
		ANIMATED = { false, true };
	};

	code
	{
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

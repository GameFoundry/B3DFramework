#define UV 1
#define TRANSPARENCY 1
#include "$ENGINE$\SpriteCommon.bslinc"

shader SpriteText
{
	mixin SpriteCommon;

	code
	{
		[alias(gMainTexture)]
		SamplerState gMainTexSamp;
		Texture2D gMainTexture;

		float4 fsmain(in float4 inPos : SV_Position, float2 uv : TEXCOORD0) : SV_Target
		{
			return float4(gTint.rgb, gMainTexture.Sample(gMainTexSamp, uv).r * gTint.a);
		}
	};
};

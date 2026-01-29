#define ENABLE_UV 1
#define TRANSPARENCY 1
#include "$ENGINE$\SpriteCommon.bslinc"

shader SpriteText
{
	mixin SpriteCommon;

	variations
	{
		ENABLE_CLIPPING = { false, true };
	};

	code
	{
		[alias(gMainTexture)]
		SamplerState gMainTexSamp;
		Texture2D gMainTexture;

		float4 fsmain(in float4 inPosition : SV_Position, float2 inUV : TEXCOORD0
		    #if ENABLE_CLIPPING
		    , in uint instanceId : TEXCOORD1
		    #endif
		) : SV_Target
		{
		    #if ENABLE_CLIPPING
            const int2 pixelPosition = (int2)inPosition.xy;
            const Area2DInt clipRegion = gClipRegions[instanceId];

            if(!IsInClipRegion(clipRegion, pixelPosition))
                discard;
		    #endif

			return float4(gTint.rgb, gMainTexture.Sample(gMainTexSamp, inUV).r * gTint.a);
		}
	};
};

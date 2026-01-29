#define TRANSPARENCY 1
#include "$ENGINE$\SpriteCommon.bslinc"

shader SpriteLine
{
	mixin SpriteCommon;

	variations
	{
		ENABLE_CLIPPING = { false, true };
	};

	raster
	{
		multisample = false; // This controls line rendering algorithm
		lineaa = false;
	};
	
	code
	{
		float4 fsmain(in float4 inPosition : SV_Position
            #if ENABLE_CLIPPING
            , uint instanceId : TEXCOORD1
            #endif
        ) : SV_Target
		{
		    #if ENABLE_CLIPPING
            const int2 pixelPosition = (int2)inPosition.xy;
            const Area2DInt clipRegion = gClipRegions[instanceId];

            if(!IsInClipRegion(clipRegion, pixelPosition))
                discard;
		    #endif

			return gTint;
		}
	};
};

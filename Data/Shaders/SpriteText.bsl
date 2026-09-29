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

		/**
		 * Adjusts glyph coverage so text weight doesn't depend on its color, as coverage blended in gamma space thins light
		 * text and thickens dark text. the background is assumed to be the luminance inverse of the text, coverage gets
		 * a contrast boost, and the returned coverage makes the gamma-space blend produce the linear-space blend of the boosted coverage.
		 */
		float CorrectTextCoverage(float coverage)
		{
			float boosted = min(coverage + (1.0f - coverage) * gTextCoverageParams.x * coverage, 1.0f);
			if(gTextCoverageBlend.y == 0.0f)
				return boosted;

			float linearOutput = lerp(gTextCoverageParams.w, gTextCoverageParams.z, boosted);
			return saturate((pow(linearOutput, gTextCoverageParams.y) - gTextCoverageBlend.x) * gTextCoverageBlend.y);
		}

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

			return float4(gTint.rgb, CorrectTextCoverage(gMainTexture.Sample(gMainTexSamp, inUV).r) * gTint.a);
		}
	};
};

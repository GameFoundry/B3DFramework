// NOTE: All the vector graphics code was ported from nanovg's OpenGL renderer
shader VectorGraphics
{
    variations
	{
        DRAW_MODE =
        {
            0, // Draw fill shapes - IncrementWrap on pass front, DecrementWrap on pass back, no color write, no culling
			1, // Draw fill AA pixels - Read only stencil
			2, // Draw fill - Only draw where stencil is non-zero & clear stencil to zero (Following DRAW_MODE 0)
			3, // Draw stroke - Increment stencil on pass
			4, // Draw stroke AA pixels - Read only stencil
			5, // Clear stencil buffer - No color write
			6, // Convex fill - No stencil
        };

        BLEND_MODE =
        {
            0,  // NVG_SOURCE_OVER
            1,  // NVG_SOURCE_IN
            2,  // NVG_SOURCE_OUT
            3,  // NVG_ATOP
            4,  // NVG_DESTINATION_OVER
            5,  // NVG_DESTINATION_IN
            6,  // NVG_DESTINATION_OUT
            7,  // NVG_DESTINATION_ATOP
            8,  // NVG_LIGHTER
            9,  // NVG_COPY
            10, // NVG_XOR
        };

        EDGE_AA = { true, false };
    };

	depth
	{
		read = false;
		write = false;
	};

#if DRAW_MODE == 0
    raster
    {
        cull = none;
    };
#endif

	blend
	{
		target
		{
#if DRAW_MODE == 0
			enabled = false;
			writemask = empty;
#elif DRAW_MODE == 5
			enabled = false;
			writemask = empty;
#else
			enabled = true;
			writemask = RGBA;
#endif

			// NVG_SOURCE_OVER
#if BLEND_MODE == 0
			color = { one, srcIA, add };
			alpha = { one, srcIA, add };
#endif

			// NVG_SOURCE_IN
#if BLEND_MODE == 1
			color = { dstA, zero, add };
			alpha = { dstA, zero, add };
#endif

			// NVG_SOURCE_OUT
#if BLEND_MODE == 2
			color = { dstIA, zero, add };
			alpha = { dstIA, zero, add };
#endif

			// NVG_ATOP
#if BLEND_MODE == 3
			color = { dstA, srcIA, add };
			alpha = { dstA, srcIA, add };
#endif

			// NVG_DESTINATION_OVER
#if BLEND_MODE == 4
			color = { dstIA, one, add };
			alpha = { dstIA, one, add };
#endif

			// NVG_DESTINATION_IN
#if BLEND_MODE == 5
			color = { zero, srcA, add };
			alpha = { zero, srcA, add };
#endif

			// NVG_DESTINATION_OUT
#if BLEND_MODE == 6
			color = { zero, srcIA, add };
			alpha = { zero, srcIA, add };
#endif

			// NVG_DESTINATION_ATOP
#if BLEND_MODE == 7
			color = { dstIA, srcA, add };
			alpha = { dstIA, srcA, add };
#endif

			// NVG_LIGHTER
#if BLEND_MODE == 8
			color = { one, one, add };
			alpha = { one, one, add };
#endif

			// NVG_COPY
#if BLEND_MODE == 9
			color = { one, zero, add };
			alpha = { one, zero, add };
#endif

			// NVG_XOR
#if BLEND_MODE == 10
			color = { dstIA, srcIA, add };
			alpha = { dstIA, srcIA, add };
#endif
		};
	};

#if DRAW_MODE != 6
    stencil
    {
        enabled = true;
        writemask = 0xff;
        readmask = 0xff;

#if DRAW_MODE == 0
        front =
            {
                fail = keep;
                zfail = keep;
                pass = incwrap;
                compare = always;
            };
        back =
            {
                fail = keep;
                zfail = keep;
                pass = decwrap;
                compare = always;
            };
#elif DRAW_MODE == 1
        front = { keep, keep, keep, eq };
        back  = { keep, keep, keep, eq };
#elif DRAW_MODE == 2
		front = { zero, zero, zero, neq };
		back  = { zero, zero, zero, neq };
#elif DRAW_MODE == 3
		front = { keep, keep, inc, eq };
		back  = { keep, keep, inc, eq };
#elif DRAW_MODE == 4
		front = { keep, keep, keep, eq };
		back  = { keep, keep, keep, eq };
#elif DRAW_MODE == 5
		front = { zero, zero, zero, always };
		back  = { zero, zero, zero, always };
#endif
    };
#endif

	code
	{
		cbuffer RenderUniforms
		{
			float4x4 gScissorMatrix;
			float4x4 gPaintMatrix;
			float4 gInnerColor;
			float4 gOuterColor;
			float2 gScissorExtents;
			float2 gScissorScale;
			float2 gExtent;
			float gRadius;
			float gFeather;
			float gStrokeMultiplier;
			float gStrokeThreshold;
		}

		cbuffer ViewUniforms
		{
			float2 gViewportOffset;
			float2 gInverseViewportHalfSize;
			float gViewportYFlip;
		}

        void vsmain(
            in float3 inPosition : POSITION,
            in float2 inUV : TEXCOORD0,
            out float4 outPosition : SV_Position,
            out float2 outUV : TEXCOORD0,
            out float2 outVertexPosition : VPOS
            )
        {
            outPosition.x = -1.0f + ((gViewportOffset.x + inPosition.x) * gInverseViewportHalfSize.x);
            outPosition.y = (1.0f - ((gViewportOffset.y + inPosition.y) * gInverseViewportHalfSize.y)) * gViewportYFlip;
			outPosition.zw = float2(0.0f, 1.0f);

			outUV = inUV;
			outVertexPosition = inPosition;
        }

		float NVGRoundRect(float2 pt, float2 ext, float rad)
		{
			float2 ext2 = ext - float2(rad, rad);
			float2 d = abs(pt) - ext2;

			return min(max(d.x,d.y),0.0) + length(max(d,0.0)) - rad;
		}

		float NVGScissorMask(float2 p)
		{
			float2 sc = (abs((mul(gScissorMatrix, float3(p.x, p.y, 1.0f))).xy) - gScissorExtents);
			sc = float2(0.5f, 0.5f) - sc * gScissorScale;
			return clamp(sc.x, 0.0f, 1.0f) * clamp(sc.y, 0.0f, 1.0f);
		}

		// Stroke - from [0..1] to clipped pyramid, where the slope is 1px.
		float NVGStrokeMask(float2 uv)
		{
			return min(1.0f, (1.0f - abs(uv.x * 2.0f - 1.0f)) * gStrokeMultiplier) * min(1.0f, uv.y);
		}

		float4 fsmain(in float4 inPosition : SV_Position, in float2 inUV : TEXCOORD0, in float2 inVertexPosition : VPOS) : SV_Target
		{
#if DRAW_MODE != 0
			const float scissor = NVGScissorMask(inVertexPosition);
#	if EDGE_AA
			const float strokeAlpha = NVGStrokeMask(inUV);
			if(strokeAlpha < gStrokeThreshold) discard;
#	else
			const float strokeAlpha = 1.0f;
#	endif

			// Calculate gradient color using box gradient
			float2 pt = (mul((float3x3)gPaintMatrix, float3(inVertexPosition, 1.0f))).xy;
			float d = clamp((NVGRoundRect(pt, gExtent, gRadius) + gFeather * 0.5f) / gFeather, 0.0f, 1.0f);
			float4 color = lerp(gInnerColor, gOuterColor, d);

			// Combine alpha
			color *= strokeAlpha * scissor;
			return color;
#else
			return float4(1.0f, 1.0f, 1.0f, 1.0f);
#endif
		}
	};
};
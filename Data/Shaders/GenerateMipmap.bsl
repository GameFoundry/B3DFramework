shader GenerateMipmap
{
	featureset = HighEnd;

	code
	{
		// Source mip as RGBA float, laid out row-major: index = y * srcWidth + x.
		Buffer<float4> gInput;
		// Destination mip (one level down). Row-major: index = y * dstWidth + x.
		RWBuffer<float4> gOutput;
		// gParams[0] = (srcWidth, srcHeight, dstWidth, dstHeight)
		// gParams[1] = (filter, isSrgb, normalize, wrapMode)
		//   filter:   0 = box, 1 = triangle
		//   wrapMode: 0 = mirror, 1 = repeat, 2 = clamp
		Buffer<int4> gParams;

		float3 SrgbToLinear(float3 c)
		{
			// Matches the simple gamma 2.2 curve nvtt used (setGamma(2.2, 2.2)).
			return pow(max(c, 0.0f), 2.2f);
		}

		float3 LinearToSrgb(float3 c)
		{
			return pow(max(c, 0.0f), 1.0f / 2.2f);
		}

		// Wraps an out-of-range coordinate back into [0, size) according to the wrap mode.
		int WrapCoord(int v, int size, int mode)
		{
			if (mode == 1) // Repeat
			{
				v = v % size;
				if (v < 0)
					v += size;
				return v;
			}
			else if (mode == 2) // Clamp
			{
				return clamp(v, 0, size - 1);
			}
			else // Mirror
			{
				if (size == 1)
					return 0;
				int period = 2 * size;
				v = v % period;
				if (v < 0)
					v += period;
				if (v >= size)
					v = period - 1 - v;
				return v;
			}
		}

		// Loads a source texel, optionally converting from sRGB to linear for correct filtering.
		float4 LoadTexel(int x, int y, int srcW, int srcH, int wrapMode, bool srgb)
		{
			int sx = WrapCoord(x, srcW, wrapMode);
			int sy = WrapCoord(y, srcH, wrapMode);
			float4 t = gInput[sy * srcW + sx];
			if (srgb)
				t.rgb = SrgbToLinear(t.rgb);
			return t;
		}

		[numthreads(8, 8, 1)]
		void csmain(uint3 dispatchId : SV_DispatchThreadID)
		{
			int srcW = gParams[0].x;
			int srcH = gParams[0].y;
			int dstW = gParams[0].z;
			int dstH = gParams[0].w;

			int filter = gParams[1].x;
			bool srgb = gParams[1].y != 0;
			bool normalizeResult = gParams[1].z != 0;
			int wrapMode = gParams[1].w;

			int2 dst = int2(dispatchId.xy);
			if (dst.x >= dstW || dst.y >= dstH)
				return;

			// 1 (dimension already at 1) or 2 (halving) for each axis.
			int ratioX = srcW / dstW;
			int ratioY = srcH / dstH;

			float4 result = float4(0, 0, 0, 0);

			if (filter == 1 && (ratioX == 2 || ratioY == 2))
			{
				// Separable triangle [1,3,3,1]/8 filter over a 4x4 neighbourhood centred on
				// the source footprint. Border taps use the wrap mode.
				float w4[4];
				w4[0] = 1.0f; w4[1] = 3.0f; w4[2] = 3.0f; w4[3] = 1.0f;

				int baseX = dst.x * ratioX - 1;
				int baseY = dst.y * ratioY - 1;
				float totalW = 0.0f;
				[unroll]
				for (int iy = 0; iy < 4; ++iy)
				{
					[unroll]
					for (int ix = 0; ix < 4; ++ix)
					{
						float w = w4[ix] * w4[iy];
						result += w * LoadTexel(baseX + ix, baseY + iy, srcW, srcH, wrapMode, srgb);
						totalW += w;
					}
				}
				result /= totalW;
			}
			else
			{
				// Box filter over the ratioX x ratioY source footprint (2x2, 2x1 or 1x2).
				int baseX = dst.x * ratioX;
				int baseY = dst.y * ratioY;
				float totalW = 0.0f;
				[unroll]
				for (int iy = 0; iy < 2; ++iy)
				{
					[unroll]
					for (int ix = 0; ix < 2; ++ix)
					{
						if (ix < ratioX && iy < ratioY)
						{
							result += LoadTexel(baseX + ix, baseY + iy, srcW, srcH, wrapMode, srgb);
							totalW += 1.0f;
						}
					}
				}
				result /= totalW;
			}

			// Re-normalize a (linear-space) normal vector before any sRGB re-encode.
			if (normalizeResult)
			{
				float3 n = result.rgb * 2.0f - 1.0f;
				float len = length(n);
				if (len > 1e-6f)
					n /= len;
				result.rgb = n * 0.5f + 0.5f;
			}

			if (srgb)
				result.rgb = LinearToSrgb(result.rgb);

			gOutput[dst.y * dstW + dst.x] = result;
		}
	};
};

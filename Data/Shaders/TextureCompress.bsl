// BC6H / BC7 mode encoders live in their own headers (one mode per file where practical) to keep this dispatcher small;
// see Includes/TextureCompressBC7Mode*.bslinc. BC1/BC4 stay inline as they are tiny single-mode encoders.
#include "$ENGINE$\TextureCompression\TextureCompressBC7Mode6.bslinc"
#include "$ENGINE$\TextureCompression\TextureCompressBC7Mode1.bslinc"
#include "$ENGINE$\TextureCompression\TextureCompressBC7Mode3.bslinc"
#include "$ENGINE$\TextureCompression\TextureCompressBC7Mode2.bslinc"
#include "$ENGINE$\TextureCompression\TextureCompressBC7Mode0.bslinc"
#include "$ENGINE$\TextureCompression\TextureCompressBC7Mode5.bslinc"
#include "$ENGINE$\TextureCompression\TextureCompressBC7Mode7.bslinc"
#include "$ENGINE$\TextureCompression\TextureCompressBC7Mode4.bslinc"
#include "$ENGINE$\TextureCompression\TextureCompressBC6Mode1.bslinc"
#include "$ENGINE$\TextureCompression\TextureCompressBC6Mode2.bslinc"
#include "$ENGINE$\TextureCompression\TextureCompressBC6Mode3.bslinc"
#include "$ENGINE$\TextureCompression\TextureCompressBC6Mode4.bslinc"
#include "$ENGINE$\TextureCompression\TextureCompressBC6Mode5.bslinc"
#include "$ENGINE$\TextureCompression\TextureCompressBC6Mode6.bslinc"
#include "$ENGINE$\TextureCompression\TextureCompressBC6Mode7.bslinc"
#include "$ENGINE$\TextureCompression\TextureCompressBC6Mode8.bslinc"
#include "$ENGINE$\TextureCompression\TextureCompressBC6Mode9.bslinc"
#include "$ENGINE$\TextureCompression\TextureCompressBC6Mode10.bslinc"
#include "$ENGINE$\TextureCompression\TextureCompressBC6Mode11.bslinc"
#include "$ENGINE$\TextureCompression\TextureCompressBC6Mode12.bslinc"
#include "$ENGINE$\TextureCompression\TextureCompressBC6Mode13.bslinc"
#include "$ENGINE$\TextureCompression\TextureCompressBC6Mode14.bslinc"

shader TextureCompress
{
	featureset = HighEnd;

	mixin TextureCompressBC7Mode6;
	mixin TextureCompressBC7Mode1;
	mixin TextureCompressBC7Mode3;
	mixin TextureCompressBC7Mode2;
	mixin TextureCompressBC7Mode0;
	mixin TextureCompressBC7Mode5;
	mixin TextureCompressBC7Mode7;
	mixin TextureCompressBC7Mode4;
	mixin TextureCompressBC6Mode1;
	mixin TextureCompressBC6Mode2;
	mixin TextureCompressBC6Mode3;
	mixin TextureCompressBC6Mode4;
	mixin TextureCompressBC6Mode5;
	mixin TextureCompressBC6Mode6;
	mixin TextureCompressBC6Mode7;
	mixin TextureCompressBC6Mode8;
	mixin TextureCompressBC6Mode9;
	mixin TextureCompressBC6Mode10;
	mixin TextureCompressBC6Mode11;
	mixin TextureCompressBC6Mode12;
	mixin TextureCompressBC6Mode13;
	mixin TextureCompressBC6Mode14;

	variations
	{
		// Target block-compressed format. Numbered sequentially by format, then by encoder mode within a format:
		//
		//   0       - BC1  (RGB, 64-bit block)
		//   1       - BC3  (RGBA: BC4 alpha block + BC1 color block, 128-bit)
		//   2       - BC4  (single red channel, 64-bit block)
		//   3       - BC5  (RG: two BC4 blocks, 128-bit)
		//   4 .. 17 - BC6H (RGB HDR / UF16, 128-bit), one encoder mode per variation: 4 = mode 1 ... 17 = mode 14
		//  18 .. 25 - BC7  (RGBA, 128-bit),           one encoder mode per variation: 18 = mode 0 ... 25 = mode 7
		FORMAT = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25 };
	};

	code
	{
		// 64-bit formats output a uint2 per block, 128-bit formats output a uint4. BC1 (0) and BC4 (2) are 64-bit.
		#if FORMAT == 0 || FORMAT == 2
			#define BLOCK_TYPE uint2
		#else
			#define BLOCK_TYPE uint4
		#endif

		// BC6H is dispatched as 14 single-mode kernels (FORMAT 4..17, encoder modes 1..14).
		#define IS_BC6H (FORMAT >= 4 && FORMAT <= 17)
		// BC6H two-region modes (1..10) run cooperatively: 32 threads/block, one per partition (FORMAT 4..13).
		#define IS_BC6H_TWOREGION (FORMAT >= 4 && FORMAT <= 13)
		// BC7 is dispatched as 8 single-mode kernels (FORMAT 18..25, encoder modes 0..7). Both share a running-best buffer.
		#define IS_BC7 (FORMAT >= 18 && FORMAT <= 25)

		// Source pixels as RGBA8 (read as normalized float4), laid out row-major: index = y * width + x.
		Buffer<float4> gInput;
		RWBuffer<BLOCK_TYPE> gOutput;
		// Two int2 entries: [0] = texture size in pixels, [1] = number of 4x4 blocks along each axis.
		Buffer<int2> gMeta;
		#if IS_BC7 || IS_BC6H
			// Per-block lowest error so far, carried between the BC7/BC6H mode-group dispatches. The seed group (FORMAT 4
			// for BC7, 5 for BC6H) writes it; later groups read it to continue the running minimum and write it back.
			RWBuffer<float> gBestErr;
		#endif

		uint PackColor565(float3 color)
		{
			uint r = (uint)round(saturate(color.r) * 31.0f);
			uint g = (uint)round(saturate(color.g) * 63.0f);
			uint b = (uint)round(saturate(color.b) * 31.0f);
			return (r << 11) | (g << 5) | b;
		}

		float3 Unpack565(uint c)
		{
			// Expand to 8-bit using bit replication, matching the hardware BCn decoder.
			// A plain divide (c/31) does NOT match the decoder and biases index selection.
			uint r5 = (c >> 11) & 0x1F;
			uint g6 = (c >> 5) & 0x3F;
			uint b5 = c & 0x1F;
			uint r8 = (r5 << 3) | (r5 >> 2);
			uint g8 = (g6 << 2) | (g6 >> 4);
			uint b8 = (b5 << 3) | (b5 >> 2);
			return float3(r8, g8, b8) / 255.0f;
		}

		// Encodes a 4x4 RGB block into a 64-bit BC1 block (no punch-through alpha).
		// Endpoints come from the principal axis of the block's colour distribution (PCA),
		// followed by one least-squares refinement pass over the assigned indices.
		uint2 CompressBC1(float3 texels[16])
		{
			// Mean, covariance and bounding box in a single pass.
			float3 mean = texels[0];
			[unroll]
			for (uint i = 1; i < 16; ++i)
				mean += texels[i];
			mean *= (1.0f / 16.0f);

			float cxx = 0, cxy = 0, cxz = 0, cyy = 0, cyz = 0, czz = 0;
			float3 minC = texels[0];
			float3 maxC = texels[0];
			[unroll]
			for (uint i = 0; i < 16; ++i)
			{
				float3 d = texels[i] - mean;
				cxx += d.x * d.x; cxy += d.x * d.y; cxz += d.x * d.z;
				cyy += d.y * d.y; cyz += d.y * d.z; czz += d.z * d.z;
				minC = min(minC, texels[i]);
				maxC = max(maxC, texels[i]);
			}

			// Power iteration for the dominant eigenvector of the covariance matrix. Seed
			// with the bounding-box diagonal so flat/degenerate blocks still get a sane axis.
			float3 axis = maxC - minC;
			if (dot(axis, axis) < 1e-8f)
				axis = float3(1, 1, 1);
			[unroll]
			for (uint k = 0; k < 8; ++k)
			{
				float3 v;
				v.x = cxx * axis.x + cxy * axis.y + cxz * axis.z;
				v.y = cxy * axis.x + cyy * axis.y + cyz * axis.z;
				v.z = cxz * axis.x + cyz * axis.y + czz * axis.z;
				float m = max(max(abs(v.x), abs(v.y)), abs(v.z));
				axis = (m > 1e-7f) ? (v / m) : axis;
			}

			// --- Cluster fit (squish style) -------------------------------------------------------
			// Range-fit (PCA extremes + Lloyd) leaves several dB on the table for smooth gradients because the index
			// boundaries are only locally optimal. Cluster fit instead projects the texels onto the principal axis,
			// sorts them, and enumerates every ordered split of the sorted run into the 4 palette levels {0,1/3,2/3,1};
			// for each split it solves the least-squares-optimal endpoint pair in closed form and keeps the lowest-error
			// one. The optimal index assignment is monotonic along the axis, so a sorted ordered partition is exhaustive.

			// PCA-extreme endpoints, kept only as a fallback seed if every cluster partition turns out degenerate.
			float minProj = 1e20f;
			float maxProj = -1e20f;
			[unroll]
			for (uint i = 0; i < 16; ++i)
			{
				float proj = dot(texels[i] - mean, axis);
				minProj = min(minProj, proj);
				maxProj = max(maxProj, proj);
			}
			float3 bestE0 = saturate(mean + axis * maxProj);
			float3 bestE1 = saturate(mean + axis * minProj);

			// Sort the texels by their projection onto the axis (ascending). Insertion sort over a runtime loop keeps
			// the IR small (no unrolled 16-wide sorting network); the inner shift is a runtime loop for the same reason.
			float3 sortedPts[16];
			float sortKey[16];
			[unroll]
			for (uint i = 0; i < 16; ++i)
			{
				sortedPts[i] = texels[i];
				sortKey[i] = dot(texels[i] - mean, axis);
			}
			for (uint a = 1; a < 16; ++a)
			{
				float keyP = sortKey[a];
				float3 keyV = sortedPts[a];
				uint b = a;
				[loop]
				while (b > 0 && sortKey[b - 1] > keyP)
				{
					sortKey[b] = sortKey[b - 1];
					sortedPts[b] = sortedPts[b - 1];
					--b;
				}
				sortKey[b] = keyP;
				sortedPts[b] = keyV;
			}

			// Totals over all texels (constant across partitions): sum of points and sum of |point|^2.
			float3 sumAll = float3(0, 0, 0);
			float pSq = 0.0f;
			[unroll]
			for (uint i = 0; i < 16; ++i)
			{
				sumAll += sortedPts[i];
				pSq += dot(sortedPts[i], sortedPts[i]);
			}

			// Enumerate ordered partitions (n0,n1,n2,n3) of the 16 sorted points across the four palette levels.
			// acc0/acc1/acc2 carry the running point sums of groups 0/1/2 as the split points sweep; group 3 is the
			// remainder. Each (n0,n1,n2) is one closed-form least-squares endpoint solve plus an error evaluation. The
			// loops are runtime (not [unroll]) so the ~969-iteration search stays a small loop body on the AMD compiler.
			float bestSSE = 1e30f;
			float3 acc0 = float3(0, 0, 0);
			for (uint n0 = 0; n0 <= 16; ++n0)
			{
				float3 acc1 = float3(0, 0, 0);
				for (uint n1 = 0; n0 + n1 <= 16; ++n1)
				{
					float3 acc2 = float3(0, 0, 0);
					for (uint n2 = 0; n0 + n1 + n2 <= 16; ++n2)
					{
						uint n3 = 16u - n0 - n1 - n2;
						float3 s0 = acc0;
						float3 s1 = acc1;
						float3 s2 = acc2;
						float3 s3 = sumAll - acc0 - acc1 - acc2;
						float fn0 = (float)n0, fn1 = (float)n1, fn2 = (float)n2, fn3 = (float)n3;

						// Normal-equation coefficients for the blend factors (1-w) = {1, 2/3, 1/3, 0} per level.
						float A = fn0 + fn1 * (4.0f / 9.0f) + fn2 * (1.0f / 9.0f);
						float Cc = fn1 * (1.0f / 9.0f) + fn2 * (4.0f / 9.0f) + fn3;
						float Bb = (fn1 + fn2) * (2.0f / 9.0f);
						float det = A * Cc - Bb * Bb;
						if (det > 1e-9f)
						{
							float3 rhsA = s0 + s1 * (2.0f / 3.0f) + s2 * (1.0f / 3.0f);
							float3 rhsB = s1 * (1.0f / 3.0f) + s2 * (2.0f / 3.0f) + s3;
							float inv = 1.0f / det;
							float3 e0 = (Cc * rhsA - Bb * rhsB) * inv;
							float3 e1 = (A * rhsB - Bb * rhsA) * inv;

							// Reconstructed palette value per group, and the resulting total squared error.
							float3 r0 = e0;
							float3 r1 = (2.0f / 3.0f) * e0 + (1.0f / 3.0f) * e1;
							float3 r2 = (1.0f / 3.0f) * e0 + (2.0f / 3.0f) * e1;
							float3 r3 = e1;
							float sse = pSq
								- 2.0f * (dot(r0, s0) + dot(r1, s1) + dot(r2, s2) + dot(r3, s3))
								+ fn0 * dot(r0, r0) + fn1 * dot(r1, r1) + fn2 * dot(r2, r2) + fn3 * dot(r3, r3);
							if (sse < bestSSE)
							{
								bestSSE = sse;
								bestE0 = saturate(e0);
								bestE1 = saturate(e1);
							}
						}

						if (n0 + n1 + n2 < 16u)
							acc2 += sortedPts[n0 + n1 + n2];
					}
					if (n0 + n1 < 16u)
						acc1 += sortedPts[n0 + n1];
				}
				if (n0 < 16u)
					acc0 += sortedPts[n0];
			}

			float3 maxColor = bestE0;
			float3 minColor = bestE1;

			// Maps each 2-bit index to its endpoint1 (minColor) blend factor.
			float idxW[4];
			idxW[0] = 0.0f; idxW[1] = 1.0f; idxW[2] = 1.0f / 3.0f; idxW[3] = 2.0f / 3.0f;

			// Quantize the cluster-fit endpoints to 5:6:5 and assign indices once, seeding the ±1 polish below.
			uint c0 = PackColor565(maxColor);
			uint c1 = PackColor565(minColor);
			uint indices = 0;
			{
				float3 q0 = Unpack565(c0);
				float3 q1 = Unpack565(c1);
				float3 palette[4];
				palette[0] = q0;
				palette[1] = q1;
				palette[2] = (2.0f * q0 + q1) / 3.0f;
				palette[3] = (q0 + 2.0f * q1) / 3.0f;
				[unroll]
				for (uint j = 0; j < 16; ++j)
				{
					uint best = 0;
					float bestDist = 1e20f;
					[unroll]
					for (uint p = 0; p < 4; ++p)
					{
						float3 d = texels[j] - palette[p];
						float dist = dot(d, d);
						if (dist < bestDist) { bestDist = dist; best = p; }
					}
					indices |= best << (j * 2);
				}
			}

			// +/-1 endpoint polish: with indices fixed, nudge each 5:6:5 sub-channel code by -1/0/+1 and keep the lowest
			// squared-error pair (separable per channel), then re-assign indices. Recovers error the float least-squares
			// fit loses to 5:6:5 quantization. Reconstruction matches Unpack565 (bit replication).
			[unroll]
			for (uint polishIter = 0; polishIter < BCN_POLISH_ITERS; ++polishIter)
			{
				int codes0[3] = { (int)((c0 >> 11) & 0x1Fu), (int)((c0 >> 5) & 0x3Fu), (int)(c0 & 0x1Fu) };
				int codes1[3] = { (int)((c1 >> 11) & 0x1Fu), (int)((c1 >> 5) & 0x3Fu), (int)(c1 & 0x1Fu) };
				int maxCode[3] = { 31, 63, 31 };

				uint idx[16];
				[unroll]
				for (uint j = 0; j < 16; ++j)
					idx[j] = (indices >> (j * 2)) & 3u;

				[unroll]
				for (uint ch = 0; ch < 3; ++ch)
				{
					int cur0 = codes0[ch], cur1 = codes1[ch];
					int mc = maxCode[ch];
					float polBest = 1e30f;
					int sel0 = cur0, sel1 = cur1;
					[unroll]
					for (int d0 = -1; d0 <= 1; ++d0)
					{
						int nc0 = clamp(cur0 + d0, 0, mc);
						// 5-bit channels (R,B) replicate the top 2 bits; the 6-bit channel (G) replicates the top 4.
						float q0 = (float)((ch == 1u) ? ((nc0 << 2) | (nc0 >> 4)) : ((nc0 << 3) | (nc0 >> 2))) / 255.0f;
						[unroll]
						for (int d1 = -1; d1 <= 1; ++d1)
						{
							int nc1 = clamp(cur1 + d1, 0, mc);
							float q1 = (float)((ch == 1u) ? ((nc1 << 2) | (nc1 >> 4)) : ((nc1 << 3) | (nc1 >> 2))) / 255.0f;
							float se = 0;
							[unroll]
							for (uint t = 0; t < 16; ++t)
							{
								float w = idxW[idx[t]];
								float pal = (1.0f - w) * q0 + w * q1;
								float diff = texels[t][ch] - pal;
								se += diff * diff;
							}
							if (se < polBest) { polBest = se; sel0 = nc0; sel1 = nc1; }
						}
					}
					codes0[ch] = sel0; codes1[ch] = sel1;
				}

				c0 = ((uint)codes0[0] << 11) | ((uint)codes0[1] << 5) | (uint)codes0[2];
				c1 = ((uint)codes1[0] << 11) | ((uint)codes1[1] << 5) | (uint)codes1[2];

				// Re-assign indices against the polished endpoints.
				float3 q0v = Unpack565(c0);
				float3 q1v = Unpack565(c1);
				float3 palette[4];
				palette[0] = q0v;
				palette[1] = q1v;
				palette[2] = (2.0f * q0v + q1v) / 3.0f;
				palette[3] = (q0v + 2.0f * q1v) / 3.0f;
				indices = 0;
				[unroll]
				for (uint j = 0; j < 16; ++j)
				{
					uint best = 0;
					float bestDist = 1e20f;
					[unroll]
					for (uint p = 0; p < 4; ++p)
					{
						float3 d = texels[j] - palette[p];
						float dist = dot(d, d);
						if (dist < bestDist) { bestDist = dist; best = p; }
					}
					indices |= best << (j * 2);
				}
			}

			// 4-colour (opaque) mode requires c0 > c1. Swapping endpoints inverts the
			// palette (0<->1, 2<->3), so flip the low bit of every 2-bit index to match.
			if (c0 < c1)
			{
				uint tmp = c0; c0 = c1; c1 = tmp;
				indices ^= 0x55555555u;
			}

			return uint2(c0 | (c1 << 16), indices);
		}

		// Encodes a 4x4 single-channel block into a 64-bit BC4 block using the
		// 8-value interpolated mode (r0 > r1). Endpoints start at the value extent and are
		// least-squares-refined over the assigned indices.
		uint2 CompressBC4(float texels[16])
		{
			float minV = texels[0];
			float maxV = texels[0];
			[unroll]
			for (uint i = 1; i < 16; ++i)
			{
				minV = min(minV, texels[i]);
				maxV = max(maxV, texels[i]);
			}

			// Maps each 3-bit index to its r1 (minV) blend factor.
			float idxW[8];
			idxW[0] = 0.0f; idxW[1] = 1.0f;
			idxW[2] = 1.0f / 7.0f; idxW[3] = 2.0f / 7.0f; idxW[4] = 3.0f / 7.0f;
			idxW[5] = 4.0f / 7.0f; idxW[6] = 5.0f / 7.0f; idxW[7] = 6.0f / 7.0f;

			uint r0 = 0, r1 = 0;
			uint idxLo = 0;
			uint idxHi = 0;

			// Pass 0: assign indices then least-squares-refine the endpoints.
			// Pass 1: re-assign indices against the refined endpoints and emit.
			// Runtime loop (not [unroll]), because we're seeing hangs when compiling this shader on AMD
			for (uint refinePass = 0; refinePass < BCN_REFINE_PASSES; ++refinePass)
			{
				r0 = (uint)round(saturate(maxV) * 255.0f);
				r1 = (uint)round(saturate(minV) * 255.0f);
				float fr0 = r0 / 255.0f;
				float fr1 = r1 / 255.0f;

				float palette[8];
				palette[0] = fr0;
				palette[1] = fr1;
				[unroll]
				for (uint p = 1; p < 7; ++p)
					palette[p + 1] = ((7 - p) * fr0 + p * fr1) / 7.0f;

				uint idx[16];
				idxLo = 0;
				idxHi = 0;
				[unroll]
				for (uint j = 0; j < 16; ++j)
				{
					float v = texels[j];
					uint best = 0;
					float bestDist = 1e20f;
					[unroll]
					for (uint k = 0; k < 8; ++k)
					{
						float d = v - palette[k];
						float dist = d * d;
						if (dist < bestDist)
						{
							bestDist = dist;
							best = k;
						}
					}
					idx[j] = best;

					uint pos = j * 3;
					if (pos < 32)
					{
						idxLo |= best << pos;
						if (pos + 3 > 32)
							idxHi |= best >> (32 - pos);
					}
					else
						idxHi |= best << (pos - 32);
				}

				if (refinePass + 1 < BCN_REFINE_PASSES)
				{
					// Solve the 2x2 normal equations for the endpoints that minimise error
					// given the fixed indices.
					float A = 0, B = 0, C = 0, sumA = 0, sumB = 0;
					[unroll]
					for (uint j = 0; j < 16; ++j)
					{
						float w = idxW[idx[j]];
						float a = 1.0f - w;
						A += a * a; B += a * w; C += w * w;
						sumA += a * texels[j];
						sumB += w * texels[j];
					}
					float det = A * C - B * B;
					if (abs(det) > 1e-7f)
					{
						float invDet = 1.0f / det;
						float e0 = ( C * sumA - B * sumB) * invDet;
						float e1 = (-B * sumA + A * sumB) * invDet;
						// Keep r0 >= r1 so pass 1 stays in 8-value mode; indices are
						// reassigned fresh against the ordered palette, so no remap needed.
						maxV = saturate(max(e0, e1));
						minV = saturate(min(e0, e1));
					}
				}
			}

			// Joint endpoint window search (replaces the index-fixed +/-1 polish). For every (r0,r1) candidate in a
			// window around the Lloyd-refined endpoints, do a *fresh* nearest-level assignment and evaluate the true
			// squared error. Because each candidate re-derives its own index map, this explores different assignments
			// globally and escapes the local minimum Lloyd converges to - which is the bulk of the quality gap to CPU
			// encoders. For evenly-spaced levels the nearest index is closed form (project onto the ramp + round), so
			// no inner 8-way palette match is needed. The o0/o1 loops are runtime (not [unroll]) to keep the IR small.
			{
				const int kW = 8; // +/- search radius in 8-bit endpoint codes
				int baseR0 = (int)r0, baseR1 = (int)r1;
				int bestR0 = baseR0, bestR1 = baseR1;
				float bestErr = 1e30f;
				[loop]
				for (int o0 = -kW; o0 <= kW; ++o0)
				{
					int nc0 = clamp(baseR0 + o0, 0, 255);
					float q0 = nc0 / 255.0f;
					[loop]
					for (int o1 = -kW; o1 <= kW; ++o1)
					{
						int nc1 = clamp(baseR1 + o1, 0, 255);
						float q1 = nc1 / 255.0f;
						float diff = q1 - q0;
						float invd = (abs(diff) > 1e-6f) ? (7.0f / diff) : 0.0f;
						float se = 0;
						[unroll]
						for (uint t = 0; t < 16; ++t)
						{
							float v = texels[t];
							float kf = clamp(round((v - q0) * invd), 0.0f, 7.0f);
							float recon = q0 + kf * diff / 7.0f;
							float e = v - recon;
							se += e * e;
						}
						if (se < bestErr) { bestErr = se; bestR0 = nc0; bestR1 = nc1; }
					}
				}
				r0 = (uint)bestR0; r1 = (uint)bestR1;
			}

			// 8-value mode requires r0 >= r1; the palette set is identical under a swap, so reorder and let the final
			// re-assignment below pick fresh indices (no remap needed).
			if (r0 < r1) { uint tmp = r0; r0 = r1; r1 = tmp; }

			// Final index assignment + pack against the polished, ordered endpoints.
			{
				float fr0 = r0 / 255.0f;
				float fr1 = r1 / 255.0f;
				float pal[8];
				pal[0] = fr0; pal[1] = fr1;
				[unroll]
				for (uint p = 1; p < 7; ++p)
					pal[p + 1] = ((7 - p) * fr0 + p * fr1) / 7.0f;

				idxLo = 0;
				idxHi = 0;
				[unroll]
				for (uint j = 0; j < 16; ++j)
				{
					uint best = 0;
					float bestDist = 1e20f;
					[unroll]
					for (uint k = 0; k < 8; ++k)
					{
						float d = texels[j] - pal[k];
						float dist = d * d;
						if (dist < bestDist) { bestDist = dist; best = k; }
					}
					uint pos = j * 3;
					if (pos < 32)
					{
						idxLo |= best << pos;
						if (pos + 3 > 32)
							idxHi |= best >> (32 - pos);
					}
					else
						idxHi |= best << (pos - 32);
				}
			}

			// Block layout (little-endian): byte0 = r0, byte1 = r1, then 48 index bits.
			uint lo = r0 | (r1 << 8) | (idxLo << 16);
			uint hi = (idxLo >> 16) | (idxHi << 16);
			return uint2(lo, hi);
		}

		#if IS_BC6H_TWOREGION
		[numthreads(32, 1, 1)] // 32 threads = the 32 BC6H two-region partitions; one thread-group per 4x4 block
		#else
		[numthreads(8, 8, 1)]
		#endif
		void csmain(uint3 dispatchId : SV_DispatchThreadID, uint3 groupId : SV_GroupID, uint3 groupThreadId : SV_GroupThreadID)
		{
			int2 texSize = gMeta[0];
			uint2 numBlocks = (uint2)gMeta[1];

			#if IS_BC6H_TWOREGION
			uint2 blockId = groupId.xy;    // one thread-group per 4x4 block
			uint partId = groupThreadId.x; // 0..31: this thread's partition
			#else
			uint2 blockId = dispatchId.xy;
			#endif
			if (blockId.x >= numBlocks.x || blockId.y >= numBlocks.y)
				return;

			uint2 base = blockId * 4;
			uint2 maxCoord = (uint2)texSize - 1;

			#if IS_BC7
				float4 rgba[16];
				float3 rgbF[16];
			#elif IS_BC6H
				int3 hbits[16]; // RGB as UF16 half-float bits
			#else
				float3 rgb[16];
				float red[16];
				float green[16];
				float alpha[16];
			#endif

			[unroll]
			for (uint y = 0; y < 4; ++y)
			{
				[unroll]
				for (uint x = 0; x < 4; ++x)
				{
					uint2 coord = min(base + uint2(x, y), maxCoord);

					float4 texel = gInput[coord.y * (uint)texSize.x + coord.x];

					uint idx = y * 4 + x;
					#if IS_BC7
						rgba[idx] = texel;
						rgbF[idx] = texel.rgb;
					#elif IS_BC6H
						// UF16: clamp negatives, convert each channel to its half-float bit pattern, clamp to F16 max.
						// f32tof16 is used per-component (the cross-compiler supports only the scalar form).
						uint hbR = f32tof16(max(texel.r, 0.0f));
						uint hbG = f32tof16(max(texel.g, 0.0f));
						uint hbB = f32tof16(max(texel.b, 0.0f));
						hbits[idx] = int3((int)min(hbR, 0x7BFFu), (int)min(hbG, 0x7BFFu), (int)min(hbB, 0x7BFFu));
					#else
						rgb[idx] = texel.rgb;
						red[idx] = texel.r;
						green[idx] = texel.g;
						alpha[idx] = texel.a;
					#endif
				}
			}

			uint blockIndex = blockId.y * numBlocks.x + blockId.x;

			#if FORMAT == 0 // BC1
				gOutput[blockIndex] = CompressBC1(rgb);
			#elif FORMAT == 1 // BC3 = BC4(alpha) + BC1(color)
				uint2 alphaBlock = CompressBC4(alpha);
				uint2 colorBlock = CompressBC1(rgb);
				gOutput[blockIndex] = uint4(alphaBlock, colorBlock);
			#elif FORMAT == 2 // BC4
				gOutput[blockIndex] = CompressBC4(red);
			#elif FORMAT == 3 // BC5 = BC4(red) + BC4(green)
				uint2 redBlock = CompressBC4(red);
				uint2 greenBlock = CompressBC4(green);
				gOutput[blockIndex] = uint4(redBlock, greenBlock);
			#elif IS_BC6H // BC6H (UF16, HDR): one encoder mode per kernel (FORMAT 4..17 -> modes 1..14); accumulate via gBestErr.
				float modeErr;
				uint4 modeBlock;
				#if FORMAT == 4
					modeBlock = CompressBC6Mode1(hbits, partId, modeErr);
				#elif FORMAT == 5
					modeBlock = CompressBC6Mode2(hbits, partId, modeErr);
				#elif FORMAT == 6
					modeBlock = CompressBC6Mode3(hbits, partId, modeErr);
				#elif FORMAT == 7
					modeBlock = CompressBC6Mode4(hbits, partId, modeErr);
				#elif FORMAT == 8
					modeBlock = CompressBC6Mode5(hbits, partId, modeErr);
				#elif FORMAT == 9
					modeBlock = CompressBC6Mode6(hbits, partId, modeErr);
				#elif FORMAT == 10
					modeBlock = CompressBC6Mode7(hbits, partId, modeErr);
				#elif FORMAT == 11
					modeBlock = CompressBC6Mode8(hbits, partId, modeErr);
				#elif FORMAT == 12
					modeBlock = CompressBC6Mode9(hbits, partId, modeErr);
				#elif FORMAT == 13
					modeBlock = CompressBC6Mode10(hbits, partId, modeErr);
				#elif FORMAT == 14
					modeBlock = CompressBC6Mode11(hbits, modeErr);
				#elif FORMAT == 15
					modeBlock = CompressBC6Mode12(hbits, modeErr);
				#elif FORMAT == 16
					modeBlock = CompressBC6Mode13(hbits, modeErr);
				#else // FORMAT == 17
					modeBlock = CompressBC6Mode14(hbits, modeErr);
				#endif

				// Keep this mode's block if it beats the running best. gBestErr is seeded to +inf by the host, so the first
				// dispatched mode always wins; no special seed pass is needed and dispatch order is irrelevant.
				#if IS_BC6H_TWOREGION
				if (partId == 0) // group leader commits the running-best; all 32 threads computed the same block
				#endif
				{
					float prevErr6 = gBestErr[blockIndex];
					uint4 prevBlock6 = gOutput[blockIndex];
					if (modeErr < prevErr6) { prevErr6 = modeErr; prevBlock6 = modeBlock; }
					gOutput[blockIndex] = prevBlock6;
					gBestErr[blockIndex] = prevErr6;
				}
			#else // BC7 (FORMAT 18..25 -> encoder modes 0..7): one mode per kernel; accumulate via gBestErr.
				float modeErr;
				uint4 modeBlock;
				#if FORMAT >= 18 && FORMAT <= 21
					// RGB-only modes (0, 1, 2, 3) cannot represent alpha (the decoder forces A=255), so charge them the
					// alpha they drop before comparing against the alpha-capable modes (4/5/6/7).
					float alphaPenalty = 0;
					[unroll]
					for (uint i = 0; i < 16; ++i)
					{
						float da = saturate(rgba[i].a) * 255.0f - 255.0f;
						alphaPenalty += da * da;
					}
				#endif

				#if FORMAT == 18
					modeBlock = CompressBC7Mode0(rgbF, modeErr); modeErr += alphaPenalty; // 3-subset RGB
				#elif FORMAT == 19
					modeBlock = CompressBC7Mode1(rgbF, modeErr); modeErr += alphaPenalty; // 2-subset RGB
				#elif FORMAT == 20
					modeBlock = CompressBC7Mode2(rgbF, modeErr); modeErr += alphaPenalty; // 3-subset RGB
				#elif FORMAT == 21
					modeBlock = CompressBC7Mode3(rgbF, modeErr); modeErr += alphaPenalty; // 2-subset RGB
				#elif FORMAT == 22
					modeBlock = CompressBC7Mode4(rgba, modeErr); // RGBA single-subset, rotation + dual index
				#elif FORMAT == 23
					modeBlock = CompressBC7Mode5(rgba, modeErr); // RGBA single-subset, rotation + dual index
				#elif FORMAT == 24
					modeBlock = CompressBC7Mode6(rgba, modeErr); // RGBA single-subset
				#else // FORMAT == 25
					modeBlock = CompressBC7Mode7(rgba, modeErr); // RGBA two-subset
				#endif

				// Keep this mode's block if it beats the running best (gBestErr seeded to +inf by the host; see BC6H note).
				float prevErr = gBestErr[blockIndex];
				uint4 prevBlock = gOutput[blockIndex];
				if (modeErr < prevErr) { prevErr = modeErr; prevBlock = modeBlock; }
				gOutput[blockIndex] = prevBlock;
				gBestErr[blockIndex] = prevErr;
			#endif
		}
	};
};

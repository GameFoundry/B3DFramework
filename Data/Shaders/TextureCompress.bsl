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
		// Target block-compressed format.
		// 0  - BC1 (RGB, 64-bit block)
		// 1  - BC4 (single red channel, 64-bit block)
		// 2  - BC3 (RGBA: BC4 alpha block + BC1 color block, 128-bit)
		// 3  - BC5 (RG: two BC4 blocks, 128-bit)
		// 4  - BC7  (RGBA, 128-bit)        mode group A: modes 6/5/4/7/1 - seeds the running best
		// 5  - BC6H (RGB HDR / UF16, 128-bit) mode group A: one-region modes 11/12/13/14 - seeds the running best
		// 6  - BC7  mode group B: mode 3      - continues the running best
		// 7  - BC7  mode group C: mode 2      - continues the running best
		// 8  - BC7  mode group D: mode 0      - continues the running best (final BC7 group)
		// 9  - BC6H mode group B: modes 1/2   - continues the running best
		// 10 - BC6H mode group C: modes 3/4   - continues the running best
		// 11 - BC6H mode group D: modes 5/6   - continues the running best
		// 12 - BC6H mode group E: modes 7/8   - continues the running best
		// 13 - BC6H mode group F: modes 9/10  - continues the running best (final BC6H group)
		//
		// Both BC7 and BC6H split their modes across several dispatches because inlining every mode into one compute
		// kernel produces SPIR-V large enough to hang AMD's Vulkan driver (amdvlk / LLPC) inside vkCreateComputePipelines.
		// The per-kernel budget is small, so the heaviest modes are isolated into their own (or a small) group:
		//   - BC7  (FORMAT 4 -> 6 -> 7 -> 8): ~two two-subset modes plus the single-subset modes compile together, but a
		//     three-subset mode (0 or 2) must sit alone - even pairing it with one other multi-subset mode overruns.
		//   - BC6H (FORMAT 5 -> 9 -> 10 -> 11 -> 12 -> 13): the one-region modes (11-14, no partition search) are light and
		//     all fit in the seed group, while each two-region mode (1-10) runs a 32-partition search and is heavy, so they
		//     are paired two-per-group.
		// Within each format the dispatches share the gOutput block buffer and a gBestErr buffer that carries the per-block
		// best error from one group to the next; the C++ orchestration (B3DTextureCompressor.cpp) runs the groups in
		// sequence over the same buffers.
		FORMAT = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13 };
	};

	code
	{
		// 64-bit formats output a uint2 per block, 128-bit formats output a uint4.
		#if FORMAT == 0 || FORMAT == 1
			#define BLOCK_TYPE uint2
		#else
			#define BLOCK_TYPE uint4
		#endif

		// BC7 is split across four dispatches (FORMAT 4/6/7/8); they share input/output layout and a running-best buffer.
		#define IS_BC7 (FORMAT == 4 || FORMAT == 6 || FORMAT == 7 || FORMAT == 8)
		// BC6H is split across six dispatches (FORMAT 5/9/10/11/12/13), same running-best scheme as BC7.
		#define IS_BC6H (FORMAT == 5 || FORMAT == 9 || FORMAT == 10 || FORMAT == 11 || FORMAT == 12 || FORMAT == 13)

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

			// Endpoints = the extreme projections onto the principal axis.
			float minProj = 1e20f;
			float maxProj = -1e20f;
			[unroll]
			for (uint i = 0; i < 16; ++i)
			{
				float proj = dot(texels[i] - mean, axis);
				minProj = min(minProj, proj);
				maxProj = max(maxProj, proj);
			}
			float3 maxColor = saturate(mean + axis * maxProj);
			float3 minColor = saturate(mean + axis * minProj);

			// Maps each 2-bit index to its endpoint1 (minColor) blend factor.
			float idxW[4];
			idxW[0] = 0.0f; idxW[1] = 1.0f; idxW[2] = 1.0f / 3.0f; idxW[3] = 2.0f / 3.0f;

			uint c0 = 0, c1 = 0;
			uint indices = 0;

			// Pass 0: assign indices then least-squares-refine the endpoints.
			// Pass 1: re-assign indices against the refined endpoints and emit.
			// Runtime loop (not [unroll]), because we're seeing hangs when compiling this shader on AMD
			for (uint refinePass = 0; refinePass < BCN_REFINE_PASSES; ++refinePass)
			{
				c0 = PackColor565(maxColor);
				c1 = PackColor565(minColor);
				float3 q0 = Unpack565(c0);
				float3 q1 = Unpack565(c1);

				float3 palette[4];
				palette[0] = q0;
				palette[1] = q1;
				palette[2] = (2.0f * q0 + q1) / 3.0f;
				palette[3] = (q0 + 2.0f * q1) / 3.0f;

				uint idx[16];
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
						if (dist < bestDist)
						{
							bestDist = dist;
							best = p;
						}
					}
					idx[j] = best;
					indices |= best << (j * 2);
				}

				if (refinePass + 1 < BCN_REFINE_PASSES)
				{
					// Solve the 2x2 normal equations for the endpoints that minimise error
					// given the fixed indices. Same matrix for all three channels.
					float A = 0, B = 0, C = 0;
					float3 sumA = float3(0, 0, 0);
					float3 sumB = float3(0, 0, 0);
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
						maxColor = saturate(( C * sumA - B * sumB) * invDet);
						minColor = saturate((-B * sumA + A * sumB) * invDet);
					}
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

			// +/-1 endpoint polish: with indices fixed, nudge each 8-bit endpoint by -1/0/+1 and keep the lowest
			// squared-error pair, then re-assign indices. Recovers error the float least-squares fit loses to rounding.
			[unroll]
			for (uint polishIter = 0; polishIter < BCN_POLISH_ITERS; ++polishIter)
			{
				// Index assignment against the current endpoints (8-value ramp, r0 >= r1).
				float fr0 = r0 / 255.0f;
				float fr1 = r1 / 255.0f;
				float pal[8];
				pal[0] = fr0; pal[1] = fr1;
				[unroll]
				for (uint p = 1; p < 7; ++p)
					pal[p + 1] = ((7 - p) * fr0 + p * fr1) / 7.0f;

				uint idx[16];
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
					idx[j] = best;
				}

				// Optimize the two endpoints (single channel) with indices held fixed.
				int cur0 = (int)r0, cur1 = (int)r1;
				float polBest = 1e30f;
				int sel0 = cur0, sel1 = cur1;
				[unroll]
				for (int d0 = -1; d0 <= 1; ++d0)
				{
					int nc0 = clamp(cur0 + d0, 0, 255);
					float q0 = nc0 / 255.0f;
					[unroll]
					for (int d1 = -1; d1 <= 1; ++d1)
					{
						int nc1 = clamp(cur1 + d1, 0, 255);
						float q1 = nc1 / 255.0f;
						float se = 0;
						[unroll]
						for (uint t = 0; t < 16; ++t)
						{
							float w = idxW[idx[t]];
							float p = (1.0f - w) * q0 + w * q1;
							float diff = texels[t] - p;
							se += diff * diff;
						}
						if (se < polBest) { polBest = se; sel0 = nc0; sel1 = nc1; }
					}
				}
				r0 = (uint)sel0; r1 = (uint)sel1;
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

		[numthreads(8, 8, 1)]
		void csmain(uint3 dispatchId : SV_DispatchThreadID)
		{
			int2 texSize = gMeta[0];
			uint2 numBlocks = (uint2)gMeta[1];

			uint2 blockId = dispatchId.xy;
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
			#elif FORMAT == 1 // BC4
				gOutput[blockIndex] = CompressBC4(red);
			#elif FORMAT == 2 // BC3 = BC4(alpha) + BC1(color)
				uint2 alphaBlock = CompressBC4(alpha);
				uint2 colorBlock = CompressBC1(rgb);
				gOutput[blockIndex] = uint4(alphaBlock, colorBlock);
			#elif FORMAT == 3 // BC5 = BC4(red) + BC4(green)
				uint2 redBlock = CompressBC4(red);
				uint2 greenBlock = CompressBC4(green);
				gOutput[blockIndex] = uint4(redBlock, greenBlock);
			#elif IS_BC6H // BC6H (UF16, HDR): split into mode groups; results accumulate via gBestErr (see FORMAT note).
				float bestErr6;
				uint4 bestBlock6;
				#if FORMAT == 5
					// Seed group: one-region modes 11-14. They have no partition search, so all four fit in one kernel.
					bestBlock6 = CompressBC6Mode11(hbits, bestErr6);

					float err12;
					uint4 block12 = CompressBC6Mode12(hbits, err12);
					if (err12 < bestErr6) { bestErr6 = err12; bestBlock6 = block12; }

					float err13;
					uint4 block13 = CompressBC6Mode13(hbits, err13);
					if (err13 < bestErr6) { bestErr6 = err13; bestBlock6 = block13; }

					float err14;
					uint4 block14 = CompressBC6Mode14(hbits, err14);
					if (err14 < bestErr6) { bestErr6 = err14; bestBlock6 = block14; }
				#else
					// Two-region groups: continue from the running best. Each two-region mode does a 32-partition search and
					// is "heavy", so they are paired two-per-group to stay within the amdvlk compile budget.
					bestErr6 = gBestErr[blockIndex];
					bestBlock6 = gOutput[blockIndex];

					#if FORMAT == 9
						float err1;
						uint4 block1 = CompressBC6Mode1(hbits, err1);
						if (err1 < bestErr6) { bestErr6 = err1; bestBlock6 = block1; }

						float err2;
						uint4 block2 = CompressBC6Mode2(hbits, err2);
						if (err2 < bestErr6) { bestErr6 = err2; bestBlock6 = block2; }
					#elif FORMAT == 10
						float err3;
						uint4 block3 = CompressBC6Mode3(hbits, err3);
						if (err3 < bestErr6) { bestErr6 = err3; bestBlock6 = block3; }

						float err4;
						uint4 block4 = CompressBC6Mode4(hbits, err4);
						if (err4 < bestErr6) { bestErr6 = err4; bestBlock6 = block4; }
					#elif FORMAT == 11
						float err5;
						uint4 block5 = CompressBC6Mode5(hbits, err5);
						if (err5 < bestErr6) { bestErr6 = err5; bestBlock6 = block5; }

						float err6;
						uint4 block6 = CompressBC6Mode6(hbits, err6);
						if (err6 < bestErr6) { bestErr6 = err6; bestBlock6 = block6; }
					#elif FORMAT == 12
						float err7;
						uint4 block7 = CompressBC6Mode7(hbits, err7);
						if (err7 < bestErr6) { bestErr6 = err7; bestBlock6 = block7; }

						float err8;
						uint4 block8 = CompressBC6Mode8(hbits, err8);
						if (err8 < bestErr6) { bestErr6 = err8; bestBlock6 = block8; }
					#else // FORMAT == 13
						float err9;
						uint4 block9 = CompressBC6Mode9(hbits, err9);
						if (err9 < bestErr6) { bestErr6 = err9; bestBlock6 = block9; }

						float err10;
						uint4 block10 = CompressBC6Mode10(hbits, err10);
						if (err10 < bestErr6) { bestErr6 = err10; bestBlock6 = block10; }
					#endif
				#endif

				gOutput[blockIndex] = bestBlock6;
				#if FORMAT != 13
					// Publish the running best for the next group's dispatch (the final group, FORMAT 13, skips this).
					gBestErr[blockIndex] = bestErr6;
				#endif
			#else // BC7 (FORMAT 4/6/7): each FORMAT value evaluates one mode group; results accumulate via gBestErr.
				// RGB-only modes (0, 1, 2, 3) cannot represent alpha (the decoder forces A=255), so charge them the alpha
				// they drop before comparing against alpha-capable modes (4/5/6/7).
				float alphaPenalty = 0;
				[unroll]
				for (uint i = 0; i < 16; ++i)
				{
					float da = saturate(rgba[i].a) * 255.0f - 255.0f;
					alphaPenalty += da * da;
				}

				float bestErr;
				uint4 bestBlock;
				#if FORMAT == 4
					// Group A (first dispatch): single-subset modes 6/5/4 plus two-subset modes 7/1. Seeds the running best.
					bestBlock = CompressBC7Mode6(rgba, bestErr);

					// Modes 5 and 4 encode full RGBA, so they compete without the alpha penalty.
					float err5;
					uint4 block5 = CompressBC7Mode5(rgba, err5);
					if (err5 < bestErr) { bestErr = err5; bestBlock = block5; }

					float err4;
					uint4 block4 = CompressBC7Mode4(rgba, err4);
					if (err4 < bestErr) { bestErr = err4; bestBlock = block4; }

					float err7;
					uint4 block7 = CompressBC7Mode7(rgba, err7);
					if (err7 < bestErr) { bestErr = err7; bestBlock = block7; }

					float err1;
					uint4 block1 = CompressBC7Mode1(rgbF, err1);
					err1 += alphaPenalty;
					if (err1 < bestErr) { bestErr = err1; bestBlock = block1; }
				#else
					// Groups B/C/D: continue from the previous group's per-block running best.
					bestErr = gBestErr[blockIndex];
					bestBlock = gOutput[blockIndex];

					#if FORMAT == 6
						// Group B: two-subset mode 3 (RGB-only).
						float err3;
						uint4 block3 = CompressBC7Mode3(rgbF, err3);
						err3 += alphaPenalty;
						if (err3 < bestErr) { bestErr = err3; bestBlock = block3; }
					#elif FORMAT == 7
						// Group C: three-subset mode 2 (RGB-only). Three-subset modes sit alone (see FORMAT note).
						float err2;
						uint4 block2 = CompressBC7Mode2(rgbF, err2);
						err2 += alphaPenalty;
						if (err2 < bestErr) { bestErr = err2; bestBlock = block2; }
					#else // FORMAT == 8
						// Group D: three-subset mode 0 (RGB-only). Final group.
						float err0;
						uint4 block0 = CompressBC7Mode0(rgbF, err0);
						err0 += alphaPenalty;
						if (err0 < bestErr) { bestErr = err0; bestBlock = block0; }
					#endif
				#endif

				gOutput[blockIndex] = bestBlock;
				#if FORMAT != 8
					// Publish the running best for the next group's dispatch (the final group, FORMAT 8, skips this).
					gBestErr[blockIndex] = bestErr;
				#endif
			#endif
		}
	};
};

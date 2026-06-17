// BC6H / BC7 mode encoders live in their own headers (one mode per file where practical) to keep this dispatcher small;
// see Includes/TextureCompression/
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
#include "$ENGINE$\TextureCompression\TextureCompressBC1.bslinc"
#include "$ENGINE$\TextureCompression\TextureCompressBC4.bslinc"

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
	mixin TextureCompressBC1;
	mixin TextureCompressBC4;

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

		// Source pixels as a 2D texture: RGBA8 (read as normalized float4) for LDR, or RGBA32F for HDR (BC6H).
		Texture2D gInput;

		// One packed block per texel, at the block-grid coordinate: a uint2 (64-bit formats) or uint4 (128-bit).
		RWTexture2D<BLOCK_TYPE> gOutput;

		#if IS_BC7 || IS_BC6H
			// Per-block lowest error so far, carried between the BC7/BC6H mode-group dispatches; addressed by block
			// coordinate. The host seeds it to +inf so the first dispatched mode always wins; later modes continue the
			// running minimum and write it back.
			RWTexture2D<float> gBestErr;
		#endif

		[internal]
		cbuffer Parameters
		{
			int2 gTextureSize; // texture size in pixels (whole texture)
			int2 gBlockCount;  // number of 4x4 blocks along each axis (whole texture)
			int2 gBlockOffset; // block-grid offset of the dispatched tile; added to the local block id (both X and Y)
		}

		#if IS_BC6H_TWOREGION
		[numthreads(32, 1, 1)] // 32 threads = the 32 BC6H two-region partitions; one thread-group per 4x4 block
		#else
		[numthreads(8, 8, 1)]
		#endif
		void csmain(uint3 dispatchId : SV_DispatchThreadID, uint3 groupId : SV_GroupID, uint3 groupThreadId : SV_GroupThreadID)
		{
			int2 texSize = gTextureSize;
			uint2 numBlocks = (uint2)gBlockCount;

			#if IS_BC6H_TWOREGION
			uint2 blockId = groupId.xy;    // one thread-group per 4x4 block
			uint partId = groupThreadId.x; // 0..31: this thread's partition
			#else
			uint2 blockId = dispatchId.xy;
			#endif
			#if IS_BC7 || IS_BC6H
			// The per-tile gBestErr scratch is only one tile in size, so it is indexed by the tile-local block id (the
			// dispatch grid starts at (0,0)), captured here before the global offset is applied below.
			uint2 localBlockId = blockId;
			#endif
			// Large textures are dispatched in square tiles: each tile's grid starts at (0,0), so shift it to its real
			// block coordinates. The shared input/output textures are addressed with this global block id.
			blockId += (uint2)gBlockOffset;
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

					float4 texel = gInput.Load(int3((int2)coord, 0));

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

			#if FORMAT == 0 // BC1
				gOutput[blockId] = CompressBC1(rgb, true); // standalone BC1: 3-colour mode allowed
			#elif FORMAT == 1 // BC3 = BC4(alpha) + BC1(color)
				uint2 alphaBlock = CompressBC4(alpha);
				uint2 colorBlock = CompressBC1(rgb, false); // BC3 colour block is always 4-colour
				gOutput[blockId] = uint4(alphaBlock, colorBlock);
			#elif FORMAT == 2 // BC4
				gOutput[blockId] = CompressBC4(red);
			#elif FORMAT == 3 // BC5 = BC4(red) + BC4(green)
				uint2 redBlock = CompressBC4(red);
				uint2 greenBlock = CompressBC4(green);
				gOutput[blockId] = uint4(redBlock, greenBlock);
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
					float prevErr6 = gBestErr[localBlockId];
					uint4 prevBlock6 = gOutput[blockId];
					if (modeErr < prevErr6) { prevErr6 = modeErr; prevBlock6 = modeBlock; }
					gOutput[blockId] = prevBlock6;
					gBestErr[localBlockId] = prevErr6;
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
				float prevErr = gBestErr[localBlockId];
				uint4 prevBlock = gOutput[blockId];
				if (modeErr < prevErr) { prevErr = modeErr; prevBlock = modeBlock; }
				gOutput[blockId] = prevBlock;
				gBestErr[localBlockId] = prevErr;
			#endif
		}
	};
};

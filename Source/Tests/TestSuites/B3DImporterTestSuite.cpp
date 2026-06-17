//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DImporterTestSuite.h"

#include "Importer/B3DImporter.h"
#include "Importer/B3DTextureImportOptions.h"
#include "Audio/B3DAudioClipImportOptions.h"
#include "Image/B3DTexture.h"
#include "Image/B3DPixelUtility.h"
#include "Image/B3DTextureCompressor.h"
#include "Image/B3DGenerateMipmap.h"
#include "Image/B3DPixelData.h"
#include "Image/B3DColor.h"
#include "Audio/B3DAudioClip.h"
#include "Resources/B3DBuiltinResources.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuDeviceCapabilities.h"

// Dependencies of the GPU hardware-decode oracle (DecodeToFloatOnGpu) below.
#include "Renderer/B3DRenderer.h"
#include "Renderer/B3DRendererMaterial.h"
#include "Renderer/B3DGpuUniformBuffer.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuBuffer.h"
#include "GpuBackend/B3DGpuParameterSet.h"
#include "GpuBackend/B3DGpuPipelineState.h"
#include "CoreObject/B3DRenderThread.h"
#include "Threading/B3DAsyncOp.h"

// B3D primitives for test inputs (config variables / command line) and file output (data streams).
#include "Utility/B3DConfigVariable.h"
#include "Utility/B3DCommandLine.h"
#include "Utility/B3DTimer.h"
#include "FileSystem/B3DFileSystem.h"
#include "FileSystem/B3DDataStream.h"
#include "B3DApplication.h"

#include <cmath>
#include <cstring>

using namespace b3d;

namespace b3d::render
{
	namespace
	{
		/** Per-dispatch constants for the GPU block-decode kernel (maps to the TextureDecompress.bsl `cbuffer Parameters`). */
		B3D_UNIFORM_BUFFER_BEGIN(TextureDecompressParameterDefinition)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2I, gSize)  // Decoded output size in pixels
		B3D_UNIFORM_BUFFER_END

		TextureDecompressParameterDefinition gTextureDecompressParameters;
	}
}

namespace
{
	// ---- Test configuration ----

	/** 
	 * Comparison images and raw block/source dumps are written only when this is enabled (the tests are otherwise
	 * purely numeric). Set via the command line: --test.dumpCompressedImages=true.
	 */
	TConfigVariable<bool> gDumpCompressedImages("test.dumpCompressedImages",
		"Write side-by-side [source | decoded] comparison images and raw block dumps to the working directory during the "
		"GPU texture-compression tests.", false, ConfigVariableFlag::ReadOnly);

	// By default the compression tests run on the small built-in image (or a synthesized gradient for BC6H). These
	// command-line parameters point a test at a larger external asset instead. 
	//   --test.ldrImage=<path>   LDR source for TestGpuCompress_Psnr (any importer-loadable image; dims floored to /4).
	//   --test.bc6hImage=<path>  HDR source for TestGpuCompress_BC6H_Psnr (imported as RGBA32F).
	//   --test.perfImage=<path>  LDR source for TestGpuCompress_Perf (also gates whether that test runs at all).
	Path GetImagePathParameter(const String& name)
	{
		const String value = CommandLine::GetParameterValue(name);
		return value.empty() ? Path() : Path(value);
	}

	// ---- CPU reference BCn decoders ----

	// These mirror the hardware BCn decoder bit-for-bit and are used only to measure encoder PSNR. They are the software
	// model the GPU hardware-decoder (below) is cross-checked against.

	// Expands a 565-packed colour to 8-bit per channel by bit replication, matching the hardware BCn decoder.
	void Expand565(u16 c, i32& r, i32& g, i32& b)
	{
		const i32 r5 = (c >> 11) & 0x1F;
		const i32 g6 = (c >> 5) & 0x3F;
		const i32 b5 = c & 0x1F;

		r = (r5 << 3) | (r5 >> 2);
		g = (g6 << 2) | (g6 >> 4);
		b = (b5 << 3) | (b5 >> 2);
	}

	// Reads len bits at absolute bit position start, LSB-first (matching the encoder's bit packing).
	u32 BC6GetBits(const u8* block, u32 start, u32 len)
	{
		u32 v = 0;
		for(u32 i = 0; i < len; ++i)
		{
			const u32 p = start + i;
			v |= (u32)((block[p >> 3] >> (p & 7)) & 1u) << i;
		}
		return v;
	}

	i32 BC6SignExtend(i32 w, i32 bits)
	{
		return (w & (1 << (bits - 1))) ? (w | (~0 << bits)) : w;
	}

	i32 BC6Unquantize(i32 q, i32 prec) // UF16
	{
		if(prec >= 15) return q;
		if(q == 0) return 0;
		if(q == (1 << prec) - 1) return 0xFFFF;
		return ((q << 16) + 0x8000) >> prec;
	}

	i32 BC6Finish(i32 u) { return (u * 31) >> 6; } // UF16 magnitude scale

	i32 BC6Lerp(i32 a, i32 b, i32 i, i32 denom)
	{
		static const i32 w4[16] = { 0, 4, 9, 13, 17, 21, 26, 30, 34, 38, 43, 47, 51, 55, 60, 64 };
		static const i32 w3[8] = { 0, 9, 18, 27, 37, 46, 55, 64 };
		const i32* w = (denom == 15) ? w4 : w3;
		return (a * w[denom - i] + b * w[i]) >> 6;
	}

	// Converts a 16-bit half-float bit pattern to a 32-bit float.
	float BC6HalfToFloat(u16 h)
	{
		const u32 sign = (h >> 15) & 1u;
		const u32 exp = (h >> 10) & 0x1Fu;
		const u32 man = h & 0x3FFu;
		u32 out;
		if(exp == 0)
		{
			if(man == 0)
				out = sign << 31; // +/- zero
			else
			{
				// Subnormal half -> normalized float.
				i32 e = -1;
				u32 m = man;
				do { e++; m <<= 1; } while((m & 0x400u) == 0);
				m &= 0x3FFu;
				out = (sign << 31) | ((u32)(127 - 15 - e) << 23) | (m << 13);
			}
		}
		else if(exp == 0x1F)
			out = (sign << 31) | 0x7F800000u | (man << 13); // inf / nan
		else
			out = (sign << 31) | ((exp - 15 + 127) << 23) | (man << 13);

		float f;
		std::memcpy(&f, &out, sizeof(f));

		return f;
	}

	// Decodes a BC1 colour block (8 bytes) into 16 RGBA quads (row-major). With fourColorOnly the 3-colour/punch-through
	// mode is disabled, as required for the BC2/BC3 colour sub-block (whose hardware decode is always 4-colour).
	void DecodeBC1(const u8* block, bool fourColorOnly, u8 outRGBA[64])
	{
		const u16 c0 = (u16)(block[0] | (block[1] << 8));
		const u16 c1 = (u16)(block[2] | (block[3] << 8));

		i32 pal[4][3];
		Expand565(c0, pal[0][0], pal[0][1], pal[0][2]);
		Expand565(c1, pal[1][0], pal[1][1], pal[1][2]);

		bool transparentBlack = false;
		if(c0 > c1 || fourColorOnly)
		{
			for(i32 channelIndex = 0; channelIndex < 3; ++channelIndex)
			{
				pal[2][channelIndex] = (2 * pal[0][channelIndex] + pal[1][channelIndex] + 1) / 3;
				pal[3][channelIndex] = (pal[0][channelIndex] + 2 * pal[1][channelIndex] + 1) / 3;
			}
		}
		else
		{
			for(i32 channelIndex = 0; channelIndex < 3; ++channelIndex)
			{
				pal[2][channelIndex] = (pal[0][channelIndex] + pal[1][channelIndex]) / 2;
				pal[3][channelIndex] = 0;
			}

			transparentBlack = true;
		}

		const u32 indices = block[4] | (block[5] << 8) | (block[6] << 16) | (block[7] << 24);
		for(i32 texelIndex = 0; texelIndex < 16; ++texelIndex)
		{
			const u32 idx = (indices >> (texelIndex * 2)) & 0x3;
			outRGBA[texelIndex * 4 + 0] = (u8)pal[idx][0];
			outRGBA[texelIndex * 4 + 1] = (u8)pal[idx][1];
			outRGBA[texelIndex * 4 + 2] = (u8)pal[idx][2];
			outRGBA[texelIndex * 4 + 3] = (transparentBlack && idx == 3) ? 0 : 255;
		}
	}

	// Decodes a BC4 block (8 bytes) into 16 single-channel values.
	void DecodeBC4(const u8* block, u8 out[16])
	{
		const i32 r0 = block[0];
		const i32 r1 = block[1];

		i32 pal[8];
		pal[0] = r0;
		pal[1] = r1;
		if(r0 > r1)
		{
			for(i32 k = 1; k < 7; ++k)
				pal[k + 1] = ((7 - k) * r0 + k * r1) / 7;
		}
		else
		{
			for(i32 k = 1; k < 5; ++k)
				pal[k + 1] = ((5 - k) * r0 + k * r1) / 5;
			pal[6] = 0;
			pal[7] = 255;
		}

		u64 bits = 0;
		for(i32 i = 0; i < 6; ++i)
			bits |= (u64)block[2 + i] << (i * 8);

		for(i32 texelIndex = 0; texelIndex < 16; ++texelIndex)
		{
			const u32 idx = (u32)((bits >> (texelIndex * 3)) & 0x7);
			out[texelIndex] = (u8)pal[idx];
		}
	}

	// Decodes a BC7 block (16 bytes) into 16 RGBA quads (row-major). Drives every field width off the standard BC7
	// per-mode descriptor table, so enabling a new encoder mode needs no decoder change. Matches the hardware decoder:
	// the P-bit is appended as the endpoint LSB, then the endpoint is bit-replicated up to 8 bits.
	void DecodeBC7(const u8* block, u8 outRGBA[64])
	{
		struct ModeDesc { u8 ns, pb, rb, isb, cb, ab, epb, spb, ib, ib2; };

		// ns=subsets, pb=partition bits, rb=rotation bits, isb=index-selector bits, cb=colour bits, ab=alpha bits,
		// epb=per-endpoint P-bits, spb=shared (per-subset) P-bits, ib=primary index bits, ib2=secondary index bits.
		static const ModeDesc kModes[8] = {
			{ 3, 4, 0, 0, 4, 0, 1, 0, 3, 0 }, // 0
			{ 2, 6, 0, 0, 6, 0, 0, 1, 3, 0 }, // 1
			{ 3, 6, 0, 0, 5, 0, 0, 0, 2, 0 }, // 2
			{ 2, 6, 0, 0, 7, 0, 1, 0, 2, 0 }, // 3
			{ 1, 0, 2, 1, 5, 6, 0, 0, 2, 3 }, // 4
			{ 1, 0, 2, 0, 7, 8, 0, 0, 2, 2 }, // 5
			{ 1, 0, 0, 0, 7, 7, 1, 0, 4, 0 }, // 6
			{ 2, 6, 0, 0, 5, 5, 1, 0, 2, 0 }, // 7
		};

		static const u32 kPart2[64] = {
			0x0000CCCCu, 0x00008888u, 0x0000EEEEu, 0x0000ECC8u, 0x0000C880u, 0x0000FEECu, 0x0000FEC8u, 0x0000EC80u,
			0x0000C800u, 0x0000FFECu, 0x0000FE80u, 0x0000E800u, 0x0000FFE8u, 0x0000FF00u, 0x0000FFF0u, 0x0000F000u,
			0x0000F710u, 0x0000008Eu, 0x00007100u, 0x000008CEu, 0x0000008Cu, 0x00007310u, 0x00003100u, 0x00008CCEu,
			0x0000088Cu, 0x00003110u, 0x00006666u, 0x0000366Cu, 0x000017E8u, 0x00000FF0u, 0x0000718Eu, 0x0000399Cu,
			0x0000AAAAu, 0x0000F0F0u, 0x00005A5Au, 0x000033CCu, 0x00003C3Cu, 0x000055AAu, 0x00009696u, 0x0000A55Au,
			0x000073CEu, 0x000013C8u, 0x0000324Cu, 0x00003BDCu, 0x00006996u, 0x0000C33Cu, 0x00009966u, 0x00000660u,
			0x00000272u, 0x000004E4u, 0x00004E40u, 0x00002720u, 0x0000C936u, 0x0000936Cu, 0x000039C6u, 0x0000639Cu,
			0x00009336u, 0x00009CC6u, 0x0000817Eu, 0x0000E718u, 0x0000CCF0u, 0x00000FCCu, 0x00007744u, 0x0000EE22u
		};

		static const u8 kAnchor2[64] = {
			15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
			15,  2,  8,  2,  2,  8,  8, 15,  2,  8,  2,  2,  8,  8,  2,  2,
			15, 15,  6,  8,  2,  8, 15, 15,  2,  8,  2,  2,  2, 15, 15,  6,
			 6,  2,  6,  8, 15, 15,  2,  2, 15, 15, 15, 15, 15,  2,  2, 15
		};

		static const u32 kPart3[64] = {
			0xF60008CCu, 0x73008CC8u, 0x3310CC80u, 0x00CEEC00u, 0xCC003300u, 0xCC0000CCu, 0x00CCFF00u, 0x3300CCCCu,
			0xF0000F00u, 0xF0000FF0u, 0xFF0000F0u, 0x88884444u, 0x88886666u, 0xCCCC2222u, 0xEC80136Cu, 0x7310008Cu,
			0xC80036C8u, 0x310008CEu, 0xCCC03330u, 0x0CCCF000u, 0xEE0000EEu, 0x77008888u, 0xCC0022C0u, 0x33004430u,
			0x00CC0C22u, 0xFC880344u, 0x06606996u, 0x66009960u, 0xC88C0330u, 0xF9000066u, 0x0CC0C22Cu, 0x73108C00u,
			0xEC801300u, 0x08CEC400u, 0xEC80004Cu, 0x44442222u, 0x0F0000F0u, 0x49242492u, 0x42942942u, 0x0C30C30Cu,
			0x03C0C03Cu, 0xFF0000AAu, 0x5500AA00u, 0xCCCC3030u, 0x0C0CC0C0u, 0x66669090u, 0x0FF0A00Au, 0x5550AAA0u,
			0xF0000AAAu, 0x0E0EE0E0u, 0x88887070u, 0x99906660u, 0xE00E0EE0u, 0x88880770u, 0xF0000666u, 0x99006600u,
			0xFF000066u, 0xC00C0CC0u, 0xCCCC0330u, 0x90006000u, 0x08088080u, 0xEEEE1010u, 0xFFF0000Au, 0x731008CEu
		};

		static const u8 kAnchor3a[64] = {
			 3,  3, 15, 15,  8,  3, 15, 15,  8,  8,  6,  6,  6,  5,  3,  3,
			 3,  3,  8, 15,  3,  3,  6, 10,  5,  8,  8,  6,  8,  5, 15, 15,
			 8, 15,  3,  5,  6, 10,  8, 15, 15,  3, 15,  5, 15, 15, 15, 15,
			 3, 15,  5,  5,  5,  8,  5, 10,  5, 10,  8, 13, 15, 12,  3,  3
		};

		static const u8 kAnchor3b[64] = {
			15,  8,  8,  3, 15, 15,  3,  8, 15, 15, 15, 15, 15, 15, 15,  8,
			15,  8, 15,  3, 15,  8, 15,  8,  3, 15,  6, 10, 15, 15, 10,  8,
			15,  3, 15, 10, 10,  8,  9, 10,  6, 15,  8, 15,  3,  6,  6,  8,
			15,  3, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,  8
		};

		static const i32 kWt[5][16] = {
			{ 0 },
			{ 0, 64 },
			{ 0, 21, 43, 64 },
			{ 0, 9, 18, 27, 37, 46, 55, 64 },
			{ 0, 4, 9, 13, 17, 21, 26, 30, 34, 38, 43, 47, 51, 55, 60, 64 }
		};

		u32 pos = 0;
		auto getBits = [&](u32 count) -> u32
		{
			u32 v = 0;
			for(u32 i = 0; i < count; ++i)
			{
				const u32 p = pos + i;
				v |= ((block[p >> 3] >> (p & 7)) & 1u) << i;
			}
			pos += count;
			return v;
		};

		u32 mode = 0;
		while(mode < 8 && getBits(1) == 0)
			++mode;

		if(mode >= 8)
		{
			for(i32 i = 0; i < 64; ++i)
				outRGBA[i] = 0;

			return;
		}
		const ModeDesc& m = kModes[mode];

		const u32 rotation = m.rb ? getBits(m.rb) : 0;
		const u32 idxSel = m.isb ? getBits(m.isb) : 0;
		const u32 partition = m.pb ? getBits(m.pb) : 0;
		const u32 numEnd = m.ns * 2u;

		// Endpoints are stored channel-major: all R, then all G, all B, then (if present) all A.
		i32 ep[6][4] = {};
		for(u32 e = 0; e < numEnd; ++e) ep[e][0] = (i32)getBits(m.cb);
		for(u32 e = 0; e < numEnd; ++e) ep[e][1] = (i32)getBits(m.cb);
		for(u32 e = 0; e < numEnd; ++e) ep[e][2] = (i32)getBits(m.cb);
		if(m.ab)
			for(u32 e = 0; e < numEnd; ++e) ep[e][3] = (i32)getBits(m.ab);

		i32 pbit[6] = { -1, -1, -1, -1, -1, -1 };
		if(m.epb)
			for(u32 e = 0; e < numEnd; ++e) pbit[e] = (i32)getBits(1);
		else if(m.spb)
			for(u32 s = 0; s < m.ns; ++s) { const i32 b = (i32)getBits(1); pbit[2 * s] = b; pbit[2 * s + 1] = b; }

		const i32 cbits = m.cb + ((m.epb || m.spb) ? 1 : 0);
		const i32 abits = m.ab ? (m.ab + ((m.epb || m.spb) ? 1 : 0)) : 0;
		auto expand = [](i32 v, i32 bits) -> i32 { return (v << (8 - bits)) | (v >> (2 * bits - 8)); };

		i32 R8[6], G8[6], B8[6], A8[6];
		for(u32 e = 0; e < numEnd; ++e)
		{
			const i32 p = pbit[e];
			const i32 r = (p >= 0) ? ((ep[e][0] << 1) | p) : ep[e][0];
			const i32 g = (p >= 0) ? ((ep[e][1] << 1) | p) : ep[e][1];
			const i32 b = (p >= 0) ? ((ep[e][2] << 1) | p) : ep[e][2];
			R8[e] = expand(r, cbits);
			G8[e] = expand(g, cbits);
			B8[e] = expand(b, cbits);
			if(m.ab)
			{
				const i32 a = (p >= 0) ? ((ep[e][3] << 1) | p) : ep[e][3];
				A8[e] = expand(a, abits);
			}
			else
				A8[e] = 255;
		}

		u32 anchor[3] = { 0, 0, 0 };
		if(m.ns == 2)
			anchor[1] = kAnchor2[partition];
		else if(m.ns == 3)
		{
			anchor[1] = kAnchor3a[partition];
			anchor[2] = kAnchor3b[partition];
		}

		u32 idx[16] = {};
		u32 idx2[16] = {};
		for(u32 t = 0; t < 16; ++t)
		{
			const bool isAnchor = (t == anchor[0]) || (m.ns >= 2 && t == anchor[1]) || (m.ns >= 3 && t == anchor[2]);
			idx[t] = getBits(isAnchor ? (u32)(m.ib - 1) : m.ib);
		}
		if(m.ib2)
			for(u32 t = 0; t < 16; ++t)
				idx2[t] = getBits((t == 0) ? (u32)(m.ib2 - 1) : m.ib2);

		for(u32 t = 0; t < 16; ++t)
		{
			u32 subset = 0;
			if(m.ns == 2)
				subset = (kPart2[partition] >> t) & 1u;
			else if(m.ns == 3)
			{
				const u32 pv = kPart3[partition];
				subset = ((pv >> (16u + t)) & 1u) ? 2u : (((pv >> t) & 1u) ? 1u : 0u);
			}
			const u32 e0 = 2 * subset, e1 = 2 * subset + 1;

			u32 ci, cw, ai, aw;
			if(m.ib2 == 0) { ci = idx[t]; cw = m.ib; ai = idx[t]; aw = m.ib; }
			else if(idxSel == 0) { ci = idx[t]; cw = m.ib; ai = idx2[t]; aw = m.ib2; }
			else { ci = idx2[t]; cw = m.ib2; ai = idx[t]; aw = m.ib; }

			const i32 wc = kWt[cw][ci];
			const i32 wa = kWt[aw][ai];
			i32 R = ((64 - wc) * R8[e0] + wc * R8[e1] + 32) >> 6;
			i32 G = ((64 - wc) * G8[e0] + wc * G8[e1] + 32) >> 6;
			i32 B = ((64 - wc) * B8[e0] + wc * B8[e1] + 32) >> 6;
			i32 A = ((64 - wa) * A8[e0] + wa * A8[e1] + 32) >> 6;

			if(rotation == 1) { const i32 tmp = R; R = A; A = tmp; }
			else if(rotation == 2) { const i32 tmp = G; G = A; A = tmp; }
			else if(rotation == 3) { const i32 tmp = B; B = A; A = tmp; }

			outRGBA[t * 4 + 0] = (u8)R;
			outRGBA[t * 4 + 1] = (u8)G;
			outRGBA[t * 4 + 2] = (u8)B;
			outRGBA[t * 4 + 3] = (u8)A;
		}
	}

	// Decodes a BC6H block in the unsigned (UF16) variant (16 bytes) into 16 RGB float triplets. Handles all 14 modes;
	// an unrecognised mode lead outputs zero.
	void DecodeBC6H_UF16(const u8* block, float out[16][3])
	{
		const u32 lead = (block[0] & 0x02) ? (block[0] & 0x1F) : (block[0] & 0x01);

		i32 wBits = 0;
		i32 tBits[3] = { 0, 0, 0 };
		bool oneRegion = true;
		bool transformed = false;
		i32 shape = 0;
		// Endpoint codes: ecA[endpoint A/B for subset 0][channel] and ecB for subset 1 (two-region).
		i32 ecA[2][3] = { {0, 0, 0}, {0, 0, 0} }; // [subset][ch] endpoint A (w / y)
		i32 ecB[2][3] = { {0, 0, 0}, {0, 0, 0} }; // [subset][ch] endpoint B (x / z)

		if(lead == 0x03) // mode 11: one region, 10:10, no transform
		{
			wBits = 10; oneRegion = true; transformed = false;
			tBits[0] = tBits[1] = tBits[2] = 10;
			ecA[0][0] = BC6GetBits(block, 5, 10);  // rw
			ecA[0][1] = BC6GetBits(block, 15, 10); // gw
			ecA[0][2] = BC6GetBits(block, 25, 10); // bw
			ecB[0][0] = BC6GetBits(block, 35, 10); // rx
			ecB[0][1] = BC6GetBits(block, 45, 10); // gx
			ecB[0][2] = BC6GetBits(block, 55, 10); // bx
		}
		else if(lead == 0x07) // mode 12: one region, 11-bit base, 9-bit delta (transform)
		{
			wBits = 11; oneRegion = true; transformed = true;
			tBits[0] = tBits[1] = tBits[2] = 9;
			ecA[0][0] = BC6GetBits(block, 5, 10) | (BC6GetBits(block, 44, 1) << 10);
			ecA[0][1] = BC6GetBits(block, 15, 10) | (BC6GetBits(block, 54, 1) << 10);
			ecA[0][2] = BC6GetBits(block, 25, 10) | (BC6GetBits(block, 64, 1) << 10);
			ecB[0][0] = BC6GetBits(block, 35, 9);
			ecB[0][1] = BC6GetBits(block, 45, 9);
			ecB[0][2] = BC6GetBits(block, 55, 9);
		}
		else if(lead == 0x0B) // mode 13: one region, 12-bit base, 8-bit delta (transform)
		{
			wBits = 12; oneRegion = true; transformed = true;
			tBits[0] = tBits[1] = tBits[2] = 8;
			ecA[0][0] = BC6GetBits(block, 5, 10) | (BC6GetBits(block, 44, 1) << 10) | (BC6GetBits(block, 43, 1) << 11);
			ecA[0][1] = BC6GetBits(block, 15, 10) | (BC6GetBits(block, 54, 1) << 10) | (BC6GetBits(block, 53, 1) << 11);
			ecA[0][2] = BC6GetBits(block, 25, 10) | (BC6GetBits(block, 64, 1) << 10) | (BC6GetBits(block, 63, 1) << 11);
			ecB[0][0] = BC6GetBits(block, 35, 8);
			ecB[0][1] = BC6GetBits(block, 45, 8);
			ecB[0][2] = BC6GetBits(block, 55, 8);
		}
		else if(lead == 0x0F) // mode 14: one region, 16-bit base, 4-bit delta (transform)
		{
			wBits = 16; oneRegion = true; transformed = true;
			tBits[0] = tBits[1] = tBits[2] = 4;
			// The top 6 base bits [15:10] are stored MSB-first (block bit 39 = rw[15] .. bit 44 = rw[10]); read them
			// reversed, the same way mode 13 reads its 2 high bits.
			ecA[0][0] = BC6GetBits(block, 5, 10)
			          | (BC6GetBits(block, 44, 1) << 10) | (BC6GetBits(block, 43, 1) << 11) | (BC6GetBits(block, 42, 1) << 12)
			          | (BC6GetBits(block, 41, 1) << 13) | (BC6GetBits(block, 40, 1) << 14) | (BC6GetBits(block, 39, 1) << 15);
			ecA[0][1] = BC6GetBits(block, 15, 10)
			          | (BC6GetBits(block, 54, 1) << 10) | (BC6GetBits(block, 53, 1) << 11) | (BC6GetBits(block, 52, 1) << 12)
			          | (BC6GetBits(block, 51, 1) << 13) | (BC6GetBits(block, 50, 1) << 14) | (BC6GetBits(block, 49, 1) << 15);
			ecA[0][2] = BC6GetBits(block, 25, 10)
			          | (BC6GetBits(block, 64, 1) << 10) | (BC6GetBits(block, 63, 1) << 11) | (BC6GetBits(block, 62, 1) << 12)
			          | (BC6GetBits(block, 61, 1) << 13) | (BC6GetBits(block, 60, 1) << 14) | (BC6GetBits(block, 59, 1) << 15);
			ecB[0][0] = BC6GetBits(block, 35, 4);
			ecB[0][1] = BC6GetBits(block, 45, 4);
			ecB[0][2] = BC6GetBits(block, 55, 4);
		}
		else if(lead == 0x00) // mode 1: two region, 10-bit base, 5/5/5 delta (transform)
		{
			wBits = 10; oneRegion = false; transformed = true;
			tBits[0] = tBits[1] = tBits[2] = 5;
			shape = (i32)BC6GetBits(block, 77, 5);
			ecA[0][0] = BC6GetBits(block, 5, 10);  // rw
			ecA[0][1] = BC6GetBits(block, 15, 10); // gw
			ecA[0][2] = BC6GetBits(block, 25, 10); // bw
			ecB[0][0] = BC6GetBits(block, 35, 5);  // rx
			ecB[0][1] = BC6GetBits(block, 45, 5);  // gx
			ecB[0][2] = BC6GetBits(block, 55, 5);  // bx
			ecA[1][0] = BC6GetBits(block, 65, 5);  // ry
			ecA[1][1] = BC6GetBits(block, 41, 4) | (BC6GetBits(block, 2, 1) << 4);  // gy
			ecA[1][2] = BC6GetBits(block, 61, 4) | (BC6GetBits(block, 3, 1) << 4);  // by
			ecB[1][0] = BC6GetBits(block, 71, 5);  // rz
			ecB[1][1] = BC6GetBits(block, 51, 4) | (BC6GetBits(block, 40, 1) << 4); // gz
			ecB[1][2] = BC6GetBits(block, 50, 1) | (BC6GetBits(block, 60, 1) << 1)
			          | (BC6GetBits(block, 70, 1) << 2) | (BC6GetBits(block, 76, 1) << 3)
			          | (BC6GetBits(block, 4, 1) << 4);                              // bz
		}
		else if(lead == 0x01) // mode 2: two region, 7-bit base, 6/6/6 delta (transform)
		{
			wBits = 7; oneRegion = false; transformed = true;
			tBits[0] = tBits[1] = tBits[2] = 6;
			shape = (i32)BC6GetBits(block, 77, 5);
			ecA[0][0] = BC6GetBits(block, 5, 7);   // rw
			ecA[0][1] = BC6GetBits(block, 15, 7);  // gw
			ecA[0][2] = BC6GetBits(block, 25, 7);  // bw
			ecB[0][0] = BC6GetBits(block, 35, 6);  // rx
			ecB[0][1] = BC6GetBits(block, 45, 6);  // gx
			ecB[0][2] = BC6GetBits(block, 55, 6);  // bx
			ecA[1][0] = BC6GetBits(block, 65, 6);  // ry
			ecA[1][1] = BC6GetBits(block, 41, 4) | (BC6GetBits(block, 24, 1) << 4) | (BC6GetBits(block, 2, 1) << 5); // gy
			ecA[1][2] = BC6GetBits(block, 61, 4) | (BC6GetBits(block, 14, 1) << 4) | (BC6GetBits(block, 22, 1) << 5); // by
			ecB[1][0] = BC6GetBits(block, 71, 6);  // rz
			ecB[1][1] = BC6GetBits(block, 51, 4) | (BC6GetBits(block, 3, 1) << 4) | (BC6GetBits(block, 4, 1) << 5);   // gz
			ecB[1][2] = BC6GetBits(block, 12, 1) | (BC6GetBits(block, 13, 1) << 1) | (BC6GetBits(block, 23, 1) << 2)
			          | (BC6GetBits(block, 32, 1) << 3) | (BC6GetBits(block, 34, 1) << 4) | (BC6GetBits(block, 33, 1) << 5); // bz
		}
		else if(lead == 0x02) // mode 3: two region, 11-bit base, 5/4/4 delta (transform)
		{
			wBits = 11; oneRegion = false; transformed = true;
			tBits[0] = 5; tBits[1] = 4; tBits[2] = 4;
			shape = (i32)BC6GetBits(block, 77, 5);
			ecA[0][0] = BC6GetBits(block, 5, 10) | (BC6GetBits(block, 40, 1) << 10);  // rw
			ecA[0][1] = BC6GetBits(block, 15, 10) | (BC6GetBits(block, 49, 1) << 10); // gw
			ecA[0][2] = BC6GetBits(block, 25, 10) | (BC6GetBits(block, 59, 1) << 10); // bw
			ecB[0][0] = BC6GetBits(block, 35, 5);  // rx
			ecB[0][1] = BC6GetBits(block, 45, 4);  // gx
			ecB[0][2] = BC6GetBits(block, 55, 4);  // bx
			ecA[1][0] = BC6GetBits(block, 65, 5);  // ry
			ecA[1][1] = BC6GetBits(block, 41, 4);  // gy
			ecA[1][2] = BC6GetBits(block, 61, 4);  // by
			ecB[1][0] = BC6GetBits(block, 71, 5);  // rz
			ecB[1][1] = BC6GetBits(block, 51, 4);  // gz
			ecB[1][2] = BC6GetBits(block, 50, 1) | (BC6GetBits(block, 60, 1) << 1)
			          | (BC6GetBits(block, 70, 1) << 2) | (BC6GetBits(block, 76, 1) << 3); // bz
		}
		else if(lead == 0x06) // mode 4: two region, 11-bit base, 4/5/4 delta (transform)
		{
			wBits = 11; oneRegion = false; transformed = true;
			tBits[0] = 4; tBits[1] = 5; tBits[2] = 4;
			shape = (i32)BC6GetBits(block, 77, 5);
			ecA[0][0] = BC6GetBits(block, 5, 10) | (BC6GetBits(block, 39, 1) << 10);  // rw
			ecA[0][1] = BC6GetBits(block, 15, 10) | (BC6GetBits(block, 50, 1) << 10); // gw
			ecA[0][2] = BC6GetBits(block, 25, 10) | (BC6GetBits(block, 59, 1) << 10); // bw
			ecB[0][0] = BC6GetBits(block, 35, 4);  // rx
			ecB[0][1] = BC6GetBits(block, 45, 5);  // gx
			ecB[0][2] = BC6GetBits(block, 55, 4);  // bx
			ecA[1][0] = BC6GetBits(block, 65, 4);  // ry
			ecA[1][1] = BC6GetBits(block, 41, 4) | (BC6GetBits(block, 75, 1) << 4);  // gy
			ecA[1][2] = BC6GetBits(block, 61, 4);  // by
			ecB[1][0] = BC6GetBits(block, 71, 4);  // rz
			ecB[1][1] = BC6GetBits(block, 51, 4) | (BC6GetBits(block, 40, 1) << 4); // gz
			ecB[1][2] = BC6GetBits(block, 69, 1) | (BC6GetBits(block, 60, 1) << 1)
			          | (BC6GetBits(block, 70, 1) << 2) | (BC6GetBits(block, 76, 1) << 3); // bz
		}
		else if(lead == 0x0A) // mode 5: two region, 11-bit base, 4/4/5 delta (transform)
		{
			wBits = 11; oneRegion = false; transformed = true;
			tBits[0] = 4; tBits[1] = 4; tBits[2] = 5;
			shape = (i32)BC6GetBits(block, 77, 5);
			ecA[0][0] = BC6GetBits(block, 5, 10) | (BC6GetBits(block, 39, 1) << 10);  // rw
			ecA[0][1] = BC6GetBits(block, 15, 10) | (BC6GetBits(block, 49, 1) << 10); // gw
			ecA[0][2] = BC6GetBits(block, 25, 10) | (BC6GetBits(block, 60, 1) << 10); // bw
			ecB[0][0] = BC6GetBits(block, 35, 4);  // rx
			ecB[0][1] = BC6GetBits(block, 45, 4);  // gx
			ecB[0][2] = BC6GetBits(block, 55, 5);  // bx
			ecA[1][0] = BC6GetBits(block, 65, 4);  // ry
			ecA[1][1] = BC6GetBits(block, 41, 4);  // gy
			ecA[1][2] = BC6GetBits(block, 61, 4) | (BC6GetBits(block, 40, 1) << 4);  // by
			ecB[1][0] = BC6GetBits(block, 71, 4);  // rz
			ecB[1][1] = BC6GetBits(block, 51, 4);  // gz
			ecB[1][2] = BC6GetBits(block, 50, 1) | (BC6GetBits(block, 69, 1) << 1)
			          | (BC6GetBits(block, 70, 1) << 2) | (BC6GetBits(block, 76, 1) << 3)
			          | (BC6GetBits(block, 75, 1) << 4);                              // bz
		}
		else if(lead == 0x0E) // mode 6: two region, 9-bit base, 5/5/5 delta (transform)
		{
			wBits = 9; oneRegion = false; transformed = true;
			tBits[0] = tBits[1] = tBits[2] = 5;
			shape = (i32)BC6GetBits(block, 77, 5);
			ecA[0][0] = BC6GetBits(block, 5, 9);   // rw
			ecA[0][1] = BC6GetBits(block, 15, 9);  // gw
			ecA[0][2] = BC6GetBits(block, 25, 9);  // bw
			ecB[0][0] = BC6GetBits(block, 35, 5);  // rx
			ecB[0][1] = BC6GetBits(block, 45, 5);  // gx
			ecB[0][2] = BC6GetBits(block, 55, 5);  // bx
			ecA[1][0] = BC6GetBits(block, 65, 5);  // ry
			ecA[1][1] = BC6GetBits(block, 41, 4) | (BC6GetBits(block, 24, 1) << 4);  // gy
			ecA[1][2] = BC6GetBits(block, 61, 4) | (BC6GetBits(block, 14, 1) << 4);  // by
			ecB[1][0] = BC6GetBits(block, 71, 5);  // rz
			ecB[1][1] = BC6GetBits(block, 51, 4) | (BC6GetBits(block, 40, 1) << 4); // gz
			ecB[1][2] = BC6GetBits(block, 50, 1) | (BC6GetBits(block, 60, 1) << 1)
			          | (BC6GetBits(block, 70, 1) << 2) | (BC6GetBits(block, 76, 1) << 3)
			          | (BC6GetBits(block, 34, 1) << 4);                              // bz
		}
		else if(lead == 0x12) // mode 7: two region, 8-bit base, 6/5/5 delta (transform)
		{
			wBits = 8; oneRegion = false; transformed = true;
			tBits[0] = 6; tBits[1] = 5; tBits[2] = 5;
			shape = (i32)BC6GetBits(block, 77, 5);
			ecA[0][0] = BC6GetBits(block, 5, 8);   // rw
			ecA[0][1] = BC6GetBits(block, 15, 8);  // gw
			ecA[0][2] = BC6GetBits(block, 25, 8);  // bw
			ecB[0][0] = BC6GetBits(block, 35, 6);  // rx
			ecB[0][1] = BC6GetBits(block, 45, 5);  // gx
			ecB[0][2] = BC6GetBits(block, 55, 5);  // bx
			ecA[1][0] = BC6GetBits(block, 65, 6);  // ry
			ecA[1][1] = BC6GetBits(block, 41, 4) | (BC6GetBits(block, 24, 1) << 4);  // gy
			ecA[1][2] = BC6GetBits(block, 61, 4) | (BC6GetBits(block, 14, 1) << 4);  // by
			ecB[1][0] = BC6GetBits(block, 71, 6);  // rz
			ecB[1][1] = BC6GetBits(block, 51, 4) | (BC6GetBits(block, 13, 1) << 4); // gz
			ecB[1][2] = BC6GetBits(block, 50, 1) | (BC6GetBits(block, 60, 1) << 1)
			          | (BC6GetBits(block, 23, 1) << 2) | (BC6GetBits(block, 33, 1) << 3)
			          | (BC6GetBits(block, 34, 1) << 4);                              // bz
		}
		else if(lead == 0x16) // mode 8: two region, 8-bit base, 5/6/5 delta (transform)
		{
			wBits = 8; oneRegion = false; transformed = true;
			tBits[0] = 5; tBits[1] = 6; tBits[2] = 5;
			shape = (i32)BC6GetBits(block, 77, 5);
			ecA[0][0] = BC6GetBits(block, 5, 8);   // rw
			ecA[0][1] = BC6GetBits(block, 15, 8);  // gw
			ecA[0][2] = BC6GetBits(block, 25, 8);  // bw
			ecB[0][0] = BC6GetBits(block, 35, 5);  // rx
			ecB[0][1] = BC6GetBits(block, 45, 6);  // gx
			ecB[0][2] = BC6GetBits(block, 55, 5);  // bx
			ecA[1][0] = BC6GetBits(block, 65, 5);  // ry
			ecA[1][1] = BC6GetBits(block, 41, 4) | (BC6GetBits(block, 24, 1) << 4) | (BC6GetBits(block, 23, 1) << 5); // gy
			ecA[1][2] = BC6GetBits(block, 61, 4) | (BC6GetBits(block, 14, 1) << 4);  // by
			ecB[1][0] = BC6GetBits(block, 71, 5);  // rz
			ecB[1][1] = BC6GetBits(block, 51, 4) | (BC6GetBits(block, 40, 1) << 4) | (BC6GetBits(block, 33, 1) << 5); // gz
			ecB[1][2] = BC6GetBits(block, 13, 1) | (BC6GetBits(block, 60, 1) << 1)
			          | (BC6GetBits(block, 70, 1) << 2) | (BC6GetBits(block, 76, 1) << 3)
			          | (BC6GetBits(block, 34, 1) << 4);                              // bz
		}
		else if(lead == 0x1A) // mode 9: two region, 8-bit base, 5/5/6 delta (transform)
		{
			wBits = 8; oneRegion = false; transformed = true;
			tBits[0] = 5; tBits[1] = 5; tBits[2] = 6;
			shape = (i32)BC6GetBits(block, 77, 5);
			ecA[0][0] = BC6GetBits(block, 5, 8);   // rw
			ecA[0][1] = BC6GetBits(block, 15, 8);  // gw
			ecA[0][2] = BC6GetBits(block, 25, 8);  // bw
			ecB[0][0] = BC6GetBits(block, 35, 5);  // rx
			ecB[0][1] = BC6GetBits(block, 45, 5);  // gx
			ecB[0][2] = BC6GetBits(block, 55, 6);  // bx
			ecA[1][0] = BC6GetBits(block, 65, 5);  // ry
			ecA[1][1] = BC6GetBits(block, 41, 4) | (BC6GetBits(block, 24, 1) << 4);  // gy
			ecA[1][2] = BC6GetBits(block, 61, 4) | (BC6GetBits(block, 14, 1) << 4) | (BC6GetBits(block, 23, 1) << 5); // by
			ecB[1][0] = BC6GetBits(block, 71, 5);  // rz
			ecB[1][1] = BC6GetBits(block, 51, 4) | (BC6GetBits(block, 40, 1) << 4); // gz
			ecB[1][2] = BC6GetBits(block, 50, 1) | (BC6GetBits(block, 13, 1) << 1)
			          | (BC6GetBits(block, 70, 1) << 2) | (BC6GetBits(block, 76, 1) << 3)
			          | (BC6GetBits(block, 34, 1) << 4) | (BC6GetBits(block, 33, 1) << 5); // bz
		}
		else if(lead == 0x1E) // mode 10: two region, 6-bit base, 6/6/6, no transform
		{
			wBits = 6; oneRegion = false; transformed = false;
			tBits[0] = tBits[1] = tBits[2] = 6;
			shape = (i32)BC6GetBits(block, 77, 5);
			ecA[0][0] = BC6GetBits(block, 5, 6);   // rw
			ecA[0][1] = BC6GetBits(block, 15, 6);  // gw
			ecA[0][2] = BC6GetBits(block, 25, 6);  // bw
			ecB[0][0] = BC6GetBits(block, 35, 6);  // rx
			ecB[0][1] = BC6GetBits(block, 45, 6);  // gx
			ecB[0][2] = BC6GetBits(block, 55, 6);  // bx
			ecA[1][0] = BC6GetBits(block, 65, 6);  // ry
			ecA[1][1] = BC6GetBits(block, 41, 4) | (BC6GetBits(block, 24, 1) << 4) | (BC6GetBits(block, 21, 1) << 5); // gy
			ecA[1][2] = BC6GetBits(block, 61, 4) | (BC6GetBits(block, 14, 1) << 4) | (BC6GetBits(block, 22, 1) << 5); // by
			ecB[1][0] = BC6GetBits(block, 71, 6);  // rz
			ecB[1][1] = BC6GetBits(block, 51, 4) | (BC6GetBits(block, 11, 1) << 4) | (BC6GetBits(block, 31, 1) << 5); // gz
			ecB[1][2] = BC6GetBits(block, 12, 1) | (BC6GetBits(block, 13, 1) << 1)
			          | (BC6GetBits(block, 23, 1) << 2) | (BC6GetBits(block, 32, 1) << 3)
			          | (BC6GetBits(block, 34, 1) << 4) | (BC6GetBits(block, 33, 1) << 5); // bz
		}
		else
		{
			for(i32 i = 0; i < 16; ++i) { out[i][0] = out[i][1] = out[i][2] = 0.0f; }
			return;
		}

		// Endpoint transform (UF16): subset-0 A is the base; the others are deltas off it when transformed.
		const i32 numSubsets = oneRegion ? 1 : 2;
		i32 EA[2][3], EB[2][3];
		const i32 wmask = (1 << wBits) - 1;
		for(i32 ch = 0; ch < 3; ++ch)
		{
			const i32 base = ecA[0][ch];
			EA[0][ch] = base;
			EB[0][ch] = transformed ? ((BC6SignExtend(ecB[0][ch], tBits[ch]) + base) & wmask) : ecB[0][ch];
			if(numSubsets == 2)
			{
				EA[1][ch] = transformed ? ((BC6SignExtend(ecA[1][ch], tBits[ch]) + base) & wmask) : ecA[1][ch];
				EB[1][ch] = transformed ? ((BC6SignExtend(ecB[1][ch], tBits[ch]) + base) & wmask) : ecB[1][ch];
			}
		}

		// Palette: 16 entries (one region, 4-bit) or 8 (two region, 3-bit).
		const i32 maxIdx = oneRegion ? 16 : 8;
		i32 pal[2][16][3];
		for(i32 r = 0; r < numSubsets; ++r)
			for(i32 ch = 0; ch < 3; ++ch)
			{
				const i32 a = BC6Unquantize(EA[r][ch], wBits);
				const i32 b = BC6Unquantize(EB[r][ch], wBits);
				for(i32 i = 0; i < maxIdx; ++i)
					pal[r][i][ch] = BC6Finish(BC6Lerp(a, b, i, maxIdx - 1));
			}

		// Indices.
		u32 idx[16];
		if(oneRegion)
		{
			u32 start = 65;
			idx[0] = BC6GetBits(block, start, 3); start += 3;
			for(i32 i = 1; i < 16; ++i) { idx[i] = BC6GetBits(block, start, 4); start += 4; }
		}
		else
		{
			static const i32 anchorTable[32] = {
				15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
				15,  2,  8,  2,  2,  8,  8, 15,  2,  8,  2,  2,  8,  8,  2,  2 };
			u32 start = 82;
			idx[0] = BC6GetBits(block, start, 2); start += 2;
			for(i32 i = 1; i < 16; ++i)
			{
				const i32 nbits = (anchorTable[shape] == i) ? 2 : 3;
				idx[i] = BC6GetBits(block, start, nbits); start += nbits;
			}
		}

		static const u32 part2[32] = {
			0x0000CCCCu, 0x00008888u, 0x0000EEEEu, 0x0000ECC8u, 0x0000C880u, 0x0000FEECu, 0x0000FEC8u, 0x0000EC80u,
			0x0000C800u, 0x0000FFECu, 0x0000FE80u, 0x0000E800u, 0x0000FFE8u, 0x0000FF00u, 0x0000FFF0u, 0x0000F000u,
			0x0000F710u, 0x0000008Eu, 0x00007100u, 0x000008CEu, 0x0000008Cu, 0x00007310u, 0x00003100u, 0x00008CCEu,
			0x0000088Cu, 0x00003110u, 0x00006666u, 0x0000366Cu, 0x000017E8u, 0x00000FF0u, 0x0000718Eu, 0x0000399Cu };

		for(i32 i = 0; i < 16; ++i)
		{
			const i32 r = oneRegion ? 0 : (i32)((part2[shape] >> i) & 1u);
			const i32 k = (i32)idx[i];
			for(i32 ch = 0; ch < 3; ++ch)
				out[i][ch] = BC6HalfToFloat((u16)pal[r][k][ch]);
		}
	}

	// ---- GPU hardware-decoder ----

	// Decodes a compressed surface with the GPU's own texture-unit decoder - the exact path used at runtime when the
	// texture is sampled - so PSNR can be measured against what the hardware actually reconstructs. This is the ground
	// truth the CPU reference decoders above are checked against: a large CPU-vs-hardware gap means the CPU decode model
	// (shared by the encoder's cost function) has drifted from the spec, or the backend maps the format incorrectly.

	/** 
	 * RendererMaterial wrapper around the GPU block-decode compute shader: samples a compressed texture and writes the
	 * hardware-decoded RGB into a full-resolution RGBA32F UAV. 
	 */
	class TextureDecompressMaterial : public render::RendererMaterial<TextureDecompressMaterial>
	{
		RMAT_DEF("TextureDecompress.bsl")

	public:
		TextureDecompressMaterial() = default;

		void Initialize() override
		{
			mGpuParameterSet->TryGetUniformBufferParameter("Parameters", mParametersBuffer);
		}

		/** Records the dispatch that decodes input (compressed, sampled) into output (RGBA32F UAV). Render thread only. */
		void Execute(render::GpuCommandBuffer& commandBuffer, const TShared<render::Texture>& input, const TShared<render::Texture>& output, const render::GpuBufferMappedScope& parameters, const Vector2I& size)
		{
			mGpuParameterSet->SetSampledTexture("gInput", input);
			mGpuParameterSet->SetStorageTexture("gOutput", output, TextureSurface::kComplete);
			mParametersBuffer.Set(parameters);

			Bind(commandBuffer);
			commandBuffer.DispatchCompute((u32)Math::DivideAndRoundUp(size.X, 8), (u32)Math::DivideAndRoundUp(size.Y, 8));
		}

	private:
		render::GpuParameterUniformBuffer mParametersBuffer;
	};

	/**
	 * Decodes compressed on the render thread using the GPU's texture-unit decoder and sets up an asynchronous read-back.
	 * Uploads the blocks into a sampleable texture of @p compressed's format, dispatches the decode kernel (one thread per
	 * pixel, point-sampling each texel centre), then queues a non-blocking read-back into a freshly allocated RGBA32F
	 * surface. Completes op with that surface, or null on failure.
	 */
	void DecodeOnRenderThread(const TShared<PixelData>& compressed, TAsyncOp<TShared<PixelData>> op)
	{
		AssertIfNotRenderThread();

		const TShared<GpuDevice> gpuDevice = GetApplication().GetPrimaryGpuDevice();
		if(gpuDevice == nullptr)
		{
			op.CompleteOperation(nullptr);
			return;
		}

		GpuWorkContext& gpuContext = render::GetRenderer()->GetGpuContext();

		TextureDecompressMaterial* const material = TextureDecompressMaterial::Get();
		if(material == nullptr || material->GetComputePipeline() == nullptr)
		{
			op.CompleteOperation(nullptr);
			return;
		}

		const u32 width = compressed->GetWidth();
		const u32 height = compressed->GetHeight();
		if(width == 0 || height == 0)
		{
			op.CompleteOperation(nullptr);
			return;
		}

		const TShared<render::GpuCommandBufferPool> pool = gpuDevice->CreateGpuCommandBufferPool(render::GpuCommandBufferPoolCreateInformation::CreateForThisThread());

		// Compressed source as a sampled texture: sampling it runs the GPU's hardware decode. Uploaded once below.
		TextureCreateInformation inputTextureCreateInformation;
		inputTextureCreateInformation.Type = TEX_TYPE_2D;
		inputTextureCreateInformation.Format = compressed->GetFormat();
		inputTextureCreateInformation.Width = width;
		inputTextureCreateInformation.Height = height;
		inputTextureCreateInformation.Usage = TextureUsageFlag::StoreOnGPU; // device-local, sampled-only

		const TShared<render::Texture> inputTexture = gpuDevice->CreateTexture(inputTextureCreateInformation);
		if(inputTexture == nullptr)
		{
			op.CompleteOperation(nullptr);
			return;
		}

		// Full-resolution decoded output (RGBA32F), device-local + UAV so the compute kernel can write it.
		const TShared<PixelData> destination = PixelData::Create(width, height, 1, PF_RGBA32F);

		TextureCreateInformation outputTextureCreateInformation;
		outputTextureCreateInformation.Type = TEX_TYPE_2D;
		outputTextureCreateInformation.Format = PF_RGBA32F;
		outputTextureCreateInformation.Width = width;
		outputTextureCreateInformation.Height = height;
		outputTextureCreateInformation.Usage = TextureUsageFlag::StoreOnGPU | TextureUsageFlag::AllowUnorderedAccessOnTheGPU;

		const TShared<render::Texture> outputTexture = gpuDevice->CreateTexture(outputTextureCreateInformation);
		if(outputTexture == nullptr)
		{
			op.CompleteOperation(nullptr);
			return;
		}

		const Vector2I size((i32)width, (i32)height);

		const TShared<render::GpuBuffer> parameterBuffer = render::gTextureDecompressParameters.CreateTransientBuffer(gpuContext);
		if(parameterBuffer == nullptr)
		{
			op.CompleteOperation(nullptr);
			return;
		}

		render::GpuBufferMappedScope parameters = parameterBuffer->Map(GpuMapOption::Write);
		render::gTextureDecompressParameters.gSize.Set(parameters, size);
		parameters.Unmap();

		// Dispatch command buffer: upload the blocks, then decode (the dispatch barrier orders the upload before the first sample).
		render::GpuCommandBufferCreateInformation commandBufferInfo;
		commandBufferInfo.Name = "TextureDecompress";

		const TShared<render::GpuCommandBuffer> commandBuffer = pool->Create(commandBufferInfo);

		render::TextureUtility::Write(gpuContext, inputTexture, *compressed, 0, 0, render::TextureWriteFlag::Normal, commandBuffer);
		material->Execute(*commandBuffer, inputTexture, outputTexture, parameters, size);

		gpuContext.SubmitCommandBuffer(commandBuffer);
		gpuDevice->WaitUntilIdle();

		// Final command buffer: a single non-blocking read-back of the decoded surface.
		render::GpuCommandBufferCreateInformation readbackInfo;
		readbackInfo.Name = "TextureDecompressReadback";

		const TShared<render::GpuCommandBuffer> readbackCommandBuffer = pool->Create(readbackInfo);

		const u32 readSize = destination->GetConsecutiveSize();
		TAsyncOp<TShared<PixelData>> readOp = render::TextureUtility::ReadAsync(gpuContext, outputTexture, *readbackCommandBuffer);

		readbackCommandBuffer->OnDidComplete.Connect(
			[op, readOp, outputTexture, inputTexture, destination, readSize]() mutable
			{
				const TShared<PixelData> data = readOp.GetReturnValue();
				if(data == nullptr)
				{
					op.CompleteOperation(nullptr);
					return;
				}

				const u32 rows = data->GetHeight();
				const u32 destinationRowByteCount = (rows != 0) ? (readSize / rows) : readSize; // tightly-packed pixel row
				const u32 sourceRowPitch = data->GetRowPitch();
				const u8* const sourceData = (const u8*)data->GetData();
				u8* const destinationData = destination->GetData();

				for(u32 y = 0; y < rows; ++y)
					memcpy(destinationData + (u64)y * destinationRowByteCount, sourceData + (u64)y * sourceRowPitch, destinationRowByteCount);

				op.CompleteOperation(destination);
			});

		gpuContext.SubmitCommandBuffer(readbackCommandBuffer);
	}

	/** 
	 * Decodes a block-compressed surface to RGBA32F on the GPU. Runs inline when already on the render thread, otherwise
	 * marshals the work across. The returned operation completes with the decoded surface, or null on failure. 
	 */
	TAsyncOp<TShared<PixelData>> DecodeToFloatOnGpu(const TShared<PixelData>& compressed)
	{
		TAsyncOp<TShared<PixelData>> op;
		if(compressed == nullptr || GetApplication().GetPrimaryGpuDevice() == nullptr)
		{
			op.CompleteOperation(nullptr);
			return op;
		}

		auto fnGpuWork = [compressed, op]() mutable { DecodeOnRenderThread(compressed, op); };
		if(B3D_CURRENT_THREAD_ID == GetRenderThread().GetThreadId())
			fnGpuWork();
		else
			GetRenderThread().PostCommand(std::move(fnGpuWork), "GPU texture decode");

		return op;
	}

	// ---- PSNR + comparison-image helpers ----

	double PsnrFromError(double squaredError, u64 samples)
	{
		const double mse = squaredError / (double)samples;
		if(mse <= 0.0)
			return 99.0;

		return 10.0 * std::log10(255.0 * 255.0 / mse);
	}

	/**
	 * Writes a [source | gap | decoded] comparison PNG. Both inputs are tightly-packed RGBA8 buffers (width*height*4,
	 * top-left origin). name yields "compress_<name>.png".
	 */
	void WriteSideBySidePNG(const String& name, u32 width, u32 height, const Vector<u8>& sourceRGBA, const Vector<u8>& decodedRGBA)
	{
		const u32 gap = 4;
		const u32 compositeWidth = width * 2 + gap;
		const TShared<PixelData> destination = PixelData::Create(compositeWidth, height, 1, PF_RGBA8);
		u8* const destinationData = destination->GetData();
		const u32 rowPitch = destination->GetRowPitch();

		for(u32 y = 0; y < height; ++y)
		{
			u8* const row = destinationData + y * rowPitch;
			for(u32 x = 0; x < width; ++x)
			{
				const u8* const sourcePixel = &sourceRGBA[(y * width + x) * 4];
				const u8* const decodedPixel = &decodedRGBA[(y * width + x) * 4];

				u8* const sourceOutputPixel = row + x * 4;
				u8* const decodedOutputPixel = row + (width + gap + x) * 4;

				sourceOutputPixel[0] = sourcePixel[0]; sourceOutputPixel[1] = sourcePixel[1]; sourceOutputPixel[2] = sourcePixel[2]; sourceOutputPixel[3] = 255;
				decodedOutputPixel[0] = decodedPixel[0]; decodedOutputPixel[1] = decodedPixel[1]; decodedOutputPixel[2] = decodedPixel[2]; decodedOutputPixel[3] = 255;
			}

			// Magenta separator so the seam is obvious.
			for(u32 g = 0; g < gap; ++g)
			{
				u8* const sep = row + (width + g) * 4;
				sep[0] = 255; sep[1] = 0; sep[2] = 255; sep[3] = 255;
			}
		}

		const Path outPath = Path(String("compress_") + name);
		if(PixelUtility::SaveImage(destination, outPath, ImageFormat::PNG, true))
			B3D_LOG(Info, LogPixelUtility, "Wrote compression comparison image '{0}.png'", name);
	}

	/**
	 * Writes a [source | gap | decoded] comparison OpenEXR. Both inputs are tightly-packed RGB float buffers
	 * (width*height*3, top-left origin) of raw linear HDR values. Used for BC6H so the full dynamic range is preserved.
	 */
	void WriteSideBySideEXR(const String& name, u32 width, u32 height, const Vector<float>& sourceRGB, const Vector<float>& decodedRGB)
	{
		const u32 gap = 4;
		const u32 compositeWidth = width * 2 + gap;
		const TShared<PixelData> destination = PixelData::Create(compositeWidth, height, 1, PF_RGB32F);

		for(u32 y = 0; y < height; ++y)
		{
			for(u32 x = 0; x < width; ++x)
			{
				const float* const sourcePixel = &sourceRGB[(y * width + x) * 3];
				const float* const decodedPixel = &decodedRGB[(y * width + x) * 3];

				destination->SetColorAt(Color(sourcePixel[0], sourcePixel[1], sourcePixel[2], 1.0f), x, y);
				destination->SetColorAt(Color(decodedPixel[0], decodedPixel[1], decodedPixel[2], 1.0f), width + gap + x, y);
			}
			// Magenta separator so the seam is obvious.
			for(u32 g = 0; g < gap; ++g)
				destination->SetColorAt(Color(1.0f, 0.0f, 1.0f, 1.0f), width + g, y);
		}

		const Path outPath = Path(String("compress_") + name);
		if(PixelUtility::SaveImage(destination, outPath, ImageFormat::EXR))
			B3D_LOG(Info, LogPixelUtility, "Wrote compression comparison image '{0}.exr'", name);
	}

	/**
	 * Writes a tonemapped [source | gap | decoded] PNG for an HDR (BC6H) comparison. Both inputs are tightly-packed RGB
	 * float buffers (width*height*3) of raw linear HDR values; each is Reinhard-tonemapped and gamma-encoded to 8-bit so
	 * the result is viewable in any image viewer (the companion .exr keeps the full HDR range). name yields
	 * "compress_<name>_tonemapped.png".
	 */
	void WriteSideBySideTonemappedPNG(const String& name, u32 width, u32 height, const Vector<float>& sourceRGB, const Vector<float>& decodedRGB)
	{
		auto fnTonemap = [](float v) -> u8 {
			v = v > 0.0f ? v : 0.0f;
			v = v / (v + 1.0f); // Reinhard
			v = std::pow(v, 1.0f / 2.2f); // gamma encode for display
			return (u8)Math::Clamp((i32)std::lround(v * 255.0f), 0, 255);
		};

		Vector<u8> sourceTonemapped((u64)width * height * 4);
		Vector<u8> decodedTonemapped((u64)width * height * 4);
		for(u64 pixelIndex = 0; pixelIndex < (u64)width * height; ++pixelIndex)
		{
			sourceTonemapped[pixelIndex * 4 + 0] = fnTonemap(sourceRGB[pixelIndex * 3 + 0]); sourceTonemapped[pixelIndex * 4 + 1] = fnTonemap(sourceRGB[pixelIndex * 3 + 1]);
			sourceTonemapped[pixelIndex * 4 + 2] = fnTonemap(sourceRGB[pixelIndex * 3 + 2]); sourceTonemapped[pixelIndex * 4 + 3] = 255;
			decodedTonemapped[pixelIndex * 4 + 0] = fnTonemap(decodedRGB[pixelIndex * 3 + 0]); decodedTonemapped[pixelIndex * 4 + 1] = fnTonemap(decodedRGB[pixelIndex * 3 + 1]);
			decodedTonemapped[pixelIndex * 4 + 2] = fnTonemap(decodedRGB[pixelIndex * 3 + 2]); decodedTonemapped[pixelIndex * 4 + 3] = 255;
		}

		WriteSideBySidePNG(name + "_tonemapped", width, height, sourceTonemapped, decodedTonemapped);
	}

	/**
	 * CPU-decodes a BCn-compressed surface into display-mapped [source]/[decoded] RGBA8 buffers (tightly packed
	 * width*height*4, top-left origin) ready for WriteSideBySidePNG. The channel mapping matches the PSNR test
	 * (BC4 = grey from red, BC5 = red+green, others = RGB), so source and decoded are compared like-for-like.
	 */
	void BuildCompareSurfaces(PixelFormat format, const PixelData& source, const PixelData& compressed, u32 width, u32 height, Vector<u8>& outSource, Vector<u8>& outDecoded)
	{
		const u32 blocksX = width / 4;
		const u32 blocksY = height / 4;
		const u32 blockBytes = (format == PF_BC1 || format == PF_BC4) ? 8u : 16u;
		const u8* const blocks = compressed.GetData();

		outSource.assign((u64)width * height * 4, 0);
		outDecoded.assign((u64)width * height * 4, 0);

		for(u32 blockY = 0; blockY < blocksY; ++blockY)
		{
			for(u32 blockX = 0; blockX < blocksX; ++blockX)
			{
				const u8* const block = blocks + (blockY * blocksX + blockX) * blockBytes;

				u8 decodedRGBA[64];
				u8 decodedRed[16];
				u8 decodedGreen[16];
				if(format == PF_BC1)
					DecodeBC1(block, false, decodedRGBA);
				else if(format == PF_BC3)
				{
					DecodeBC4(block, decodedRed);
					DecodeBC1(block + 8, true, decodedRGBA);
				}
				else if(format == PF_BC4)
					DecodeBC4(block, decodedRed);
				else if(format == PF_BC7)
					DecodeBC7(block, decodedRGBA);
				else // BC5
				{
					DecodeBC4(block, decodedRed);
					DecodeBC4(block + 8, decodedGreen);
				}

				for(u32 i = 0; i < 16; ++i)
				{
					const u32 pixelX = blockX * 4 + (i % 4);
					const u32 pixelY = blockY * 4 + (i / 4);

					const Color sourceColor = source.GetColorAt(pixelX, pixelY);
					const u8 sourceComponent0 = (u8)Math::Clamp((i32)std::lround(sourceColor.R * 255.0f), 0, 255);
					const u8 sourceComponent1 = (u8)Math::Clamp((i32)std::lround(sourceColor.G * 255.0f), 0, 255);
					const u8 sourceComponent2 = (u8)Math::Clamp((i32)std::lround(sourceColor.B * 255.0f), 0, 255);

					u8 sourceRedComponent, sourceGreenComponent, sourceBlueComponent;
					u8 decodedRedComponent, decodedGreenComponent, decodedBlueComponent;
					if(format == PF_BC4)
					{
						sourceRedComponent = sourceGreenComponent = sourceBlueComponent = sourceComponent0;
						decodedRedComponent = decodedGreenComponent = decodedBlueComponent = decodedRed[i];
					}
					else if(format == PF_BC5)
					{
						sourceRedComponent = sourceComponent0;
						sourceGreenComponent = sourceComponent1;
						sourceBlueComponent = 0;

						decodedRedComponent = decodedRed[i];
						decodedGreenComponent = decodedGreen[i];
						decodedBlueComponent = 0;
					}
					else
					{
						sourceRedComponent = sourceComponent0;
						sourceGreenComponent = sourceComponent1;
						sourceBlueComponent = sourceComponent2;
						decodedRedComponent = decodedRGBA[i * 4 + 0];
						decodedGreenComponent = decodedRGBA[i * 4 + 1];
						decodedBlueComponent = decodedRGBA[i * 4 + 2];
					}

					u8* const outSourcePixel = &outSource[((u64)pixelY * width + pixelX) * 4]; 
					outSourcePixel[0] = sourceRedComponent; outSourcePixel[1] = sourceGreenComponent; outSourcePixel[2] = sourceBlueComponent; outSourcePixel[3] = 255;

					u8* const outDecodedPixel = &outDecoded[((u64)pixelY * width + pixelX) * 4]; 
					outDecodedPixel[0] = decodedRedComponent; outDecodedPixel[1] = decodedGreenComponent; outDecodedPixel[2] = decodedBlueComponent; outDecodedPixel[3] = 255;
				}
			}
		}
	}

	/** CPU-decodes a BCn-compressed surface and returns PSNR (dB) vs the source over the channels the format actually carries (BC1 = RGB, BC3 = RGBA, BC4 = R, BC5 = RG, BC7 = RGBA). */
	double ComputeCompressedPsnr(PixelFormat format, const PixelData& source, const PixelData& compressed, u32 width, u32 height)
	{
		const u32 blocksX = width / 4;
		const u32 blocksY = height / 4;
		const u32 blockBytes = (format == PF_BC1 || format == PF_BC4) ? 8u : 16u;
		const u8* const blocks = compressed.GetData();

		double error = 0.0;
		u64 samples = 0;

		for(u32 blockY = 0; blockY < blocksY; ++blockY)
		{
			for(u32 blockX = 0; blockX < blocksX; ++blockX)
			{
				const u8* const block = blocks + (blockY * blocksX + blockX) * blockBytes;

				u8 decodedRGBA[64];
				u8 decodedRed[16];
				u8 decodedGreen[16];
				if(format == PF_BC1)
					DecodeBC1(block, false, decodedRGBA);
				else if(format == PF_BC3)
				{
					DecodeBC4(block, decodedRed);
					DecodeBC1(block + 8, true, decodedRGBA);
				}
				else if(format == PF_BC4)
					DecodeBC4(block, decodedRed);
				else if(format == PF_BC7)
					DecodeBC7(block, decodedRGBA);
				else // BC5
				{
					DecodeBC4(block, decodedRed);
					DecodeBC4(block + 8, decodedGreen);
				}

				for(u32 i = 0; i < 16; ++i)
				{
					const u32 pixelX = blockX * 4 + (i % 4);
					const u32 pixelY = blockY * 4 + (i / 4);
					const Color color = source.GetColorAt(pixelX, pixelY);
					const double sourceComponent0 = (double)Math::Clamp((i32)std::lround(color.R * 255.0f), 0, 255);
					const double sourceComponent1 = (double)Math::Clamp((i32)std::lround(color.G * 255.0f), 0, 255);
					const double sourceComponent2 = (double)Math::Clamp((i32)std::lround(color.B * 255.0f), 0, 255);
					const double sourceComponent3 = (double)Math::Clamp((i32)std::lround(color.A * 255.0f), 0, 255);

					if(format == PF_BC1)
					{
						const double decodedRedComponent = sourceComponent0 - decodedRGBA[i * 4 + 0];
						const double decodedGreenComponent = sourceComponent1 - decodedRGBA[i * 4 + 1];
						const double decodedBlueComponent = sourceComponent2 - decodedRGBA[i * 4 + 2];

						error += decodedRedComponent * decodedRedComponent + decodedGreenComponent * decodedGreenComponent + decodedBlueComponent * decodedBlueComponent; 
						samples += 3;
					}
					else if(format == PF_BC3)
					{
						const double decodedRedComponent = sourceComponent0 - decodedRGBA[i * 4 + 0];
						const double decodedGreenComponent = sourceComponent1 - decodedRGBA[i * 4 + 1];
						const double decodedBlueComponent = sourceComponent2 - decodedRGBA[i * 4 + 2];
						const double decodedAlphaComponent = sourceComponent3 - decodedRed[i];

						error += decodedRedComponent * decodedRedComponent + decodedGreenComponent * decodedGreenComponent + decodedBlueComponent * decodedBlueComponent + decodedAlphaComponent * decodedAlphaComponent; 
						samples += 4;
					}
					else if(format == PF_BC4)
					{
						const double decodedRedComponent = sourceComponent0 - decodedRed[i]; 
						
						error += decodedRedComponent * decodedRedComponent; 
						samples += 1;
					}
					else if(format == PF_BC7)
					{
						const double decodedRedComponent = sourceComponent0 - decodedRGBA[i * 4 + 0];
						const double decodedGreenComponent = sourceComponent1 - decodedRGBA[i * 4 + 1];
						const double decodedBlueComponent = sourceComponent2 - decodedRGBA[i * 4 + 2];
						const double decodedAlphaComponent = sourceComponent3 - decodedRGBA[i * 4 + 3];

						error += decodedRedComponent * decodedRedComponent + decodedGreenComponent * decodedGreenComponent + decodedBlueComponent * decodedBlueComponent + decodedAlphaComponent * decodedAlphaComponent; samples += 4;
					}
					else // BC5
					{
						const double decodedRedComponent = sourceComponent0 - decodedRed[i];
						const double decodedGreenComponent = sourceComponent1 - decodedGreen[i];

						error += decodedRedComponent * decodedRedComponent + decodedGreenComponent * decodedGreenComponent; 
						samples += 2;
					}
				}
			}
		}

		return PsnrFromError(error, samples);
	}

	// ---- Optional file dumps ----

	/**
	 * Writes a raw byte buffer to fileName in the working directory. Used for the optional block/source dumps that let an
	 * external reference decoder re-decode the engine's exact bytes.
	 */
	void DumpRawBuffer(const String& fileName, const void* data, size_t byteCount)
	{
		const TShared<DataStream> stream = FileSystem::CreateAndOpenFile(Path(fileName), FileAccessFlag::Write);
		if(stream != nullptr)
			stream->Write(data, byteCount);
	}
} // anonymous namespace

ImporterTestSuite::ImporterTestSuite()
	: TestSuite("ImporterTestSuite")
{
	B3D_ADD_TEST(ImporterTestSuite::TestPngImport_Default)
	B3D_ADD_TEST(ImporterTestSuite::TestPngImport_WithMips)
	B3D_ADD_TEST(ImporterTestSuite::TestPngImport_BC3)
	B3D_ADD_TEST(ImporterTestSuite::TestGpuCompress_Psnr)
	B3D_ADD_TEST(ImporterTestSuite::TestGpuCompress_BC6H_Psnr)
	B3D_ADD_TEST(ImporterTestSuite::TestGpuCompress_Perf)
	B3D_ADD_TEST(ImporterTestSuite::TestGpuMipmaps_BoxFilter)
	B3D_ADD_TEST(ImporterTestSuite::TestOggImport_Default)
	B3D_ADD_TEST(ImporterTestSuite::TestOggImport_KeepCompressed)
	B3D_ADD_TEST(ImporterTestSuite::TestFlacImport_Default)
}

void ImporterTestSuite::StartUp()
{
	const Path dataFolder = Path::Combine(BuiltinResources::GetUnitTestDataFolder(), "ImporterTests");
	mImagePath = Path::Combine(dataFolder, "TestImage.png");
	mOggPath = Path::Combine(dataFolder, "TestAudio.ogg");
	mFlacPath = Path::Combine(dataFolder, "TestAudio.flac");
}

void ImporterTestSuite::TestPngImport_Default()
{
	TShared<TextureImportOptions> options = TextureImportOptions::Create();

	HTexture texture = GetImporter().Import<Texture>(mImagePath, options);
	B3D_TEST_ASSERT(texture != nullptr)
	B3D_TEST_ASSERT(texture.IsLoaded())

	const TextureProperties& properties = texture->GetProperties();
	B3D_TEST_ASSERT(properties.Format == PF_RGBA8)
	B3D_TEST_ASSERT(properties.Width == 64)
	B3D_TEST_ASSERT(properties.Height == 64)
	B3D_TEST_ASSERT(properties.MipMapCount == 0)
}

void ImporterTestSuite::TestPngImport_WithMips()
{
	TShared<TextureImportOptions> options = TextureImportOptions::Create();
	options->GenerateMips = true;

	HTexture texture = GetImporter().Import<Texture>(mImagePath, options);
	B3D_TEST_ASSERT(texture != nullptr)
	B3D_TEST_ASSERT(texture.IsLoaded())

	const TextureProperties& properties = texture->GetProperties();
	B3D_TEST_ASSERT(properties.Format == PF_RGBA8)
	B3D_TEST_ASSERT(properties.Width == 64)
	B3D_TEST_ASSERT(properties.Height == 64)
	B3D_TEST_ASSERT(properties.MipMapCount == 6)
}

void ImporterTestSuite::TestPngImport_BC3()
{
	TShared<TextureImportOptions> options = TextureImportOptions::Create();
	options->GenerateMips = true;
	options->Format = PF_BC3;

	HTexture texture = GetImporter().Import<Texture>(mImagePath, options);
	B3D_TEST_ASSERT(texture != nullptr)
	B3D_TEST_ASSERT(texture.IsLoaded())

	const TextureProperties& properties = texture->GetProperties();
	B3D_TEST_ASSERT(properties.Format == PF_BC3)
	B3D_TEST_ASSERT(properties.Width == 64)
	B3D_TEST_ASSERT(properties.Height == 64)
	B3D_TEST_ASSERT(properties.MipMapCount == 6)
}

void ImporterTestSuite::TestGpuCompress_Psnr()
{
	// GPU compression needs a real backend; skip on a headless / Null backend so the suite stays green on machines
	// without a usable GPU.
	const TShared<GpuDevice> device = GetApplication().GetPrimaryGpuDevice();
	if(device == nullptr || device->GetCapabilities().BackendName == "Null")
		return;

	// Load the source as RGBA8. --test.ldrImage points the test at a larger external texture (e.g. Cerberus_A.tga) to
	// compare encoders on real content; its dimensions are floored to a multiple of 4 via blocksX/blocksY below.
	TShared<TextureImportOptions> options = TextureImportOptions::Create();
	const Path ldrImage = GetImagePathParameter("test.ldrImage");
	const Path imagePath = ldrImage.IsEmpty() ? mImagePath : ldrImage;
	HTexture texture = GetImporter().Import<Texture>(imagePath, options);
	B3D_TEST_ASSERT(texture != nullptr)
	B3D_TEST_ASSERT(texture.IsLoaded())

	TAsyncOp<TShared<PixelData>> readOp = texture->ReadData(0, 0);
	readOp.BlockUntilComplete();
	TShared<PixelData> source = readOp.GetReturnValue();
	B3D_TEST_ASSERT(source != nullptr)

	const u32 width = source->GetWidth();
	const u32 height = source->GetHeight();
	const u32 blocksX = width / 4;
	const u32 blocksY = height / 4;

	// Cache source pixels as RGBA8 for comparison.
	Vector<u8> convertedSource(width * height * 4);
	for(u32 y = 0; y < height; ++y)
	{
		for(u32 x = 0; x < width; ++x)
		{
			const Color sourceColor = source->GetColorAt(x, y);
			u8* convertedColor = &convertedSource[(y * width + x) * 4];
			convertedColor[0] = (u8)Math::Clamp((i32)std::lround(sourceColor.R * 255.0f), 0, 255);
			convertedColor[1] = (u8)Math::Clamp((i32)std::lround(sourceColor.G * 255.0f), 0, 255);
			convertedColor[2] = (u8)Math::Clamp((i32)std::lround(sourceColor.B * 255.0f), 0, 255);
			convertedColor[3] = (u8)Math::Clamp((i32)std::lround(sourceColor.A * 255.0f), 0, 255);
		}
	}

	struct Case { PixelFormat Format; const char* Name; u32 BlockBytes; };
	const Case cases[] = {
		{ PF_BC1, "BC1", 8 },
		{ PF_BC3, "BC3", 16 },
		{ PF_BC4, "BC4", 8 },
		{ PF_BC5, "BC5", 16 },
		{ PF_BC7, "BC7", 16 },
	};

	// When dumping, write the exact source bytes the engine compresses against so an external reference decoder can be
	// run on identical input (isolating a source-decode difference from a codec difference).
	if(gDumpCompressedImages)
	{
		DumpRawBuffer("source_rgba.bin", convertedSource.data(), convertedSource.size());
		B3D_LOG(Info, LogPixelUtility, "Wrote engine source RGBA8 to source_rgba.bin ({0}x{1})", width, height);
	}

	for(const Case& testCase : cases)
	{
		TShared<PixelData> compressed = PixelData::Create(width, height, 1, testCase.Format);
		CompressionOptions compressionOptions;
		compressionOptions.Format = testCase.Format;

		TAsyncOp<TShared<PixelData>> compressOp = GpuTextureCompressor::Compress(source, compressed, compressionOptions);
		compressOp.BlockUntilComplete();

		const bool compressOk = compressOp.GetReturnValue() != nullptr;
		B3D_TEST_ASSERT(compressOk)

		// CPU reference PSNR over the channels this format carries.
		const double psnr = ComputeCompressedPsnr(testCase.Format, *source, *compressed, width, height);
		B3D_LOG(Info, LogPixelUtility, "GPU compression {0}: PSNR = {1} dB", testCase.Name, psnr);

		// Cross-check against the GPU hardware decode of the same blocks (the authoritative ground truth). LDR BCn decode
		// is exact integer math, so the hardware and the CPU reference decoder must agree to within float-readback
		// rounding. A large gap means either the CPU decoder is biased (so every PSNR reported from it is wrong) or the
		// backend maps the format incorrectly (this is the assert that catches the BC4-as-SNORM class of bug). The 2 dB
		// tolerance is tight enough to catch such bugs yet never trips on rounding (observed agreement <0.35 dB).
		{
			TAsyncOp<TShared<PixelData>> hardwareDecodeOperation = DecodeToFloatOnGpu(compressed);
			hardwareDecodeOperation.BlockUntilComplete();

			const TShared<PixelData> hardwareDecodedPixels = hardwareDecodeOperation.GetReturnValue();
			if(hardwareDecodedPixels != nullptr)
			{
				double hwError = 0.0;
				u64 hwSamples = 0;
				for(u32 blockY = 0; blockY < blocksY; ++blockY)
				{
					for(u32 blockX = 0; blockX < blocksX; ++blockX)
					{
						for(u32 i = 0; i < 16; ++i)
						{
							const u32 pixelX = blockX * 4 + (i % 4);
							const u32 pixelY = blockY * 4 + (i / 4);

							const u8* sourcePixel = &convertedSource[(pixelY * width + pixelX) * 4];
							const Color decodedPixel = hardwareDecodedPixels->GetColorAt(pixelX, pixelY);

							const double convertedDecodedRed = (double)std::lround(decodedPixel.R * 255.0f);
							const double convertedDecodedGreen = (double)std::lround(decodedPixel.G * 255.0f);
							const double convertedDecodedBlue = (double)std::lround(decodedPixel.B * 255.0f);
							const double convertedDecodedAlpha = (double)std::lround(decodedPixel.A * 255.0f);

							if(testCase.Format == PF_BC4)
							{
								double deltaRed = sourcePixel[0] - convertedDecodedRed;

								hwError += deltaRed * deltaRed;
								hwSamples += 1;
							}
							else if(testCase.Format == PF_BC5)
							{
								double deltaRed = sourcePixel[0] - convertedDecodedRed;
								double deltaGreen = sourcePixel[1] - convertedDecodedGreen;

								hwError += deltaRed * deltaRed + deltaGreen * deltaGreen;
								hwSamples += 2;
							}
							else if(testCase.Format == PF_BC1)
							{
								double deltaRed = sourcePixel[0] - convertedDecodedRed;
								double deltaGreen = sourcePixel[1] - convertedDecodedGreen;
								double deltaBlue = sourcePixel[2] - convertedDecodedBlue;

								hwError += deltaRed * deltaRed + deltaGreen * deltaGreen + deltaBlue * deltaBlue;
								hwSamples += 3;
							}
							else
							{
								double deltaRed = sourcePixel[0] - convertedDecodedRed;
								double deltaGreen = sourcePixel[1] - convertedDecodedGreen;
								double deltaBlue = sourcePixel[2] - convertedDecodedBlue;
								double deltaAlpha = sourcePixel[3] - convertedDecodedAlpha;

								hwError += deltaRed * deltaRed + deltaGreen * deltaGreen + deltaBlue * deltaBlue + deltaAlpha * deltaAlpha;
								hwSamples += 4;
							}
						}
					}
				}

				const double hwPsnr = PsnrFromError(hwError, hwSamples);
				B3D_LOG(Info, LogPixelUtility, "GPU compression {0}: HW-decoded PSNR = {1} dB (CPU-decoder PSNR = {2}; delta {3})", testCase.Name, hwPsnr, psnr, psnr - hwPsnr);
				B3D_TEST_ASSERT(std::fabs(psnr - hwPsnr) < 2.0)
			}
			else
				B3D_LOG(Warning, LogPixelUtility, "GPU compression {0}: hardware decode failed (DecodeToFloatOnGpu returned null)", testCase.Name);
		}

		// Sanity floor: a working encoder is far above this; random/garbage blocks would be ~8-10 dB.
		B3D_TEST_ASSERT(psnr > 15.0)

		// Optional dumps: a [source | decoded] comparison PNG, plus raw BC3/BC7 blocks for an external reference decoder
		// run on the identical source bytes written above.
		if(gDumpCompressedImages)
		{
			Vector<u8> sourceForDisplay, decodedForDisplay;
			BuildCompareSurfaces(testCase.Format, *source, *compressed, width, height, sourceForDisplay, decodedForDisplay);
			WriteSideBySidePNG(testCase.Name, width, height, sourceForDisplay, decodedForDisplay);

			if(testCase.Format == PF_BC3 || testCase.Format == PF_BC7)
			{
				const String fileName = String("compress_") + testCase.Name + ".bin";
				DumpRawBuffer(fileName, compressed->GetData(), (size_t)blocksX * blocksY * 16);
				B3D_LOG(Info, LogPixelUtility, "Wrote our {0} blocks to {1} ({2}x{3} blocks)", testCase.Name, fileName, blocksX, blocksY);
			}
		}
	}
}

void ImporterTestSuite::TestGpuCompress_BC6H_Psnr()
{
	// GPU compression needs a real backend; skip on a headless / Null backend.
	const TShared<GpuDevice> device = GetApplication().GetPrimaryGpuDevice();
	if(device == nullptr || device->GetCapabilities().BackendName == "Null")
		return;

	// Source selection. By default a synthesized smooth HDR gradient (values 0..8, beyond the LDR range) keeps the suite
	// self-contained. --test.bc6hImage points the test at a real HDR file instead, imported as full-float RGBA32F (the
	// default PF_RGBA8 would clamp the HDR range away); the native resolution is preserved for a like-for-like compare.
	const Path bc6hImage = GetImagePathParameter("test.bc6hImage");
	const bool fromFile = !bc6hImage.IsEmpty();

	TShared<PixelData> source;
	u32 width, height;
	if(fromFile)
	{
		TShared<TextureImportOptions> options = TextureImportOptions::Create();
		options->Format = PF_RGBA32F;

		HTexture texture = GetImporter().Import<Texture>(bc6hImage, options);
		B3D_TEST_ASSERT(texture != nullptr)
		B3D_TEST_ASSERT(texture.IsLoaded())

		TAsyncOp<TShared<PixelData>> readOp = texture->ReadData(0, 0);
		readOp.BlockUntilComplete();

		source = readOp.GetReturnValue();
		B3D_TEST_ASSERT(source != nullptr)

		width = source->GetWidth();
		height = source->GetHeight();
		B3D_LOG(Info, LogPixelUtility, "GPU compression BC6H: source {0}x{1} from '{2}'", width, height, bc6hImage.ToString());
	}
	else
	{
		width = height = 64;
		source = PixelData::Create(width, height, 1, PF_RGBA32F);
		for(u32 y = 0; y < height; ++y)
		{
			for(u32 x = 0; x < width; ++x)
			{
				Color color;
				color.R = (float)x / (float)(width - 1) * 4.0f;
				color.G = (float)y / (float)(height - 1) * 6.0f;
				color.B = (float)(x + y) / (float)(2 * (width - 1)) * 8.0f;
				color.A = 1.0f;

				source->SetColorAt(color, x, y);
			}
		}
	}

	TShared<PixelData> compressed = PixelData::Create(width, height, 1, PF_BC6H);
	CompressionOptions compressionOptions;
	compressionOptions.Format = PF_BC6H;

	TAsyncOp<TShared<PixelData>> compressOp = GpuTextureCompressor::Compress(source, compressed, compressionOptions);
	compressOp.BlockUntilComplete();

	const bool compressOk = compressOp.GetReturnValue() != nullptr;
	B3D_TEST_ASSERT(compressOk)

	const u32 blocksX = width / 4, blocksY = height / 4;
	const u8* blocks = compressed->GetData();
	double error = 0.0, peak = 0.0;
	u64 samples = 0;

	// Diagnostics to localize HDR range loss: the brightest source sample and what the CPU decoder reconstructs there,
	// plus the max decoded magnitude and a gross-overshoot count.
	double maxSourceChannel = -1.0, sourceAtMax = 0.0, decodedAtMax = 0.0, maxDecoded = 0.0;
	u64 grossOvershoot = 0;

	// Raw HDR display surfaces (RGB float), filled only when dumping a comparison image.
	Vector<float> sourceForDisplay, decodedForDisplay;
	if(gDumpCompressedImages)
	{
		sourceForDisplay.resize((u64)width * height * 3);
		decodedForDisplay.resize((u64)width * height * 3);
	}

	for(u32 blockY = 0; blockY < blocksY; ++blockY)
	{
		for(u32 blockX = 0; blockX < blocksX; ++blockX)
		{
			float decodedBlockPixels[16][3];
			DecodeBC6H_UF16(blocks + ((u64)blockY * blocksX + blockX) * 16, decodedBlockPixels);

			for(u32 i = 0; i < 16; ++i)
			{
				const u32 pixelX = blockX * 4 + (i % 4);
				const u32 pixelY = blockY * 4 + (i / 4);

				const Color sourceColor = source->GetColorAt(pixelX, pixelY);
				const float sourceComponents[3] = { sourceColor.R, sourceColor.G, sourceColor.B };
				for(i32 channel = 0; channel < 3; ++channel)
				{
					const double delta = (double)sourceComponents[channel] - (double)decodedBlockPixels[i][channel];
					error += delta * delta;

					peak = std::max(peak, (double)sourceComponents[channel]);
					++samples;

					maxDecoded = std::max(maxDecoded, (double)decodedBlockPixels[i][channel]);
					if((double)decodedBlockPixels[i][channel] > 1000.0)
						++grossOvershoot;

					if((double)sourceComponents[channel] > maxSourceChannel)
					{
						maxSourceChannel = sourceComponents[channel];
						sourceAtMax = sourceComponents[channel];
						decodedAtMax = decodedBlockPixels[i][channel];
					}
				}

				if(gDumpCompressedImages)
				{
					float* const sourceOutputPixel = &sourceForDisplay[((u64)pixelY * width + pixelX) * 3];
					sourceOutputPixel[0] = sourceComponents[0];
					sourceOutputPixel[1] = sourceComponents[1];
					sourceOutputPixel[2] = sourceComponents[2];

					float* const decodedOutputPixel = &decodedForDisplay[((u64)pixelY * width + pixelX) * 3];
					decodedOutputPixel[0] = decodedBlockPixels[i][0];
					decodedOutputPixel[1] = decodedBlockPixels[i][1];
					decodedOutputPixel[2] = decodedBlockPixels[i][2];
				}
			}
		}
	}

	const double mse = error / (double)samples;
	const double psnr = (mse <= 0.0) ? 99.0 : 10.0 * std::log10(peak * peak / mse);
	B3D_LOG(Info, LogPixelUtility, "GPU compression BC6H: PSNR = {0} dB (peak {1})", psnr, peak);
	B3D_LOG(Info, LogPixelUtility, "GPU compression BC6H: brightest src sample {0} -> CPU-decoded {1}; maxDecoded {2}; grossOvershoot(>1000) {3}/{4}",
		sourceAtMax, decodedAtMax, maxDecoded, grossOvershoot, samples);

	// Cross-check against the GPU hardware decode of the same blocks (the decode path used at runtime when the texture is
	// sampled), measured over the same block-aligned region with the same peak. This is the authoritative tie-breaker
	// between the CPU reference decoder and any other reference decoder for BC6H's transformed high-base modes.
	{
		TAsyncOp<TShared<PixelData>> hardwareDecodeOperation = DecodeToFloatOnGpu(compressed);
		hardwareDecodeOperation.BlockUntilComplete();

		const TShared<PixelData> hardwareDecodedPixels = hardwareDecodeOperation.GetReturnValue();
		if(hardwareDecodedPixels != nullptr)
		{
			double hardwareError = 0.0, hardwareMaxDecoded = 0.0, hardwareDecodedAtMax = 0.0, hardwareMaxSourceChannel = -1.0;
			u64 hardwareSamples = 0, hardwareGrossOvershoot = 0;
			for(u32 pixelY = 0; pixelY < blocksY * 4; ++pixelY)
			{
				for(u32 pixelX = 0; pixelX < blocksX * 4; ++pixelX)
				{
					const Color sourceColor = source->GetColorAt(pixelX, pixelY);
					const Color decodedColor = hardwareDecodedPixels->GetColorAt(pixelX, pixelY);

					const float sourceComponents[3] = { sourceColor.R, sourceColor.G, sourceColor.B };
					const float decodedComponents[3] = { decodedColor.R, decodedColor.G, decodedColor.B };

					for(i32 channel = 0; channel < 3; ++channel)
					{
						const double diff = (double)sourceComponents[channel] - (double)decodedComponents[channel];
						hardwareError += diff * diff;

						++hardwareSamples;
						hardwareMaxDecoded = std::max(hardwareMaxDecoded, (double)decodedComponents[channel]);

						if((double)decodedComponents[channel] > 1000.0)
							++hardwareGrossOvershoot;

						if((double)sourceComponents[channel] > hardwareMaxSourceChannel)
						{
							hardwareMaxSourceChannel = sourceComponents[channel];
							hardwareDecodedAtMax = decodedComponents[channel];
						}
					}
				}
			}

			const double hardwareMse = hardwareError / (double)hardwareSamples;
			const double hardwarePsnr = (hardwareMse <= 0.0) ? 99.0 : 10.0 * std::log10(peak * peak / hardwareMse);
			B3D_LOG(Info, LogPixelUtility, "GPU compression BC6H: HW-decoded PSNR = {0} dB (peak {1}); brightest src {2} -> HW-decoded {3}; maxDecoded {4}; grossOvershoot(>1000) {5}/{6}",
				hardwarePsnr, peak, hardwareMaxSourceChannel, hardwareDecodedAtMax, hardwareMaxDecoded, hardwareGrossOvershoot, hardwareSamples);

			// A correct CPU reference decoder must agree with the real GPU decoder to within decode rounding. A large gap
			// means the CPU decode model - shared by the encoder's cost function - has drifted from the spec/hardware, so
			// any CPU-measured PSNR is an illusion. This is the regression guard for the decoder-divergence class of bug
			// (e.g. a per-mode bit-order error); the two decoders agree to <0.01 dB when the model is correct.
			B3D_TEST_ASSERT(std::abs(hardwarePsnr - psnr) < 2.0)

			// Localize any CPU-vs-hardware divergence to a specific BC6H mode: bucket large per-texel divergences by the
			// block's mode prefix (the 2- or 5-bit lead). The mode(s) that light up are where the CPU decode model departs
			// from the spec/hardware.
			u64 divergeByLead[32] = {}, blocksByLead[32] = {};
			double maxDivergeByLead[32] = {};

			for(u32 blockY = 0; blockY < blocksY; ++blockY)
			{
				for(u32 blockX = 0; blockX < blocksX; ++blockX)
				{
					const u8* block = blocks + ((u64)blockY * blocksX + blockX) * 16;
					const u32 lead = (block[0] & 0x02) ? (u32)(block[0] & 0x1F) : (u32)(block[0] & 0x01);

					float decodedBlockPixels[16][3];
					DecodeBC6H_UF16(block, decodedBlockPixels);

					blocksByLead[lead & 31]++;
					for(u32 i = 0; i < 16; ++i)
					{
						const u32 pixelX = blockX * 4 + (i % 4);
						const u32 pixelY = blockY * 4 + (i / 4);
						const Color decodedPixel = hardwareDecodedPixels->GetColorAt(pixelX, pixelY);
						const float decodedPixelComponents[3] = { decodedPixel.R, decodedPixel.G, decodedPixel.B };
						for(i32 ch = 0; ch < 3; ++ch)
						{
							const double diff = std::abs((double)decodedPixelComponents[ch] - (double)decodedBlockPixels[i][ch]);
							if(diff > 100.0)
							{
								divergeByLead[lead & 31]++;
								maxDivergeByLead[lead & 31] = std::max(maxDivergeByLead[lead & 31], diff);
							}
						}
					}
				}
			}

			for(u32 l = 0; l < 32; ++l)
			{
				if(divergeByLead[l] > 0)
				{
					B3D_LOG(Info, LogPixelUtility, "GPU compression BC6H: mode-lead 0x{0} -> {1} CPU-vs-HW divergences (>100; max {2}) across {3} blocks of this lead",
						l, divergeByLead[l], maxDivergeByLead[l], blocksByLead[l]);
				}
			}
		}
		else
			B3D_LOG(Warning, LogPixelUtility, "GPU compression BC6H: hardware decode failed (DecodeToFloatOnGpu returned null)");
	}

	if(gDumpCompressedImages)
	{
		WriteSideBySideEXR("BC6H", width, height, sourceForDisplay, decodedForDisplay);
		WriteSideBySideTonemappedPNG("BC6H", width, height, sourceForDisplay, decodedForDisplay);

		DumpRawBuffer("compress_BC6H.bin", blocks, (size_t)((u64)blocksX * blocksY * 16));
		B3D_LOG(Info, LogPixelUtility, "Wrote raw BC6H block buffer 'compress_BC6H.bin' ({0} blocks)", (u64)blocksX * blocksY);
	}

	// A working one-region BC6H encoder on a smooth gradient is far above 30 dB; real HDR content varies more, so the
	// file path only enforces a sanity floor (random/garbage blocks land ~8-10 dB) and relies on the logged number.
	B3D_TEST_ASSERT(psnr > (fromFile ? 15.0 : 30.0))
}

void ImporterTestSuite::TestGpuCompress_Perf()
{
	// Large-texture throughput / memory check. Runs only when --test.perfImage is set to an image path (e.g. the
	// 4096x4096 Cerberus_A.tga), so the suite stays green on machines without the large source asset. Measures the full
	// import-style round trip (CPU->GPU upload + compute dispatch(es) + GPU->CPU readback) on a warm pipeline.
	const Path perfImage = GetImagePathParameter("test.perfImage");
	if(perfImage.IsEmpty())
		return;

	const TShared<GpuDevice> device = GetApplication().GetPrimaryGpuDevice();
	if(device == nullptr || device->GetCapabilities().BackendName == "Null")
		return;

	TShared<TextureImportOptions> options = TextureImportOptions::Create();
	HTexture texture = GetImporter().Import<Texture>(perfImage, options);
	B3D_TEST_ASSERT(texture != nullptr)
	B3D_TEST_ASSERT(texture.IsLoaded())

	TAsyncOp<TShared<PixelData>> readOp = texture->ReadData(0, 0);
	readOp.BlockUntilComplete();
	TShared<PixelData> source = readOp.GetReturnValue();
	B3D_TEST_ASSERT(source != nullptr)

	const u32 width = source->GetWidth();
	const u32 height = source->GetHeight();
	const double megapixels = (double)width * (double)height / 1.0e6;
	B3D_LOG(Info, LogPixelUtility, "GPU compress perf: source {0}x{1} ({2} Mpix) from '{3}'", width, height, megapixels, perfImage.ToString());

	struct Case { PixelFormat Format; const char* Name; };
	const Case cases[] = {
		{ PF_BC1, "BC1" },
		{ PF_BC3, "BC3" },
		{ PF_BC4, "BC4" },
		{ PF_BC5, "BC5" },
		{ PF_BC7, "BC7" },
	};

	const u32 kIterationCount = 3;

	for(const Case& testCase : cases)
	{
		TShared<PixelData> compressed = PixelData::Create(width, height, 1, testCase.Format);
		CompressionOptions compressionOptions;
		compressionOptions.Format = testCase.Format;

		// Warm-up: the first call pays the one-time pipeline compile per FORMAT variation; excluded from timing.
		TAsyncOp<TShared<PixelData>> warmOp = GpuTextureCompressor::Compress(source, compressed, compressionOptions);
		warmOp.BlockUntilComplete();

		const bool warmOk = warmOp.GetReturnValue() != nullptr;
		B3D_TEST_ASSERT(warmOk)

		double bestMs = 1.0e30;
		double sumMs = 0.0;
		Timer timer;
		for(u32 iteration = 0; iteration < kIterationCount; ++iteration)
		{
			timer.Reset();
			TAsyncOp<TShared<PixelData>> compressOp = GpuTextureCompressor::Compress(source, compressed, compressionOptions);
			compressOp.BlockUntilComplete(); // measure the full round-trip (dispatch + async read-back)

			const bool ok = compressOp.GetReturnValue() != nullptr;
			const double ms = timer.GetMicroseconds() / 1000.0;
			B3D_TEST_ASSERT(ok)

			if(ms < bestMs)
				bestMs = ms;

			sumMs += ms;
		}

		const double meanMs = sumMs / (double)kIterationCount;
		const double throughput = bestMs > 0.0 ? megapixels / (bestMs / 1000.0) : 0.0;
		B3D_LOG(Info, LogPixelUtility, "GPU compress perf {0}: best {1} ms, mean {2} ms, {3} Mpix/s (warm, full round-trip)",
			testCase.Name, bestMs, meanMs, throughput);

		// Quality of the encoder on this image, comparable to an external per-format PSNR.
		const double psnr = ComputeCompressedPsnr(testCase.Format, *source, *compressed, width, height);
		B3D_LOG(Info, LogPixelUtility, "GPU compress perf {0}: PSNR = {1} dB", testCase.Name, psnr);

		// Optional visual check: a [source | decoded] PNG written to the working directory (heavy at 4K, off by default).
		if(gDumpCompressedImages)
		{
			Vector<u8> sourceForDisplay, decodedForDisplay;
			BuildCompareSurfaces(testCase.Format, *source, *compressed, width, height, sourceForDisplay, decodedForDisplay);
			WriteSideBySidePNG(testCase.Name, width, height, sourceForDisplay, decodedForDisplay);
		}
	}
}

void ImporterTestSuite::TestGpuMipmaps_BoxFilter()
{
	// GPU mip generation needs a real backend; skip on a headless / Null backend.
	const TShared<GpuDevice> device = GetApplication().GetPrimaryGpuDevice();
	if(device == nullptr || device->GetCapabilities().BackendName == "Null")
		return;

	// Build a deterministic 4x4 RGBA8 source so the expected box averages are known.
	const u32 size = 4;
	TShared<PixelData> source = PixelData::Create(size, size, 1, PF_RGBA8);
	for(u32 y = 0; y < size; ++y)
	{
		for(u32 x = 0; x < size; ++x)
		{
			Color color;
			color.R = (float)((x * 37 + y * 11) % 256) / 255.0f;
			color.G = (float)((x * 17 + y * 53) % 256) / 255.0f;
			color.B = (float)((x * 7 + y * 101) % 256) / 255.0f;
			color.A = (float)((x * 23 + y * 5) % 256) / 255.0f;

			source->SetColorAt(color, x, y);
		}
	}

	MipMapGenOptions options; // Box filter, linear (no sRGB), no normalization.

	TAsyncOp<Vector<TShared<PixelData>>> mipmapOp = GpuGenerateMipmap::Generate(source, options);
	mipmapOp.BlockUntilComplete();

	Vector<TShared<PixelData>> mips = mipmapOp.GetReturnValue();

	const bool mipsOk = !mips.empty();
	B3D_TEST_ASSERT(mipsOk)

	// 4x4 produces a full chain: 4x4, 2x2, 1x1.
	B3D_TEST_ASSERT(mips.size() == 3)
	B3D_TEST_ASSERT(mips[0]->GetWidth() == 4 && mips[0]->GetHeight() == 4)
	B3D_TEST_ASSERT(mips[1]->GetWidth() == 2 && mips[1]->GetHeight() == 2)
	B3D_TEST_ASSERT(mips[2]->GetWidth() == 1 && mips[2]->GetHeight() == 1)

	// 8-bit round-trip plus a single average leaves at most ~1.5/255 of error per channel.
	const float tolerance = 3.0f / 255.0f;

	// Mip 0 is the unfiltered source.
	for(u32 y = 0; y < size; ++y)
	{
		for(u32 x = 0; x < size; ++x)
		{
			const Color sourcePixel = source->GetColorAt(x, y);
			const Color mipPixel = mips[0]->GetColorAt(x, y);

			B3D_TEST_ASSERT(std::fabs(sourcePixel.R - mipPixel.R) < tolerance)
			B3D_TEST_ASSERT(std::fabs(sourcePixel.G - mipPixel.G) < tolerance)
			B3D_TEST_ASSERT(std::fabs(sourcePixel.B - mipPixel.B) < tolerance)
			B3D_TEST_ASSERT(std::fabs(sourcePixel.A - mipPixel.A) < tolerance)
		}
	}

	// Mip 1: each texel is the box average of the corresponding 2x2 source block.
	for(u32 y = 0; y < 2; ++y)
	{
		for(u32 x = 0; x < 2; ++x)
		{
			float red = 0, green = 0, blue = 0, alpha = 0;
			for(u32 dy = 0; dy < 2; ++dy)
			{
				for(u32 dx = 0; dx < 2; ++dx)
				{
					const Color sourcePixel = source->GetColorAt(x * 2 + dx, y * 2 + dy);
					red += sourcePixel.R;
					green += sourcePixel.G;
					blue += sourcePixel.B;
					alpha += sourcePixel.A;
				}
			}

			red *= 0.25f;
			green *= 0.25f;
			blue *= 0.25f;
			alpha *= 0.25f;

			const Color mipPixel = mips[1]->GetColorAt(x, y);
			B3D_TEST_ASSERT(std::fabs(red - mipPixel.R) < tolerance)
			B3D_TEST_ASSERT(std::fabs(green - mipPixel.G) < tolerance)
			B3D_TEST_ASSERT(std::fabs(blue - mipPixel.B) < tolerance)
			B3D_TEST_ASSERT(std::fabs(alpha - mipPixel.A) < tolerance)
		}
	}

	// Mip 2: the average of all 16 source texels.
	{
		float red = 0, green = 0, blue = 0, alpha = 0;
		for(u32 y = 0; y < size; ++y)
		{
			for(u32 x = 0; x < size; ++x)
			{
				const Color sourcePixel = source->GetColorAt(x, y);
				red += sourcePixel.R;
				green += sourcePixel.G;
				blue += sourcePixel.B;
				alpha += sourcePixel.A;
			}
		}

		const float inverse = 1.0f / 16.0f;
		red *= inverse;
		green *= inverse;
		blue *= inverse;
		alpha *= inverse;

		const Color mipPixel = mips[2]->GetColorAt(0, 0);
		B3D_TEST_ASSERT(std::fabs(red - mipPixel.R) < tolerance)
		B3D_TEST_ASSERT(std::fabs(green - mipPixel.G) < tolerance)
		B3D_TEST_ASSERT(std::fabs(blue - mipPixel.B) < tolerance)
		B3D_TEST_ASSERT(std::fabs(alpha - mipPixel.A) < tolerance)
	}
}

void ImporterTestSuite::TestOggImport_Default()
{
	TShared<AudioClipImportOptions> options = AudioClipImportOptions::Create();
	options->Is3D = false;

	HAudioClip clip = GetImporter().Import<AudioClip>(mOggPath, options);
	B3D_TEST_ASSERT(clip != nullptr)
	B3D_TEST_ASSERT(clip.IsLoaded())

	B3D_TEST_ASSERT(clip->GetFormat() == AudioFormat::PCM)
	B3D_TEST_ASSERT(clip->GetReadMode() == AudioReadMode::LoadDecompressed)
	B3D_TEST_ASSERT(clip->GetFrequency() > 0)
	B3D_TEST_ASSERT(clip->GetChannelCount() >= 1 && clip->GetChannelCount() <= 2)
	B3D_TEST_ASSERT(clip->GetBitDepth() == 16)
	B3D_TEST_ASSERT(clip->GetSampleCount() > 0)
	B3D_TEST_ASSERT(clip->GetLength() > 0.0f)
}

void ImporterTestSuite::TestOggImport_KeepCompressed()
{
	TShared<AudioClipImportOptions> options = AudioClipImportOptions::Create();
	options->Is3D = false;
	options->Format = AudioFormat::VORBIS;
	options->ReadMode = AudioReadMode::LoadCompressed;

	HAudioClip clip = GetImporter().Import<AudioClip>(mOggPath, options);
	B3D_TEST_ASSERT(clip != nullptr)
	B3D_TEST_ASSERT(clip.IsLoaded())

	B3D_TEST_ASSERT(clip->GetFormat() == AudioFormat::VORBIS)
	B3D_TEST_ASSERT(clip->GetReadMode() == AudioReadMode::LoadCompressed)
	B3D_TEST_ASSERT(clip->GetFrequency() > 0)
	B3D_TEST_ASSERT(clip->GetChannelCount() >= 1 && clip->GetChannelCount() <= 2)
	B3D_TEST_ASSERT(clip->GetSampleCount() > 0)
	B3D_TEST_ASSERT(clip->GetLength() > 0.0f)
}

void ImporterTestSuite::TestFlacImport_Default()
{
	TShared<AudioClipImportOptions> options = AudioClipImportOptions::Create();
	options->Is3D = false;

	HAudioClip clip = GetImporter().Import<AudioClip>(mFlacPath, options);
	B3D_TEST_ASSERT(clip != nullptr)
	B3D_TEST_ASSERT(clip.IsLoaded())

	// Fixture metadata: stereo, 44.1 kHz, 16-bit, 16870 frames.
	B3D_TEST_ASSERT(clip->GetFormat() == AudioFormat::PCM)
	B3D_TEST_ASSERT(clip->GetFrequency() == 44100)
	B3D_TEST_ASSERT(clip->GetChannelCount() == 2)
	B3D_TEST_ASSERT(clip->GetBitDepth() == 16)
	B3D_TEST_ASSERT(clip->GetSampleCount() > 0)
	B3D_TEST_ASSERT(clip->GetLength() > 0.0f)
}

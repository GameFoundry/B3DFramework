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
#include "B3DApplication.h"

#include <cmath>
#include <cstring>
#include <cstdlib>

using namespace b3d;

namespace
{
	// Expands a 565-packed color to 8-bit per channel using bit replication, matching the
	// hardware BCn decoder (the same expansion the encoder shader assumes).
	void Expand565(u16 c, i32& r, i32& g, i32& b)
	{
		const i32 r5 = (c >> 11) & 0x1F;
		const i32 g6 = (c >> 5) & 0x3F;
		const i32 b5 = c & 0x1F;
		r = (r5 << 3) | (r5 >> 2);
		g = (g6 << 2) | (g6 >> 4);
		b = (b5 << 3) | (b5 >> 2);
	}

	// Decodes a BC1 color block (8 bytes) into 16 RGB triplets (alpha set to 255).
	// When @p fourColorOnly is true the 3-colour/punch-through mode is never used (BC2/BC3 color).
	void DecodeBC1Color(const u8* block, bool fourColorOnly, u8 outRGBA[64])
	{
		const u16 c0 = (u16)(block[0] | (block[1] << 8));
		const u16 c1 = (u16)(block[2] | (block[3] << 8));

		i32 pal[4][3];
		Expand565(c0, pal[0][0], pal[0][1], pal[0][2]);
		Expand565(c1, pal[1][0], pal[1][1], pal[1][2]);

		bool transparentBlack = false;
		if(c0 > c1 || fourColorOnly)
		{
			for(i32 k = 0; k < 3; ++k)
			{
				pal[2][k] = (2 * pal[0][k] + pal[1][k] + 1) / 3;
				pal[3][k] = (pal[0][k] + 2 * pal[1][k] + 1) / 3;
			}
		}
		else
		{
			for(i32 k = 0; k < 3; ++k)
			{
				pal[2][k] = (pal[0][k] + pal[1][k]) / 2;
				pal[3][k] = 0;
			}
			transparentBlack = true;
		}

		const u32 indices = block[4] | (block[5] << 8) | (block[6] << 16) | (block[7] << 24);
		for(i32 i = 0; i < 16; ++i)
		{
			const u32 idx = (indices >> (i * 2)) & 0x3;
			outRGBA[i * 4 + 0] = (u8)pal[idx][0];
			outRGBA[i * 4 + 1] = (u8)pal[idx][1];
			outRGBA[i * 4 + 2] = (u8)pal[idx][2];
			outRGBA[i * 4 + 3] = (transparentBlack && idx == 3) ? 0 : 255;
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

		for(i32 i = 0; i < 16; ++i)
		{
			const u32 idx = (u32)((bits >> (i * 3)) & 0x7);
			out[i] = (u8)pal[idx];
		}
	}

	// General BC7 block decoder. Decodes a 16-byte block into 16 RGBA quads. Drives every field width off the standard
	// BC7 per-mode descriptor table, so enabling a new encoder mode needs no decoder change (the 3-subset partition table
	// for modes 0/2 is added alongside those encoders). Matches the hardware decoder: P-bit appended as the endpoint LSB,
	// then bit-replicated to 8 bits.
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
		static const i32 wt[5][16] = {
			{ 0 },
			{ 0, 64 },
			{ 0, 21, 43, 64 },
			{ 0, 9, 18, 27, 37, 46, 55, 64 },
			{ 0, 4, 9, 13, 17, 21, 26, 30, 34, 38, 43, 47, 51, 55, 60, 64 }
		};

		u32 pos = 0;
		auto getBits = [&](u32 num) -> u32
		{
			u32 v = 0;
			for(u32 i = 0; i < num; ++i)
			{
				const u32 p = pos + i;
				v |= ((block[p >> 3] >> (p & 7)) & 1u) << i;
			}
			pos += num;
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

			const i32 wc = wt[cw][ci];
			const i32 wa = wt[aw][ai];
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

	double PsnrFromError(double squaredError, u64 samples)
	{
		const double mse = squaredError / (double)samples;
		if(mse <= 0.0)
			return 99.0;
		return 10.0 * std::log10(255.0 * 255.0 / mse);
	}

	// ---- BC6H (UF16) decode. Only the modes our encoder emits need a
	// case in DecodeBC6H_UF16; unhandled modes output zero (which collapses PSNR and flags the gap). ----

	// Reads @p len bits at absolute bit position @p start, LSB-first (matching the encoder's PutBits/SetBits).
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

	// Decodes a BC6H UF16 block (16 bytes) into 16 RGB float triplets.
	void DecodeBC6H_UF16(const u8* block, float out[16][3])
	{
		const u32 lead = (block[0] & 0x02) ? (block[0] & 0x1F) : (block[0] & 0x01);

		i32 wBits = 0;
		i32 tBits[3] = { 0, 0, 0 };
		bool oneRegion = true;
		bool transformed = false;
		i32 shape = 0;
		// Endpoint codes: ec[endpoint A/B for subset 0][channel] and ec2 for subset 1 (two-region).
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
			ecA[0][0] = BC6GetBits(block, 5, 10) | (BC6GetBits(block, 39, 6) << 10);
			ecA[0][1] = BC6GetBits(block, 15, 10) | (BC6GetBits(block, 49, 6) << 10);
			ecA[0][2] = BC6GetBits(block, 25, 10) | (BC6GetBits(block, 59, 6) << 10);
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

	// ---- Optional debug image dump. The compression tests are otherwise purely numeric (PSNR); set the env var
	// B3D_DUMP_COMPRESS_IMAGES=1 to also write side-by-side [source | decoded] PNGs to the working directory. ----

	bool DumpCompressImagesEnabled()
	{
		const char* v = std::getenv("B3D_DUMP_COMPRESS_IMAGES");
		return v != nullptr && v[0] != '\0' && !(v[0] == '0' && v[1] == '\0');
	}

	// Writes a [source | gap | decoded] comparison PNG. Both inputs are tightly-packed RGBA8 buffers (width*height*4,
	// top-left origin) already mapped to a displayable interpretation of the format. @p name yields "<name>.png".
	void WriteSideBySidePNG(const String& name, u32 width, u32 height, const Vector<u8>& srcRGBA, const Vector<u8>& decRGBA)
	{
		const u32 gap = 4;
		const u32 compositeWidth = width * 2 + gap;
		const TShared<PixelData> img = PixelData::Create(compositeWidth, height, 1, PF_RGBA8);
		u8* const dst = img->GetData();
		const u32 rowPitch = img->GetRowPitch();

		for(u32 y = 0; y < height; ++y)
		{
			u8* const row = dst + y * rowPitch;
			for(u32 x = 0; x < width; ++x)
			{
				const u8* const s = &srcRGBA[(y * width + x) * 4];
				const u8* const d = &decRGBA[(y * width + x) * 4];
				u8* const lp = row + x * 4;                       // left panel: source
				u8* const rp = row + (width + gap + x) * 4;       // right panel: decoded
				lp[0] = s[0]; lp[1] = s[1]; lp[2] = s[2]; lp[3] = 255;
				rp[0] = d[0]; rp[1] = d[1]; rp[2] = d[2]; rp[3] = 255;
			}
			// Magenta separator so the seam is obvious.
			for(u32 g = 0; g < gap; ++g)
			{
				u8* const sep = row + (width + g) * 4;
				sep[0] = 255; sep[1] = 0; sep[2] = 255; sep[3] = 255;
			}
		}

		const Path outPath = Path(String("compress_") + name);
		if(PixelUtility::SaveImage(img, outPath, ImageFormat::PNG, true))
			B3D_LOG(Info, LogPixelUtility, "Wrote compression comparison image '{0}.png'", name);
	}

	// Writes a [source | gap | decoded] comparison OpenEXR. Both inputs are tightly-packed RGB float buffers
	// (width*height*3, top-left origin) holding raw linear HDR values. @p name yields "<name>.exr". Used for BC6H so the
	// full dynamic range is preserved (PNG can't hold HDR), rather than tone-mapping down to 8-bit.
	void WriteSideBySideEXR(const String& name, u32 width, u32 height, const Vector<float>& srcRGB, const Vector<float>& decRGB)
	{
		const u32 gap = 4;
		const u32 compositeWidth = width * 2 + gap;
		const TShared<PixelData> img = PixelData::Create(compositeWidth, height, 1, PF_RGB32F);

		for(u32 y = 0; y < height; ++y)
		{
			for(u32 x = 0; x < width; ++x)
			{
				const float* const s = &srcRGB[(y * width + x) * 3];
				const float* const d = &decRGB[(y * width + x) * 3];
				img->SetColorAt(Color(s[0], s[1], s[2], 1.0f), x, y);                 // left panel: source
				img->SetColorAt(Color(d[0], d[1], d[2], 1.0f), width + gap + x, y);   // right panel: decoded
			}
			// Magenta separator so the seam is obvious.
			for(u32 g = 0; g < gap; ++g)
				img->SetColorAt(Color(1.0f, 0.0f, 1.0f, 1.0f), width + g, y);
		}

		const Path outPath = Path(String("compress_") + name);
		if(PixelUtility::SaveImage(img, outPath, ImageFormat::EXR))
			B3D_LOG(Info, LogPixelUtility, "Wrote compression comparison image '{0}.exr'", name);
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

	const TextureProperties& props = texture->GetProperties();
	B3D_TEST_ASSERT(props.Format == PF_RGBA8)
	B3D_TEST_ASSERT(props.Width == 64)
	B3D_TEST_ASSERT(props.Height == 64)
	B3D_TEST_ASSERT(props.MipMapCount == 0)
}

void ImporterTestSuite::TestPngImport_WithMips()
{
	TShared<TextureImportOptions> options = TextureImportOptions::Create();
	options->GenerateMips = true;

	HTexture texture = GetImporter().Import<Texture>(mImagePath, options);
	B3D_TEST_ASSERT(texture != nullptr)
	B3D_TEST_ASSERT(texture.IsLoaded())

	const TextureProperties& props = texture->GetProperties();
	B3D_TEST_ASSERT(props.Format == PF_RGBA8)
	B3D_TEST_ASSERT(props.Width == 64)
	B3D_TEST_ASSERT(props.Height == 64)
	B3D_TEST_ASSERT(props.MipMapCount == 6)
}

void ImporterTestSuite::TestPngImport_BC3()
{
	TShared<TextureImportOptions> options = TextureImportOptions::Create();
	options->GenerateMips = true;
	options->Format = PF_BC3;

	HTexture texture = GetImporter().Import<Texture>(mImagePath, options);
	B3D_TEST_ASSERT(texture != nullptr)
	B3D_TEST_ASSERT(texture.IsLoaded())

	const TextureProperties& props = texture->GetProperties();
	B3D_TEST_ASSERT(props.Format == PF_BC3)
	B3D_TEST_ASSERT(props.Width == 64)
	B3D_TEST_ASSERT(props.Height == 64)
	B3D_TEST_ASSERT(props.MipMapCount == 6)
}

void ImporterTestSuite::TestGpuCompress_Psnr()
{
	// GPU compression needs a real backend; skip on a headless / Null backend so the suite stays
	// green on machines without a usable GPU.
	const TShared<GpuDevice> device = GetApplication().GetPrimaryGpuDevice();
	if(device == nullptr || device->GetCapabilities().BackendName == "Null")
		return;

	// Load the source image as RGBA8 and read it back to the CPU.
	TShared<TextureImportOptions> options = TextureImportOptions::Create();
	HTexture texture = GetImporter().Import<Texture>(mImagePath, options);
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
	Vector<u8> src(width * height * 4);
	for(u32 y = 0; y < height; ++y)
		for(u32 x = 0; x < width; ++x)
		{
			const Color c = source->GetColorAt(x, y);
			u8* p = &src[(y * width + x) * 4];
			p[0] = (u8)Math::Clamp((i32)std::lround(c.R * 255.0f), 0, 255);
			p[1] = (u8)Math::Clamp((i32)std::lround(c.G * 255.0f), 0, 255);
			p[2] = (u8)Math::Clamp((i32)std::lround(c.B * 255.0f), 0, 255);
			p[3] = (u8)Math::Clamp((i32)std::lround(c.A * 255.0f), 0, 255);
		}

	struct Case { PixelFormat Format; const char* Name; u32 BlockBytes; };
	const Case cases[] = {
		{ PF_BC1, "BC1", 8 },
		{ PF_BC3, "BC3", 16 },
		{ PF_BC4, "BC4", 8 },
		{ PF_BC5, "BC5", 16 },
		{ PF_BC7, "BC7", 16 },
	};

	const bool dump = DumpCompressImagesEnabled();

	for(const Case& test : cases)
	{
		TShared<PixelData> compressed = PixelData::Create(width, height, 1, test.Format);
		CompressionOptions co;
		co.Format = test.Format;
		// Call the GPU compressor directly: PixelUtility::Compress routes through nvtt while B3D_USE_NVTT is on (the
		// default), so the test must target the GPU path explicitly to keep exercising it until the switch is complete.
		const bool compressOk = GpuTextureCompressor::Compress(*source, *compressed, co);
		B3D_TEST_ASSERT(compressOk)

		const u8* blocks = compressed->GetData();
		double error = 0.0;
		u64 samples = 0;

		// Display-mapped source/decoded surfaces, filled only when dumping comparison images.
		Vector<u8> srcDisp, decDisp;
		if(dump) { srcDisp.resize(width * height * 4); decDisp.resize(width * height * 4); }

		for(u32 by = 0; by < blocksY; ++by)
		{
			for(u32 bx = 0; bx < blocksX; ++bx)
			{
				const u8* block = blocks + (by * blocksX + bx) * test.BlockBytes;

				u8 decRGBA[64];
				u8 decR[16];
				u8 decG[16];
				if(test.Format == PF_BC1)
					DecodeBC1Color(block, false, decRGBA);
				else if(test.Format == PF_BC3)
				{
					DecodeBC4(block, decR);            // alpha block
					DecodeBC1Color(block + 8, true, decRGBA); // color block (always 4-colour)
				}
				else if(test.Format == PF_BC4)
					DecodeBC4(block, decR);
				else if(test.Format == PF_BC7)
					DecodeBC7(block, decRGBA);
				else // BC5
				{
					DecodeBC4(block, decR);
					DecodeBC4(block + 8, decG);
				}

				for(u32 i = 0; i < 16; ++i)
				{
					const u32 px = bx * 4 + (i % 4);
					const u32 py = by * 4 + (i / 4);
					const u8* s = &src[(py * width + px) * 4];

					if(test.Format == PF_BC1)
					{
						for(i32 c = 0; c < 3; ++c) { double d = (double)s[c] - decRGBA[i * 4 + c]; error += d * d; }
						samples += 3;
					}
					else if(test.Format == PF_BC3)
					{
						for(i32 c = 0; c < 3; ++c) { double d = (double)s[c] - decRGBA[i * 4 + c]; error += d * d; }
						double da = (double)s[3] - decR[i]; error += da * da; // alpha from BC4 block
						samples += 4;
					}
					else if(test.Format == PF_BC4)
					{
						double d = (double)s[0] - decR[i]; error += d * d; samples += 1;
					}
					else if(test.Format == PF_BC7)
					{
						for(i32 c = 0; c < 4; ++c) { double d = (double)s[c] - decRGBA[i * 4 + c]; error += d * d; }
						samples += 4;
					}
					else // BC5
					{
						double dr = (double)s[0] - decR[i]; double dg = (double)s[1] - decG[i];
						error += dr * dr + dg * dg; samples += 2;
					}

					if(dump)
					{
						// Map both source and decoded through the same channel interpretation for a fair comparison.
						u8 sr, sg, sb, dr, dg, db;
						if(test.Format == PF_BC4)      { sr = sg = sb = s[0]; dr = dg = db = decR[i]; }
						else if(test.Format == PF_BC5) { sr = s[0]; sg = s[1]; sb = 0; dr = decR[i]; dg = decG[i]; db = 0; }
						else                           { sr = s[0]; sg = s[1]; sb = s[2]; dr = decRGBA[i * 4 + 0]; dg = decRGBA[i * 4 + 1]; db = decRGBA[i * 4 + 2]; }
						u8* const sp = &srcDisp[(py * width + px) * 4]; sp[0] = sr; sp[1] = sg; sp[2] = sb; sp[3] = 255;
						u8* const dp = &decDisp[(py * width + px) * 4]; dp[0] = dr; dp[1] = dg; dp[2] = db; dp[3] = 255;
					}
				}
			}
		}

		const double psnr = PsnrFromError(error, samples);
		B3D_LOG(Info, LogPixelUtility, "GPU compression {0}: PSNR = {1} dB", test.Name, psnr);

		if(dump)
			WriteSideBySidePNG(test.Name, width, height, srcDisp, decDisp);

		// Sanity floor: a working encoder is far above this; random/garbage blocks would be ~8-10 dB.
		B3D_TEST_ASSERT(psnr > 15.0)
	}
}

void ImporterTestSuite::TestGpuCompress_BC6H_Psnr()
{
	// GPU compression needs a real backend; skip on a headless / Null backend.
	const TShared<GpuDevice> device = GetApplication().GetPrimaryGpuDevice();
	if(device == nullptr || device->GetCapabilities().BackendName == "Null")
		return;

	// Synthesize a smooth HDR gradient (values 0..8, beyond the LDR range) so BC6H has real HDR to encode.
	const u32 size = 64;
	TShared<PixelData> source = PixelData::Create(size, size, 1, PF_RGBA32F);
	for(u32 y = 0; y < size; ++y)
		for(u32 x = 0; x < size; ++x)
		{
			Color c;
			c.R = (float)x / (float)(size - 1) * 4.0f;
			c.G = (float)y / (float)(size - 1) * 6.0f;
			c.B = (float)(x + y) / (float)(2 * (size - 1)) * 8.0f;
			c.A = 1.0f;
			source->SetColorAt(c, x, y);
		}

	TShared<PixelData> compressed = PixelData::Create(size, size, 1, PF_BC6H);
	CompressionOptions co;
	co.Format = PF_BC6H;
	// Call the GPU compressor directly (see note in TestGpuCompress_Psnr): PixelUtility::Compress uses nvtt by default.
	const bool compressOk = GpuTextureCompressor::Compress(*source, *compressed, co);
	B3D_TEST_ASSERT(compressOk)

	const u32 blocksX = size / 4, blocksY = size / 4;
	const u8* blocks = compressed->GetData();
	double error = 0.0, peak = 0.0;
	u64 samples = 0;

	// Raw HDR display surfaces (RGB float), filled only when dumping a comparison image; written out as OpenEXR.
	const bool dump = DumpCompressImagesEnabled();
	Vector<float> srcDisp, decDisp;
	if(dump) { srcDisp.resize(size * size * 3); decDisp.resize(size * size * 3); }

	for(u32 by = 0; by < blocksY; ++by)
		for(u32 bx = 0; bx < blocksX; ++bx)
		{
			float dec[16][3];
			DecodeBC6H_UF16(blocks + (by * blocksX + bx) * 16, dec);

			for(u32 i = 0; i < 16; ++i)
			{
				const u32 px = bx * 4 + (i % 4);
				const u32 py = by * 4 + (i / 4);
				const Color s = source->GetColorAt(px, py);
				const float sc[3] = { s.R, s.G, s.B };
				for(i32 ch = 0; ch < 3; ++ch)
				{
					const double d = (double)sc[ch] - (double)dec[i][ch];
					error += d * d;
					peak = std::max(peak, (double)sc[ch]);
					++samples;
				}

				if(dump)
				{
					float* const sp = &srcDisp[(py * size + px) * 3];
					sp[0] = sc[0]; sp[1] = sc[1]; sp[2] = sc[2];
					float* const dp = &decDisp[(py * size + px) * 3];
					dp[0] = dec[i][0]; dp[1] = dec[i][1]; dp[2] = dec[i][2];
				}
			}
		}

	const double mse = error / (double)samples;
	const double psnr = (mse <= 0.0) ? 99.0 : 10.0 * std::log10(peak * peak / mse);
	B3D_LOG(Info, LogPixelUtility, "GPU compression BC6H: PSNR = {0} dB (peak {1})", psnr, peak);

	if(dump)
		WriteSideBySideEXR("BC6H", size, size, srcDisp, decDisp);

	// A working one-region BC6H encoder on a smooth gradient is far above this floor.
	B3D_TEST_ASSERT(psnr > 30.0)
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
			Color c;
			c.R = (float)((x * 37 + y * 11) % 256) / 255.0f;
			c.G = (float)((x * 17 + y * 53) % 256) / 255.0f;
			c.B = (float)((x * 7 + y * 101) % 256) / 255.0f;
			c.A = (float)((x * 23 + y * 5) % 256) / 255.0f;
			source->SetColorAt(c, x, y);
		}
	}

	MipMapGenOptions options; // Box filter, linear (no sRGB), no normalization.
	// Call the GPU generator directly: PixelUtility::GenerateMipmaps routes through nvtt while B3D_USE_NVTT is on (the
	// default), so the test must target the GPU path explicitly to keep exercising it until the switch is complete.
	Vector<TShared<PixelData>> mips;
	const bool mipsOk = GpuGenerateMipmap::Generate(source, options, mips);
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
			const Color s = source->GetColorAt(x, y);
			const Color m = mips[0]->GetColorAt(x, y);
			B3D_TEST_ASSERT(std::fabs(s.R - m.R) < tolerance)
			B3D_TEST_ASSERT(std::fabs(s.G - m.G) < tolerance)
			B3D_TEST_ASSERT(std::fabs(s.B - m.B) < tolerance)
			B3D_TEST_ASSERT(std::fabs(s.A - m.A) < tolerance)
		}
	}

	// Mip 1: each texel is the box average of the corresponding 2x2 source block.
	for(u32 y = 0; y < 2; ++y)
	{
		for(u32 x = 0; x < 2; ++x)
		{
			float r = 0, g = 0, b = 0, a = 0;
			for(u32 dy = 0; dy < 2; ++dy)
			{
				for(u32 dx = 0; dx < 2; ++dx)
				{
					const Color s = source->GetColorAt(x * 2 + dx, y * 2 + dy);
					r += s.R; g += s.G; b += s.B; a += s.A;
				}
			}
			r *= 0.25f; g *= 0.25f; b *= 0.25f; a *= 0.25f;

			const Color m = mips[1]->GetColorAt(x, y);
			B3D_TEST_ASSERT(std::fabs(r - m.R) < tolerance)
			B3D_TEST_ASSERT(std::fabs(g - m.G) < tolerance)
			B3D_TEST_ASSERT(std::fabs(b - m.B) < tolerance)
			B3D_TEST_ASSERT(std::fabs(a - m.A) < tolerance)
		}
	}

	// Mip 2: the average of all 16 source texels.
	{
		float r = 0, g = 0, b = 0, a = 0;
		for(u32 y = 0; y < size; ++y)
		{
			for(u32 x = 0; x < size; ++x)
			{
				const Color s = source->GetColorAt(x, y);
				r += s.R; g += s.G; b += s.B; a += s.A;
			}
		}
		const float inv = 1.0f / 16.0f;
		r *= inv; g *= inv; b *= inv; a *= inv;

		const Color m = mips[2]->GetColorAt(0, 0);
		B3D_TEST_ASSERT(std::fabs(r - m.R) < tolerance)
		B3D_TEST_ASSERT(std::fabs(g - m.G) < tolerance)
		B3D_TEST_ASSERT(std::fabs(b - m.B) < tolerance)
		B3D_TEST_ASSERT(std::fabs(a - m.A) < tolerance)
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

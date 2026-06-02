//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Image/B3DTextureCompressor.h"

#include "Image/B3DPixelUtility.h"
#include "Image/B3DPixelData.h"
#include "Image/B3DTexture.h"
#include "Renderer/B3DRendererMaterial.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuBuffer.h"
#include "GpuBackend/B3DGpuParameterSet.h"
#include "GpuBackend/B3DGpuPipelineState.h"
#include "CoreObject/B3DRenderThread.h"
#include "B3DApplication.h"

#include <cstring>

using namespace b3d;

namespace b3d
{
	namespace
	{
		/**
		 * Maps a block-compressed pixel format to the TextureCompress.bsl FORMAT variation index, the output buffer format
		 * (block size), and whether the source must be fed as HDR floating-point (BC6H) rather than normalized RGBA8.
		 */
		bool GetFormatInfo(PixelFormat format, i32& variation, GpuBufferFormat& bufferFormat, bool& isHdr)
		{
			isHdr = false;
			switch(format)
			{
			case PF_BC1:
			case PF_BC1a:
				variation = 0;
				bufferFormat = BF_32X2U; // 64-bit block
				return true;
			case PF_BC4:
				variation = 1;
				bufferFormat = BF_32X2U; // 64-bit block
				return true;
			case PF_BC3:
				variation = 2;
				bufferFormat = BF_32X4U; // 128-bit block
				return true;
			case PF_BC5:
				variation = 3;
				bufferFormat = BF_32X4U; // 128-bit block
				return true;
			case PF_BC7:
				variation = 4;
				bufferFormat = BF_32X4U; // 128-bit block
				return true;
			case PF_BC6H:
				variation = 5;
				bufferFormat = BF_32X4U; // 128-bit block
				isHdr = true;            // HDR source: fed as RGBA32F, not normalized RGBA8
				return true;
			default:
				return false;
			}
		}

		/** RendererMaterial wrapper around the block-compression compute shader. */
		class TextureCompressMaterial : public render::RendererMaterial<TextureCompressMaterial>
		{
			RMAT_DEF("TextureCompress.bsl")

		public:
			TextureCompressMaterial() = default;

			/** Records the dispatch that compresses @p input into @p output. Must run on the render thread. */
			void Execute(render::GpuCommandBuffer& commandBuffer, const TShared<render::GpuBuffer>& input, const TShared<render::GpuBuffer>& output, const TShared<render::GpuBuffer>& meta, const Vector2I& blockCount)
			{
				mGpuParameterSet->SetStorageBuffer("gInput", input);
				mGpuParameterSet->SetStorageBuffer("gOutput", output);
				mGpuParameterSet->SetStorageBuffer("gMeta", meta);

				Bind(commandBuffer);
				commandBuffer.DispatchCompute((u32)Math::DivideAndRoundUp(blockCount.X, 8), (u32)Math::DivideAndRoundUp(blockCount.Y, 8));
			}

			template <int FORMAT>
			static const ShaderVariationParameters& GetVariationParams()
			{
				static ShaderVariationParameters variation = ShaderVariationParameters({ ShaderVariationParameter("FORMAT", FORMAT) });

				return variation;
			}

			static TextureCompressMaterial* GetVariation(i32 variation)
			{
				switch(variation)
				{
				case 0: return Get(GetVariationParams<0>());
				case 1: return Get(GetVariationParams<1>());
				case 2: return Get(GetVariationParams<2>());
				case 3: return Get(GetVariationParams<3>());
				case 5: return Get(GetVariationParams<5>());
				default: return Get(GetVariationParams<4>());
				}
			}
		};

		/**
		 * Performs the actual GPU compression. Must be called on the render thread: it creates GPU resources, dispatches the
		 * compute kernel and reads the packed blocks back to the CPU. Returns true on success.
		 */
		bool CompressOnRenderThread(TextureCompressMaterial* material, const TShared<PixelData>& source, GpuBufferFormat inputBufferFormat, GpuBufferFormat outputBufferFormat, PixelData& destination)
		{
			AssertIfNotRenderThread();

			const TShared<GpuDevice> gpuDevice = GetApplication().GetPrimaryGpuDevice();
			if(gpuDevice == nullptr)
				return false;

			const u32 width = source->GetWidth();
			const u32 height = source->GetHeight();
			const u32 blockCountX = Math::DivideAndRoundUp(width, 4u);
			const u32 blockCountY = Math::DivideAndRoundUp(height, 4u);
			const u32 blockCount = blockCountX * blockCountY;

			// Upload the source pixels into a typed buffer, read as float4 in the shader. LDR formats use a normalized
			// RGBA8 buffer (BF_8X4); BC6H uses a full-float RGBA32F buffer (BF_32X4F) so HDR values survive. A buffer
			// avoids the image-layout transitions a freshly-uploaded sampled texture would need in a standalone dispatch.
			const TShared<render::GpuBuffer> input = gpuDevice->CreateGpuBuffer(GpuBufferCreateInformation::CreateSimpleStorage(inputBufferFormat, width * height));
			if(input == nullptr)
				return false;

			render::GpuBufferUtility::Write(input, 0, source->GetConsecutiveSize(), source->GetData());

			// Metadata buffer: [0] = texture size, [1] = number of blocks (two int2 entries).
			const i32 meta[4] = { (i32)width, (i32)height, (i32)blockCountX, (i32)blockCountY };
			const TShared<render::GpuBuffer> metaBuffer = gpuDevice->CreateGpuBuffer(GpuBufferCreateInformation::CreateSimpleStorage(BF_32X2S, 2));
			if(metaBuffer == nullptr)
				return false;

			render::GpuBufferUtility::Write(metaBuffer, 0, sizeof(meta), meta);

			// Output buffer receives the packed blocks; readable by the CPU and writable as a compute UAV.
			const TShared<render::GpuBuffer> output = gpuDevice->CreateGpuBuffer(GpuBufferCreateInformation::CreateSimpleStorage(outputBufferFormat, blockCount,
				GpuBufferFlag::StoreOnCPUWithGPUAccess | GpuBufferFlag::AllowUnorderedAccessOnTheGPU));

			if(output == nullptr)
				return false;

			const TShared<render::GpuCommandBufferPool> pool = gpuDevice->CreateGpuCommandBufferPool(render::GpuCommandBufferPoolCreateInformation::CreateForThisThread());

			render::GpuCommandBufferCreateInformation commandBufferInfo;
			commandBufferInfo.Name = "TextureCompress";
			const TShared<render::GpuCommandBuffer> commandBuffer = pool->Create(commandBufferInfo);

			// One thread per 4x4 block; the kernel uses [numthreads(8, 8, 1)].
			material->Execute(*commandBuffer, input, output, metaBuffer, Vector2I((i32)blockCountX, (i32)blockCountY));

			gpuDevice->SubmitCommandBuffer(commandBuffer);

			// Blocking read-back of the packed blocks into the destination surface (waits on the compute write).
			render::GpuBufferUtility::Read(output, 0, destination.GetConsecutiveSize(), destination.GetData());

			return true;
		}

		// ---- CPU reference decoders (used by TextureCompressionUtility) ----

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
	} // anonymous namespace

	bool GpuTextureCompressor::IsFormatSupported(PixelFormat format)
	{
		i32 variation;
		GpuBufferFormat bufferFormat;
		bool isHdr;
		return GetFormatInfo(format, variation, bufferFormat, isHdr);
	}

	bool GpuTextureCompressor::Compress(const PixelData& source, PixelData& destination, const CompressionOptions& options)
	{
		i32 variation;
		GpuBufferFormat bufferFormat;
		bool isHdr;
		if(!GetFormatInfo(options.Format, variation, bufferFormat, isHdr))
			return false;

		if(GetApplication().GetPrimaryGpuDevice() == nullptr)
			return false;

		// Convert the source to a surface the compute shader can sample: RGBA8 for LDR formats, RGBA32F for HDR (BC6H).
		// CPU work, safe on any thread.
		const PixelFormat interimFormat = isHdr ? PF_RGBA32F : PF_RGBA8;
		const GpuBufferFormat inputBufferFormat = isHdr ? BF_32X4F : BF_8X4;
		const TShared<PixelData> convertedSource = PixelData::Create(source.GetWidth(), source.GetHeight(), 1, interimFormat);
		PixelUtility::BulkPixelConversion(source, *convertedSource);

		// Compile/fetch the shader variation (blocks until ready).
		TextureCompressMaterial* const material = TextureCompressMaterial::GetVariation(variation);
		if(material == nullptr || material->GetComputePipeline() == nullptr)
			return false;

		// GPU resource creation and dispatch must run on the render thread. Run inline if we're already there,
		// otherwise marshal the work across and block until it finishes.
		bool success = false;
		auto fnGpuWork = [&]() { success = CompressOnRenderThread(material, convertedSource, inputBufferFormat, bufferFormat, destination); };

		if(B3D_CURRENT_THREAD_ID == GetRenderThread().GetThreadId())
			fnGpuWork();
		else
			GetRenderThread().PostCommand(fnGpuWork, "GPU texture compression", true);

		return success;
	}

	void TextureCompressionUtility::DecodeBC1(const u8* block, bool fourColorOnly, u8 outRGBA[64])
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

	void TextureCompressionUtility::DecodeBC4(const u8* block, u8 out[16])
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

	// General BC7 block decoder. Drives every field width off the standard BC7 per-mode descriptor table, so enabling a
	// new encoder mode needs no decoder change (the 3-subset partition table for modes 0/2 is added alongside those
	// encoders). Matches the hardware decoder: P-bit appended as the endpoint LSB, then bit-replicated to 8 bits.
	void TextureCompressionUtility::DecodeBC7(const u8* block, u8 outRGBA[64])
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

	// BC6H (UF16) decode. Only the modes our encoder emits need a case here; unhandled modes output zero (which
	// collapses PSNR and flags the gap).
	void TextureCompressionUtility::DecodeBC6H_UF16(const u8* block, float out[16][3])
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
} // namespace b3d

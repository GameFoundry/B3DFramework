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
} // namespace b3d

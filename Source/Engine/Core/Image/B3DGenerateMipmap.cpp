//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Image/B3DGenerateMipmap.h"

#include "Image/B3DPixelUtility.h"
#include "Image/B3DPixelData.h"
#include "Renderer/B3DRendererMaterial.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuBuffer.h"
#include "GpuBackend/B3DGpuParameterSet.h"
#include "GpuBackend/B3DGpuPipelineState.h"
#include "CoreObject/B3DRenderThread.h"
#include "B3DApplication.h"

#include <algorithm>

using namespace b3d;

namespace b3d
{
	namespace
	{
		/** RendererMaterial wrapper around the GPU mip-map downsample compute shader. */
		class GenerateMipmapMaterial : public render::RendererMaterial<GenerateMipmapMaterial>
		{
			RMAT_DEF("GenerateMipmap.bsl")

		public:
			GenerateMipmapMaterial() = default;

			/** Records the dispatch that downsamples @p input into @p output. Must run on the render thread. */
			void Execute(render::GpuCommandBuffer& commandBuffer, const TShared<render::GpuBuffer>& input, const TShared<render::GpuBuffer>& output, const TShared<render::GpuBuffer>& params, const Vector2I& dstSize)
			{
				mGpuParameterSet->SetStorageBuffer("gInput", input);
				mGpuParameterSet->SetStorageBuffer("gOutput", output);
				mGpuParameterSet->SetStorageBuffer("gParams", params);

				Bind(commandBuffer);
				commandBuffer.DispatchCompute((u32)Math::DivideAndRoundUp(dstSize.X, 8), (u32)Math::DivideAndRoundUp(dstSize.Y, 8));
			}
		};

		/**
		 * Generates the full mip chain on the GPU. Must run on the render thread: each level is produced by a compute
		 * dispatch that downsamples the previous level, with the result read back to a CPU RGBA32F surface. The
		 * source level (mip 0) is the unfiltered @p interim. Returns false on failure.
		 */
		bool GenerateMipmapsOnRenderThread(GenerateMipmapMaterial* material, const TShared<PixelData>& source, const MipMapGenOptions& options, u32 mipCount, Vector<TShared<PixelData>>& outMipLevels)
		{
			AssertIfNotRenderThread();

			const TShared<GpuDevice> gpuDevice = GetApplication().GetPrimaryGpuDevice();
			if(gpuDevice == nullptr)
				return false;

			i32 filter = 0; // Box
			if(options.Filter == MipMapFilter::Triangle || options.Filter == MipMapFilter::Kaiser)
				filter = 1; // Triangle (Kaiser is approximated by the triangle filter)

			i32 wrapMode = 0; // Mirror
			if(options.WrapMode == MipMapWrapMode::Repeat)
				wrapMode = 1;
			else if(options.WrapMode == MipMapWrapMode::Clamp)
				wrapMode = 2;

			const i32 isSrgb = options.IsSrgb ? 1 : 0;
			const i32 normalize = (options.IsNormalMap && options.NormalizeMipmaps) ? 1 : 0;

			const TShared<render::GpuCommandBufferPool> commandBufferPool = gpuDevice->CreateGpuCommandBufferPool(render::GpuCommandBufferPoolCreateInformation::CreateForThisThread());

			// Mip 0 is the source surface, unfiltered. GetMipmapCount returns the number of mips below the base, so the full chain has mipCount + 1 surfaces.
			outMipLevels.push_back(source);

			TShared<PixelData> previous = source;
			for(u32 mipLevel = 0; mipLevel < mipCount; ++mipLevel)
			{
				const u32 sourceWidth = previous->GetWidth();
				const u32 sourceHeight = previous->GetHeight();
				const u32 destinationWidth = std::max(1u, sourceWidth / 2);
				const u32 destinationHeight = std::max(1u, sourceHeight / 2);

				// Upload the previous level as a GPU-resident input buffer (read as float4 in the shader).
				const TShared<render::GpuBuffer> gpuInput = gpuDevice->CreateGpuBuffer(GpuBufferCreateInformation::CreateSimpleStorage(BF_32X4F, sourceWidth * sourceHeight));
				if(gpuInput == nullptr)
					return false;

				render::GpuBufferUtility::Write(gpuInput, 0, sourceWidth * sourceHeight * sizeof(float) * 4, previous->GetData());

				// Two int4 entries: [0] = (srcW, srcH, dstW, dstH), [1] = (filter, isSrgb, normalize, wrapMode).
				const i32 params[8] =
				{
					(i32)sourceWidth, (i32)sourceHeight, (i32)destinationWidth, (i32)destinationHeight,
					filter, isSrgb, normalize, wrapMode
				};
				const TShared<render::GpuBuffer> paramsBuffer = gpuDevice->CreateGpuBuffer(GpuBufferCreateInformation::CreateSimpleStorage(BF_32X4S, 2));
				if(paramsBuffer == nullptr)
					return false;

				render::GpuBufferUtility::Write(paramsBuffer, 0, sizeof(params), params);

				// Output buffer receives the downsampled level; readable by the CPU and writable as a compute UAV.
				const TShared<render::GpuBuffer> gpuOutput = gpuDevice->CreateGpuBuffer(GpuBufferCreateInformation::CreateSimpleStorage(BF_32X4F, destinationWidth * destinationHeight, GpuBufferFlag::StoreOnCPUWithGPUAccess | GpuBufferFlag::AllowUnorderedAccessOnTheGPU));
				if(gpuOutput == nullptr)
					return false;

				render::GpuCommandBufferCreateInformation commandBufferInfo;
				commandBufferInfo.Name = "GenerateMipmap";
				const TShared<render::GpuCommandBuffer> commandBuffer = commandBufferPool->Create(commandBufferInfo);

				material->Execute(*commandBuffer, gpuInput, gpuOutput, paramsBuffer, Vector2I((i32)destinationWidth, (i32)destinationHeight));

				gpuDevice->SubmitCommandBuffer(commandBuffer);

				// Blocking read-back of the downsampled level (waits on the compute write).
				TShared<PixelData> generatedMipLevel = PixelData::Create(destinationWidth, destinationHeight, 1, PF_RGBA32F);
				render::GpuBufferUtility::Read(gpuOutput, 0, destinationWidth * destinationHeight * sizeof(float) * 4, generatedMipLevel->GetData());

				outMipLevels.push_back(generatedMipLevel);
				previous = generatedMipLevel;
			}

			return true;
		}
	} // anonymous namespace

	bool GpuGenerateMipmap::Generate(const TShared<PixelData>& source, const MipMapGenOptions& options, Vector<TShared<PixelData>>& output)
	{
		if(GetApplication().GetPrimaryGpuDevice() == nullptr)
			return false;

		const u32 mipCount = PixelUtility::GetMipmapCount(source->GetWidth(), source->GetHeight(), 1, source->GetFormat());

		// Compile/fetch the compute shader (blocks until ready).
		GenerateMipmapMaterial* const material = GenerateMipmapMaterial::Get();
		if(material == nullptr || material->GetComputePipeline() == nullptr)
			return false;

		// Convert the source to RGBA32F so downsampling and gamma run at full precision. CPU work, any thread.
		const TShared<PixelData> convertedSource = PixelData::Create(source->GetWidth(), source->GetHeight(), 1, PF_RGBA32F);
		PixelUtility::BulkPixelConversion(*source, *convertedSource);

		// GPU resource creation and dispatch must run on the render thread. Run inline if we're already there,
		// otherwise marshal the work across and block until it finishes.
		Vector<TShared<PixelData>> mips;
		bool success = false;
		auto fnGpuWork = [&]() { success = GenerateMipmapsOnRenderThread(material, convertedSource, options, mipCount, mips); };

		if(B3D_CURRENT_THREAD_ID == GetRenderThread().GetThreadId())
			fnGpuWork();
		else
			GetRenderThread().PostCommand(fnGpuWork, "GPU mipmap generation", true);

		if(!success)
		{
			B3D_LOG(Warning, LogPixelUtility, "GPU mipmap generation failed. Falling back.");
			return false;
		}

		// Convert each RGBA32F level back to the source format.
		output.reserve(mips.size());
		for(const TShared<PixelData>& mip : mips)
		{
			TShared<PixelData> convertedMip = PixelData::Create(mip->GetWidth(), mip->GetHeight(), 1, source->GetFormat());
			PixelUtility::BulkPixelConversion(*mip, *convertedMip);
			output.push_back(convertedMip);
		}

		return true;
	}
} // namespace b3d

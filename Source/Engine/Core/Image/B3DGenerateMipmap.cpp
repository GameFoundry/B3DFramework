//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Image/B3DGenerateMipmap.h"

#include "Image/B3DPixelUtility.h"
#include "Image/B3DPixelData.h"
#include "Renderer/B3DRendererMaterial.h"
#include "Renderer/B3DGpuUniformBuffer.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuBuffer.h"
#include "GpuBackend/B3DGpuParameterSet.h"
#include "GpuBackend/B3DGpuPipelineState.h"
#include "CoreObject/B3DRenderThread.h"
#include "Threading/B3DAsyncOp.h"
#include "FileSystem/B3DDataStream.h"
#include "B3DApplication.h"

#include <algorithm>
#include <cstring>

using namespace b3d;

namespace b3d::render
{
	namespace
	{
		/** Per-dispatch constants for the mip-map downsample kernel. */
		B3D_UNIFORM_BUFFER_BEGIN(MipmapParamsDefinition)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2I, gSourceSize) // (srcWidth, srcHeight)
			B3D_UNIFORM_BUFFER_MEMBER(Vector2I, gDestSize)   // (dstWidth, dstHeight)
			B3D_UNIFORM_BUFFER_MEMBER(i32, gFilter)          // 0 = box, 1 = triangle
			B3D_UNIFORM_BUFFER_MEMBER(i32, gIsSrgb)          // non-zero: source is sRGB
			B3D_UNIFORM_BUFFER_MEMBER(i32, gNormalize)       // non-zero: re-normalize as a normal vector
			B3D_UNIFORM_BUFFER_MEMBER(i32, gWrapMode)        // 0 = mirror, 1 = repeat, 2 = clamp
		B3D_UNIFORM_BUFFER_END

		MipmapParamsDefinition gMipmapParameterDefinition;
	}
}

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

			void Initialize() override
			{
				mGpuParameterSet->TryGetUniformBufferParameter("Parameters", mParametersBuffer);
			}

			/** Records the dispatch that downsamples @p input into @p output. Must run on the render thread. */
			void Execute(render::GpuCommandBuffer& commandBuffer, const TShared<render::GpuBuffer>& input, const TShared<render::GpuBuffer>& output, const render::GpuBufferMappedScope& parameters, const Vector2I& dstSize)
			{
				mGpuParameterSet->SetStorageBuffer("gInput", input);
				mGpuParameterSet->SetStorageBuffer("gOutput", output);
				mParametersBuffer.Set(parameters);

				Bind(commandBuffer);
				commandBuffer.DispatchCompute((u32)Math::DivideAndRoundUp(dstSize.X, 8), (u32)Math::DivideAndRoundUp(dstSize.Y, 8));
			}

		private:
			render::GpuParameterUniformBuffer mParametersBuffer;
		};

		/**
		 * Records the full mip chain on the GPU and sets up an asynchronous read-back. Must run on the render thread.
		 *
		 * Every level is produced by a compute dispatch that downsamples the previous level. The levels are chained
		 * entirely on the GPU - level N's output buffer is bound as level N+1's input (with a barrier between), so there is
		 * no CPU round-trip between dispatches. All dispatches run on a single command buffer; after submission each
		 * generated level is read back via GpuBufferUtility::ReadAsync, and a completion callback assembles the chain (each
		 * level converted to @p sourceFormat, with @p mip0 prepended) and completes @p op. On failure @p op is completed
		 * with an empty vector. The render thread is never blocked on the GPU.
		 */
		void GenerateMipmapsOnRenderThread(const TShared<PixelData>& convertedSource, const TShared<PixelData>& mip0, PixelFormat sourceFormat, const MipMapGenOptions& options, u32 mipCount, TAsyncOp<Vector<TShared<PixelData>>> op)
		{
			AssertIfNotRenderThread();

			const TShared<GpuDevice> gpuDevice = GetApplication().GetPrimaryGpuDevice();
			if(gpuDevice == nullptr)
			{
				op.CompleteOperation(Vector<TShared<PixelData>>());
				return;
			}

			// Compile/fetch the compute shader - render-thread only (RendererMaterial access is not thread-safe). Blocks
			// until the shader is compiled.
			GenerateMipmapMaterial* const material = GenerateMipmapMaterial::Get();
			if(material == nullptr || material->GetComputePipeline() == nullptr)
			{
				op.CompleteOperation(Vector<TShared<PixelData>>());
				return;
			}

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

			render::GpuCommandBufferCreateInformation commandBufferInfo;
			commandBufferInfo.Name = "GenerateMipmap";

			const TShared<render::GpuCommandBuffer> commandBuffer = commandBufferPool->Create(commandBufferInfo);

			// Upload mip 0 (the RGBA32F-converted source) as the first input buffer; read as float4 in the shader.
			const u32 baseWidth = convertedSource->GetWidth();
			const u32 baseHeight = convertedSource->GetHeight();
			const TShared<render::GpuBuffer> inputBuffer = gpuDevice->CreateGpuBuffer(GpuBufferCreateInformation::CreateSimpleStorage(BF_32X4F, baseWidth * baseHeight, GpuBufferFlag::StoreOnGPU | GpuBufferFlag::Transient));
			if(inputBuffer == nullptr)
			{
				op.CompleteOperation(Vector<TShared<PixelData>>());
				return;
			}

			render::GpuBufferUtility::Write(inputBuffer, 0, baseWidth * baseHeight * sizeof(float) * 4, convertedSource->GetData());

			// Generate levels 1..mipCount, each downsampled from the previous level's GPU buffer (no CPU round-trip).
			TInlineArray<TShared<render::GpuBuffer>, 16> levelBuffers; // Generated level outputs, in order
			TInlineArray<Size2UI, 16> levelDimensions; // (width, height) of each generated level
			TInlineArray<TShared<render::GpuBuffer>, 16> parameterBuffers; // Transient per-dispatch uniform buffers; kept alive until the GPU completes

			TShared<render::GpuBuffer> previous = inputBuffer;
			u32 sourceWidth = baseWidth;
			u32 sourceHeight = baseHeight;
			for(u32 mipLevel = 0; mipLevel < mipCount; ++mipLevel)
			{
				const u32 destinationWidth = std::max(1u, sourceWidth / 2);
				const u32 destinationHeight = std::max(1u, sourceHeight / 2);

				// Output for this level: written as a compute UAV, read back by the CPU, and bound as the next level's input (read as a storage buffer).
				const TShared<render::GpuBuffer> output = gpuDevice->CreateGpuBuffer(GpuBufferCreateInformation::CreateSimpleStorage(BF_32X4F, destinationWidth * destinationHeight, GpuBufferFlag::StoreOnCPUWithGPUAccess | GpuBufferFlag::AllowUnorderedAccessOnTheGPU | GpuBufferFlag::Transient));
				if(output == nullptr)
				{
					op.CompleteOperation(Vector<TShared<PixelData>>());
					return;
				}

				// Per-dispatch constants for this level, in a transient uniform buffer (linear-allocator backed, thread-safe,
				// frame-lifetime). The buffer must be kept alive until the GPU completes, so it is stored in parameterBuffers
				// and captured by the completion callback. The mapped scope is unmapped (flushed) just before the dispatch.
				const TShared<render::GpuBuffer> parameterBuffer = render::gMipmapParameterDefinition.CreateTransientBuffer();
				if(parameterBuffer == nullptr)
				{
					op.CompleteOperation(Vector<TShared<PixelData>>());
					return;
				}

				render::GpuBufferMappedScope parameters = parameterBuffer->Map(GpuMapOption::Write);
				render::gMipmapParameterDefinition.gSourceSize.Set(parameters, Vector2I((i32)sourceWidth, (i32)sourceHeight));
				render::gMipmapParameterDefinition.gDestSize.Set(parameters, Vector2I((i32)destinationWidth, (i32)destinationHeight));
				render::gMipmapParameterDefinition.gFilter.Set(parameters, filter);
				render::gMipmapParameterDefinition.gIsSrgb.Set(parameters, isSrgb);
				render::gMipmapParameterDefinition.gNormalize.Set(parameters, normalize);
				render::gMipmapParameterDefinition.gWrapMode.Set(parameters, wrapMode);

				parameters.Unmap();

				material->Execute(*commandBuffer, previous, output, parameters, Vector2I((i32)destinationWidth, (i32)destinationHeight));

				parameterBuffers.Add(parameterBuffer);
				levelBuffers.Add(output);
				levelDimensions.Add(Size2UI(destinationWidth, destinationHeight));

				previous = output;
				sourceWidth = destinationWidth;
				sourceHeight = destinationHeight;
			}

			// Queue a non-blocking read-back of every generated level on the same command buffer. Each ReadAsync connects a
			// completion callback that fills a MemoryDataStream; the finalize callback below is connected last, so by the
			// time it runs (events fire in registration order) every read-back has completed.
			TInlineArray<TAsyncOp<TShared<MemoryDataStream>>, 16> readOps;
			readOps.reserve(mipCount);
			for(u32 mipLevel = 0; mipLevel < mipCount; ++mipLevel)
			{
				const u32 byteCount = levelDimensions[mipLevel].Width * levelDimensions[mipLevel].Height * (u32)sizeof(float) * 4u;
				readOps.Add(render::GpuBufferUtility::ReadAsync(levelBuffers[mipLevel], 0, byteCount, *commandBuffer));
			}

			// Assemble the chain once the GPU finishes. Captures keep the level buffers (read-back sources) and read ops
			// alive until completion. Runs on the render thread (the command buffer's owner thread).
			commandBuffer->OnDidComplete.Connect(
				[op, readOps, levelBuffers, inputBuffer, parameterBuffers, levelDimensions, sourceFormat, mip0]() mutable
				{
					Vector<TShared<PixelData>> outMipLevels;
					outMipLevels.reserve(levelDimensions.size() + 1);
					outMipLevels.push_back(mip0);

					for(u32 mipLevel = 0; mipLevel < (u32)levelDimensions.size(); ++mipLevel)
					{
						const TShared<MemoryDataStream> data = readOps[mipLevel].GetReturnValue();
						if(data == nullptr)
						{
							op.CompleteOperation(Vector<TShared<PixelData>>());
							return;
						}

						const u32 width = levelDimensions[mipLevel].Width;
						const u32 height = levelDimensions[mipLevel].Height;

						const TShared<PixelData> rgbaLevel = PixelData::Create(width, height, 1, PF_RGBA32F);
						memcpy(rgbaLevel->GetData(), data->Data(), (size_t)width * height * sizeof(float) * 4);

						const TShared<PixelData> convertedLevel = PixelData::Create(width, height, 1, sourceFormat);
						PixelUtility::BulkPixelConversion(*rgbaLevel, *convertedLevel);
						outMipLevels.push_back(convertedLevel);
					}

					op.CompleteOperation(std::move(outMipLevels));
				});

			gpuDevice->SubmitCommandBuffer(commandBuffer);
		}
	} // anonymous namespace

	TAsyncOp<Vector<TShared<PixelData>>> GpuGenerateMipmap::Generate(const TShared<PixelData>& source, const MipMapGenOptions& options)
	{
		// On failure we still return a *completed* op (with an empty chain) so a caller blocking on it never hangs.
		auto fnFailed = []()
		{
			TAsyncOp<Vector<TShared<PixelData>>> op;
			op.CompleteOperation(Vector<TShared<PixelData>>());
			return op;
		};

		if(GetApplication().GetPrimaryGpuDevice() == nullptr)
			return fnFailed();

		const u32 mipCount = PixelUtility::GetMipmapCount(source->GetWidth(), source->GetHeight(), 1, source->GetFormat());

		// CPU work, any thread: convert the source to RGBA32F so downsampling and gamma run at full precision, and make a
		// distinct mip-0 surface in the source format (the chain's first level is the unfiltered source).
		const PixelFormat sourceFormat = source->GetFormat();
		const TShared<PixelData> convertedSource = PixelData::Create(source->GetWidth(), source->GetHeight(), 1, PF_RGBA32F);
		PixelUtility::BulkPixelConversion(*source, *convertedSource);

		const TShared<PixelData> mip0 = PixelData::Create(source->GetWidth(), source->GetHeight(), 1, sourceFormat);
		PixelUtility::BulkPixelConversion(*source, *mip0);

		// GPU resource creation and dispatch must run on the render thread. The work is non-blocking: it fetches the
		// material, records the dispatches, queues the read-backs and returns immediately; the op completes from a
		// command-buffer completion callback once the GPU finishes. Run inline if we are already on the render thread,
		// otherwise marshal it across.
		TAsyncOp<Vector<TShared<PixelData>>> op;
		auto fnGpuWork = [convertedSource, mip0, sourceFormat, options, mipCount, op]() mutable
		{
			GenerateMipmapsOnRenderThread(convertedSource, mip0, sourceFormat, options, mipCount, op);
		};

		if(B3D_CURRENT_THREAD_ID == GetRenderThread().GetThreadId())
			fnGpuWork();
		else
			GetRenderThread().PostCommand(std::move(fnGpuWork), "GPU mipmap generation");

		return op;
	}
} // namespace b3d

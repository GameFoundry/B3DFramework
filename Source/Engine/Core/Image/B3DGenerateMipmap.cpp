//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Image/B3DGenerateMipmap.h"

#include "Image/B3DPixelUtility.h"
#include "Image/B3DPixelData.h"
#include "Renderer/B3DRenderer.h"
#include "Renderer/B3DRendererMaterial.h"
#include "Renderer/B3DGpuUniformBuffer.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuBuffer.h"
#include "GpuBackend/B3DGpuParameterSet.h"
#include "GpuBackend/B3DGpuPipelineState.h"
#include "FrameGraph/B3DFrameGraph.h"
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
				mParameterLayout->GetStorageBufferParameter("gInput", mInputParameter);
				mParameterLayout->GetStorageBufferParameter("gOutput", mOutputParameter);
				mParameterLayout->TryGetUniformBufferParameter("Parameters", mParametersBuffer);
			}

			/** Creates the parameter set of @p pass, which downsamples @p input into @p output. Returns the set to pass to Execute(). */
			TShared<render::GpuParameterSet> Prepare(render::FrameGraphPass& pass, const TShared<render::GpuBuffer>& input, const TShared<render::GpuBuffer>& output, const render::GpuBufferMappedScope& parameters)
			{
				const TShared<render::GpuParameterSet> set = CreateGpuParameterSet(pass);
				mInputParameter.Set(set, input);
				mOutputParameter.Set(set, output);
				mParametersBuffer.Set(set, parameters);

				return set;
			}

			/** Binds @p set, from Prepare(), and records the dispatch that writes a level of @p destinationSize. */
			void Execute(render::FrameGraphPassContext& context, const TShared<render::GpuParameterSet>& set, const Vector2I& destinationSize) const
			{
				render::GpuCommandBuffer& commandBuffer = context.GetCommandBuffer();

				Bind(commandBuffer, set);
				commandBuffer.DispatchCompute((u32)Math::DivideAndRoundUp(destinationSize.X, 8), (u32)Math::DivideAndRoundUp(destinationSize.Y, 8));
			}

		private:
			render::GpuParameterStorageBuffer mInputParameter;
			render::GpuParameterStorageBuffer mOutputParameter;
			render::GpuParameterUniformBuffer mParametersBuffer;
		};

		/**
		 * Generates the full mip chain on the GPU, in one frame graph, and sets up an asynchronous read-back. Must run on the render thread.
		 *
		 * Every level is produced by a compute pass that downsamples the previous level. The levels are chained entirely on the GPU, as
		 * transient buffers of the graph: level N's output is level N+1's input, so there is no CPU round-trip between dispatches. A last
		 * copy pass reads every level back into one staging buffer, and a completion callback assembles the chain (each level converted to
		 * @p sourceFormat, with @p mip0 prepended) and completes @p op. On failure @p op is completed with an empty vector. The render
		 * thread waits for the GPU before returning, so @p op is complete by then.
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

			// A source without smaller levels needs no GPU work
			if(mipCount == 0)
			{
				op.CompleteOperation(Vector<TShared<PixelData>>({ mip0 }));
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

			GpuWorkContext& gpuContext = render::GetRenderer()->GetGpuContext();

			// Upload mip 0 (the RGBA32F-converted source) as the first input buffer; read as float4 in the shader. The upload goes on the
			// transfer command buffer, which is submitted before the graph. The graph only reads the buffer, so it is not imported.
			const u32 baseWidth = convertedSource->GetWidth();
			const u32 baseHeight = convertedSource->GetHeight();
			const TShared<render::GpuBuffer> inputBuffer = gpuContext.CreateScratchGpuBuffer(GpuBufferCreateInformation::CreateSimpleStorage(BF_32X4F, baseWidth * baseHeight, GpuBufferFlag::StoreOnGPU));
			if(inputBuffer == nullptr)
			{
				op.CompleteOperation(Vector<TShared<PixelData>>());
				return;
			}

			render::GpuBufferUtility::Write(gpuContext, inputBuffer, 0, baseWidth * baseHeight * sizeof(float) * 4, convertedSource->GetData());

			render::FrameGraph graph(gpuContext, "GenerateMipmap");

			// Generate levels 1..mipCount, each downsampled from the previous level's GPU buffer (no CPU round-trip).
			TInlineArray<TShared<render::GpuBuffer>, 16> levelBuffers; // Generated level outputs, in order
			TInlineArray<Size2UI, 16> levelDimensions; // (width, height) of each generated level
			TInlineArray<TShared<render::GpuBuffer>, 16> parameterBuffers; // Scratch per-dispatch uniform buffers; kept alive until the GPU completes
			u32 readbackByteCount = 0;

			TShared<render::GpuBuffer> previous = inputBuffer;
			u32 sourceWidth = baseWidth;
			u32 sourceHeight = baseHeight;
			for(u32 mipLevel = 0; mipLevel < mipCount; ++mipLevel)
			{
				const u32 destinationWidth = std::max(1u, sourceWidth / 2);
				const u32 destinationHeight = std::max(1u, sourceHeight / 2);

				// Output for this level: written by this level's pass, then read by the next level's pass and by the read-back
				const TShared<render::GpuBuffer> output = graph.CreateBuffer("GenerateMipmapLevel", GpuBufferCreateInformation::CreateSimpleStorage(BF_32X4F, destinationWidth * destinationHeight, GpuBufferFlag::StoreOnGPU | GpuBufferFlag::AllowUnorderedAccessOnTheGPU));

				// Per-dispatch constants for this level, in a scratch uniform buffer (linear-allocator backed, thread-safe,
				// frame-lifetime). The buffer must be kept alive until the GPU completes, so it is stored in parameterBuffers
				// and captured by the completion callback. The mapped scope is unmapped (flushed) before the graph executes.
				const TShared<render::GpuBuffer> parameterBuffer = render::gMipmapParameterDefinition.CreateScratchBuffer(gpuContext);
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

				render::FrameGraphComputePass pass;
				const TShared<render::GpuParameterSet> set = material->Prepare(pass, previous, output, parameters);

				const Vector2I destinationSize((i32)destinationWidth, (i32)destinationHeight);
				graph.AddComputePass("GenerateMipmap", std::move(pass), [material, set, destinationSize](render::FrameGraphPassContext& context)
				{
					material->Execute(context, set, destinationSize);
				});

				parameterBuffers.Add(parameterBuffer);
				levelBuffers.Add(output);
				levelDimensions.Add(Size2UI(destinationWidth, destinationHeight));
				readbackByteCount += destinationWidth * destinationHeight * (u32)sizeof(float) * 4u;

				previous = output;
				sourceWidth = destinationWidth;
				sourceHeight = destinationHeight;
			}

			// Read every generated level back into one staging buffer, one level after another
			const TShared<render::GpuBuffer> stagingBuffer = gpuDevice->CreateGpuBuffer(GpuBufferCreateInformation::CreateStagingRead(readbackByteCount));
			if(stagingBuffer == nullptr)
			{
				op.CompleteOperation(Vector<TShared<PixelData>>());
				return;
			}

			graph.ImportBuffer(stagingBuffer);

			render::FrameGraphCopyPass readbackPass;
			for(const TShared<render::GpuBuffer>& levelBuffer : levelBuffers)
				readbackPass.AddSource(levelBuffer);

			readbackPass.AddDestination(stagingBuffer);

			graph.AddCopyPass("GenerateMipmapReadback", std::move(readbackPass),
				[op, levelBuffers, levelDimensions, stagingBuffer, inputBuffer, parameterBuffers, sourceFormat, mip0](render::FrameGraphPassContext& context)
			{
				render::GpuCommandBuffer& commandBuffer = context.GetCommandBuffer();

				u32 stagingOffset = 0;
				for(u32 mipLevel = 0; mipLevel < (u32)levelBuffers.Size(); ++mipLevel)
				{
					const u32 byteCount = levelDimensions[mipLevel].Width * levelDimensions[mipLevel].Height * (u32)sizeof(float) * 4u;
					commandBuffer.CopyBufferToBuffer(levelBuffers[mipLevel], stagingBuffer, 0, stagingOffset, byteCount);
					stagingOffset += byteCount;
				}

				// Assemble the chain once the GPU finishes. Captures keep the staging buffer, mip 0's buffer and the uniform buffers
				// alive until completion. Runs on the render thread (the command buffer's owner thread).
				commandBuffer.OnDidComplete.Connect(
					[op, levelDimensions, stagingBuffer, inputBuffer, parameterBuffers, sourceFormat, mip0]() mutable
					{
						const render::GpuBufferMappedScope mapping = stagingBuffer->Map(GpuMapOption::Read);
						const u8* levelData = (const u8*)mapping.GetMappedMemory();

						Vector<TShared<PixelData>> outMipLevels;
						outMipLevels.reserve(levelDimensions.size() + 1);
						outMipLevels.push_back(mip0);

						for(u32 mipLevel = 0; mipLevel < (u32)levelDimensions.size(); ++mipLevel)
						{
							const u32 width = levelDimensions[mipLevel].Width;
							const u32 height = levelDimensions[mipLevel].Height;
							const size_t byteCount = (size_t)width * height * sizeof(float) * 4;

							const TShared<PixelData> rgbaLevel = PixelData::Create(width, height, 1, PF_RGBA32F);
							memcpy(rgbaLevel->GetData(), levelData, byteCount);
							levelData += byteCount;

							const TShared<PixelData> convertedLevel = PixelData::Create(width, height, 1, sourceFormat);
							PixelUtility::BulkPixelConversion(*rgbaLevel, *convertedLevel);
							outMipLevels.push_back(convertedLevel);
						}

						op.CompleteOperation(std::move(outMipLevels));
					});

				// A command buffer destroyed without being submitted never completes
				commandBuffer.OnDestroyed.Connect([op](bool isSubmitted) mutable
				{
					if(!isSubmitted)
						op.CompleteOperation(Vector<TShared<PixelData>>());
				});
			});

			graph.Compile();
			graph.Execute();

			// Command buffer completion is otherwise only processed at the end of a frame, and callers such as importers block on the op
			// without frames running. Waiting runs the completion callback.
			gpuDevice->WaitUntilIdle();
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

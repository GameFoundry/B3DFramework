//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DFrameGraphTestSuite.h"
#include "Utility/B3DUnitTestGpuPipelines.h"
#include "Utility/B3DGpuBarrierTestUtility.h"
#include "CoreObject/B3DRenderThread.h"
#include "FrameGraph/B3DFrameGraph.h"
#include "GpuBackend/B3DGpuBackend.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuDeviceCapabilities.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuParameterSet.h"
#include "GpuBackend/B3DGpuParameterSetPool.h"
#include "GpuBackend/B3DGpuPipelineState.h"
#include "GpuBackend/B3DGpuPipelineParameterLayout.h"
#include "GpuBackend/B3DGpuTransientResourceAllocator.h"
#include "GpuBackend/B3DGpuWorkContext.h"
#include "Profiling/B3DProfilerGPU.h"
#include "Renderer/B3DRendererUtility.h"
#include "FileSystem/B3DDataStream.h"
#include "Image/B3DPixelData.h"
#include "Image/B3DTexture.h"
#include "Threading/B3DAsyncOp.h"

using namespace b3d;
using namespace b3d::render;

FrameGraphTestSuite::FrameGraphTestSuite()
	: TestSuite("FrameGraphTestSuite")
{
	B3D_ADD_TEST(FrameGraphTestSuite::TestSplitBarriers)
	B3D_ADD_TEST(FrameGraphTestSuite::TestCulling)
	B3D_ADD_TEST(FrameGraphTestSuite::TestSplitAliasAcquire)
	B3D_ADD_TEST(FrameGraphTestSuite::TestCrossGraphOrdering)
	B3D_ADD_TEST(FrameGraphTestSuite::TestExtract)
	B3D_ADD_TEST(FrameGraphTestSuite::TestExport)
	B3D_ADD_TEST(FrameGraphTestSuite::TestRestingReads)
	B3D_ADD_TEST(FrameGraphTestSuite::TestLateMemory)
	B3D_ADD_TEST(FrameGraphTestSuite::TestReadback)
	B3D_ADD_TEST(FrameGraphTestSuite::TestParameterHandles)
	B3D_ADD_TEST(FrameGraphTestSuite::TestProfiling)
	B3D_ADD_TEST(FrameGraphTestSuite::TestRasterPass)
	B3D_ADD_TEST(FrameGraphTestSuite::TestDepthReadOnly)
	B3D_ADD_TEST(FrameGraphTestSuite::TestTwoAspectAliasAcquire)
	B3D_ADD_TEST(FrameGraphTestSuite::TestExecuteCallbacks)

#if B3D_BUILD_TYPE_DEVELOPMENT
	B3D_ADD_TEST(FrameGraphTestSuite::TestValidation)
#endif
}

namespace
{
	constexpr u32 kSize = 16; /**< Width and height of the resources the test dispatch writes. */
	constexpr u32 kDispatchPattern = 0x5A000000; /**< High bits the test dispatch writes into every element. */

	/** D3D12 debug layer hint about the command buffers that only bring resources to rest. */
	constexpr const char* kBarrierOnlyWarning = "references command lists that have recorded only Barrier commands";

	/** Name of the test graphs. */
	constexpr const char* kGraphName = "Frame graph test";

	/** Resources and helpers the frame graph tests share. Lives on the render thread. */
	struct FrameGraphTestContext
	{
		FrameGraphTestContext(GpuDevice& device, const TShared<GpuComputePipelineState>& pipeline, const TShared<GpuComputePipelineState>& samplePipeline = nullptr)
			: Device(device), Pipeline(pipeline), SamplePipeline(samplePipeline), Context(GpuWorkContext::Create(device))
		{
			Pool = GpuBarrierTestUtility::CreateCommandBufferPool(device);

			TextureInformation.Width = kSize;
			TextureInformation.Height = kSize;
			TextureInformation.Format = PF_R32U;
			TextureInformation.Usage = TextureUsageFlag::AllowUnorderedAccessOnTheGPU;

			const ImageSubresourcePitch pitch = device.CreateTexture(TextureInformation)->GetStagingBufferPitchForSubresource(0, 0);
			TextureRowPitch = pitch.RowPitch;
			TextureReadbackSize = pitch.RowPitch * pitch.SliceHeight * sizeof(u32);
		}

		/** Creates a buffer the test dispatch can write. */
		TShared<render::GpuBuffer> CreateBuffer(const char* name) const
		{
			const TShared<render::GpuBuffer> buffer = Device.CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32), kSize * kSize,
				GpuBufferFlag::StoreOnGPU | GpuBufferFlag::AllowUnorderedAccessOnTheGPU));
			buffer->SetName(name);
			return buffer;
		}

		/** Creates a texture with @p usage, which the test dispatch can write by default. It starts at rest. */
		TShared<render::Texture> CreateTexture(const char* name, TextureUsageFlags usage = TextureUsageFlag::AllowUnorderedAccessOnTheGPU) const
		{
			TextureCreateInformation information = TextureInformation;
			information.Name = name;
			information.Usage = usage;

			return Device.CreateTexture(information);
		}

		/** Creates a buffer the host can read @p size bytes from. */
		TShared<render::GpuBuffer> CreateReadback(u32 size) const
		{
			return Device.CreateGpuBuffer(GpuBufferCreateInformation::CreateStagingRead(size));
		}

		/** Adds a pass that dispatches the test pipeline into @p buffer and @p texture, and exports both after it if @p exports is true. */
		void AddDispatch(FrameGraph& graph, const char* name, const TShared<render::GpuBuffer>& buffer, const TShared<render::Texture>& texture, bool exports = false) const
		{
			FrameGraphComputePass pass;
			const TShared<render::GpuParameterSet> parameters = Context->GetParameterSetPool().Create(Pipeline->GetParameterLayout()->GetSet(0), 0);
			pass.AddParameterSet(parameters);
			parameters->SetStorageBuffer("OutputBuffer", buffer);
			parameters->SetStorageTexture("OutputTexture", texture, TextureSurface());

			if(exports)
			{
				pass.Export(buffer);
				pass.Export(texture);
			}

			const TShared<GpuComputePipelineState> pipeline = Pipeline;
			graph.AddComputePass(name, std::move(pass), [pipeline, parameters](FrameGraphPassContext& passContext)
			{
				render::GpuCommandBuffer& commands = passContext.GetCommandBuffer();
				commands.SetGpuComputePipelineState(pipeline);
				commands.SetGpuParameterSet(parameters);
				commands.DispatchCompute(1, 1, 1);
			});
		}

		/** Adds a pass that samples @p texture with the test sample pipeline, and writes its texels into @p buffer. */
		void AddSample(FrameGraph& graph, const char* name, const TShared<render::Texture>& texture, const TShared<render::GpuBuffer>& buffer) const
		{
			FrameGraphComputePass pass;
			const TShared<render::GpuParameterSet> parameters = Context->GetParameterSetPool().Create(SamplePipeline->GetParameterLayout()->GetSet(0), 0);
			pass.AddParameterSet(parameters);
			parameters->SetSampledTexture("InputTexture", texture);
			parameters->SetStorageBuffer("OutputBuffer", buffer);

			const TShared<GpuComputePipelineState> pipeline = SamplePipeline;
			graph.AddComputePass(name, std::move(pass), [pipeline, parameters](FrameGraphPassContext& passContext)
			{
				render::GpuCommandBuffer& commands = passContext.GetCommandBuffer();
				commands.SetGpuComputePipelineState(pipeline);
				commands.SetGpuParameterSet(parameters);
				commands.DispatchCompute(1, 1, 1);
			});
		}

		/** Adds a pass that copies @p buffer into @p readback, which the graph imports. */
		void AddReadback(FrameGraph& graph, const char* name, const TShared<render::GpuBuffer>& buffer, const TShared<render::GpuBuffer>& readback) const
		{
			graph.ImportBuffer(readback);

			FrameGraphCopyPass pass;
			pass.AddSource(buffer);
			pass.AddDestination(readback);

			graph.AddCopyPass(name, std::move(pass), [buffer, readback](FrameGraphPassContext& passContext)
			{
				passContext.GetCommandBuffer().CopyBufferToBuffer(buffer, readback, 0, 0, kSize * kSize * sizeof(u32));
			});
		}

		/** Adds a pass that copies @p buffer and @p texture into @p bufferReadback and @p textureReadback, which the graph imports. */
		void AddReadback(FrameGraph& graph, const char* name, const TShared<render::GpuBuffer>& buffer, const TShared<render::Texture>& texture, const TShared<render::GpuBuffer>& bufferReadback,
			const TShared<render::GpuBuffer>& textureReadback) const
		{
			graph.ImportBuffer(bufferReadback);
			graph.ImportBuffer(textureReadback);

			FrameGraphCopyPass pass;
			pass.AddSource(buffer);
			pass.AddSource(texture);
			pass.AddDestination(bufferReadback);
			pass.AddDestination(textureReadback);

			graph.AddCopyPass(name, std::move(pass), [buffer, texture, bufferReadback, textureReadback](FrameGraphPassContext& passContext)
			{
				render::GpuCommandBuffer& commands = passContext.GetCommandBuffer();
				commands.CopyBufferToBuffer(buffer, bufferReadback, 0, 0, kSize * kSize * sizeof(u32));
				commands.CopyTextureToBuffer(texture, textureReadback, 0, 0, 0);
			});
		}

		/**
		 * Returns true if @p readback holds the test dispatch's values, laid out as the buffer, or as a texture copy if @p isTexture is true.
		 * A graph's end returns the readback to rest, which makes the copy visible to the host. Wait for the GPU to finish first.
		 */
		bool HasDispatchValues(const TShared<render::GpuBuffer>& readback, bool isTexture) const
		{
			const render::GpuBufferMappedScope mapping = readback->Map(GpuMapOption::Read);
			if(!mapping.IsValid())
				return false;

			return HasDispatchValues(static_cast<const u32*>(mapping.GetMappedMemory()), isTexture ? TextureRowPitch : kSize);
		}

		/** Returns true if @p elements hold the test dispatch's values, in rows @p rowPitch elements apart. */
		static bool HasDispatchValues(const u32* elements, u32 rowPitch)
		{
			for(u32 row = 0; row < kSize; row++)
			{
				for(u32 column = 0; column < kSize; column++)
				{
					if(elements[row * rowPitch + column] != (kDispatchPattern | (row * kSize + column)))
						return false;
				}
			}

			return true;
		}

		GpuDevice& Device;
		TShared<GpuComputePipelineState> Pipeline;
		TShared<GpuComputePipelineState> SamplePipeline; /**< Pipeline of AddSample(). */
		TShared<GpuWorkContext> Context;
		TShared<GpuCommandBufferPool> Pool;
		TextureCreateInformation TextureInformation;
		u32 TextureRowPitch = 0;
		u32 TextureReadbackSize = 0;
	};

	/** Returns the device the tests run on, or null if there is none. */
	TShared<GpuDevice> GetTestDevice()
	{
		GpuBackend& backend = GpuBackend::Instance();
		if(backend.GetDeviceCount() == 0)
			return nullptr;

		return backend.GetDevice(0);
	}

	/** Runs @p test on the render thread and waits for it to finish. */
	void RunOnRenderThread(const char* name, const std::function<void()>& test)
	{
		GetRenderThread().PostCommand([&test]() { test(); }, name, true);
	}
} // namespace

void FrameGraphTestSuite::TestSplitBarriers()
{
	const TShared<GpuDevice> device = GetTestDevice();
	const TShared<GpuComputePipelineState> pipeline = device != nullptr ? CreateTestDispatchPipeline(*device) : nullptr;
	if(pipeline == nullptr)
		return;

	LoggingScope logs(*this);
	logs.IgnoreWarning(kBarrierOnlyWarning);

	RunOnRenderThread("FrameGraphTestSuite::TestSplitBarriers", [this, &device, &pipeline]()
	{
		const FrameGraphTestContext test(*device, pipeline);
		const TShared<render::GpuBuffer> producedBuffer = test.CreateBuffer("Produced buffer");
		const TShared<render::Texture> producedTexture = test.CreateTexture("Produced texture");
		const TShared<render::GpuBuffer> bufferReadback = test.CreateReadback(kSize * kSize * sizeof(u32));
		const TShared<render::GpuBuffer> textureReadback = test.CreateReadback(test.TextureReadbackSize);

		{
			FrameGraph graph(*test.Context, kGraphName);
			graph.ImportBuffer(producedBuffer);
			graph.ImportTexture(producedTexture);
			test.AddDispatch(graph, "Producer", producedBuffer, producedTexture);

			// Imported resources are observed after the graph, so the unrelated passes survive
			for(const char* name : { "Unrelated 1", "Unrelated 2" })
			{
				const TShared<render::GpuBuffer> buffer = test.CreateBuffer(name);
				const TShared<render::Texture> texture = test.CreateTexture(name);
				graph.ImportBuffer(buffer);
				graph.ImportTexture(texture);
				test.AddDispatch(graph, name, buffer, texture);
			}

			test.AddReadback(graph, "Consumer", producedBuffer, producedTexture, bufferReadback, textureReadback);
			graph.Compile();

			// The producer's writes reach the consumer through one batch of split barriers
			const FrameGraphStatistics& statistics = graph.GetStatistics();
			B3D_TEST_ASSERT(statistics.CulledPassCount == 0)
			B3D_TEST_ASSERT(statistics.SplitBarrierCount >= 2)
			B3D_TEST_ASSERT(statistics.BarrierBatchCount >= 1)

			graph.Execute();
		}

		device->WaitUntilIdle();
		B3D_TEST_ASSERT_MSG(test.HasDispatchValues(bufferReadback, false), "The consumer does not read what the producer wrote into the buffer.")
		B3D_TEST_ASSERT_MSG(test.HasDispatchValues(textureReadback, true), "The consumer does not read what the producer wrote into the texture.")
		B3D_TEST_ASSERT(producedBuffer->GetFrameGraphResourceIndex() == ~0u && producedTexture->GetFrameGraphResourceIndex() == ~0u)

		device->EndFrame();
	});
}

void FrameGraphTestSuite::TestCulling()
{
	const TShared<GpuDevice> device = GetTestDevice();
	const TShared<GpuComputePipelineState> pipeline = device != nullptr ? CreateTestDispatchPipeline(*device) : nullptr;
	if(pipeline == nullptr)
		return;

	LoggingScope logs(*this);
	logs.IgnoreWarning(kBarrierOnlyWarning);

	RunOnRenderThread("FrameGraphTestSuite::TestCulling", [this, &device, &pipeline]()
	{
		const FrameGraphTestContext test(*device, pipeline);

		TextureCreateInformation targetInformation;
		targetInformation.Width = kSize;
		targetInformation.Height = kSize;
		targetInformation.Format = PF_RGBA8;
		targetInformation.Usage = TextureUsageFlag::RenderTarget;
		targetInformation.ClearColor = Color(1.0f, 0.0f, 0.0f, 1.0f);

		const ImageSubresourcePitch pitch = device->CreateTexture(targetInformation)->GetStagingBufferPitchForSubresource(0, 0);
		const TShared<render::GpuBuffer> readback = test.CreateReadback(pitch.RowPitch * pitch.SliceHeight * sizeof(u32));

		{
			FrameGraph graph(*test.Context, kGraphName);
			const TShared<render::Texture> target = graph.CreateTexture("Cleared target", targetInformation);
			B3D_TEST_ASSERT(target != nullptr)
			if(target == nullptr)
				return;

			// The second clear hides the first one from the read
			for(const char* name : { "First clear", "Second clear" })
			{
				FrameGraphRasterPass pass;
				pass.SetColorAttachment(0, target, FrameGraphAttachmentLoad::Clear);
				graph.AddRasterPass(name, std::move(pass), nullptr);
			}

			graph.ImportBuffer(readback);

			FrameGraphCopyPass copyPass;
			copyPass.AddSource(target);
			copyPass.AddDestination(readback);
			graph.AddCopyPass("Read", std::move(copyPass), [target, readback](FrameGraphPassContext& passContext)
			{
				passContext.GetCommandBuffer().CopyTextureToBuffer(target, readback, 0, 0);
			});

			// Nothing reads the transients of this pass, so they are never placed
			test.AddDispatch(graph, "Unobserved", graph.CreateBuffer("Unobserved buffer", GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32), kSize * kSize,
				GpuBufferFlag::StoreOnGPU | GpuBufferFlag::AllowUnorderedAccessOnTheGPU)), graph.CreateTexture("Unobserved texture", test.TextureInformation));

			graph.Compile();
			B3D_TEST_ASSERT(graph.IsPassCulled(0))
			B3D_TEST_ASSERT(!graph.IsPassCulled(1))
			B3D_TEST_ASSERT(!graph.IsPassCulled(2))
			B3D_TEST_ASSERT(graph.IsPassCulled(3))
			B3D_TEST_ASSERT(graph.GetStatistics().CulledPassCount == 2)
			B3D_TEST_ASSERT(graph.GetStatistics().AliasAcquireCount == 1u)

			graph.Execute();
		}

		device->WaitUntilIdle();

		const render::GpuBufferMappedScope mapping = readback->Map(GpuMapOption::Read);
		B3D_TEST_ASSERT(mapping.IsValid())
		if(!mapping.IsValid())
			return;

		bool isCleared = true;
		const u32* pixels = static_cast<const u32*>(mapping.GetMappedMemory());
		for(u32 row = 0; row < kSize; row++)
		{
			for(u32 column = 0; column < kSize; column++)
				isCleared &= pixels[row * pitch.RowPitch + column] == 0xFF0000FF;
		}

		B3D_TEST_ASSERT_MSG(isCleared, "The read does not see the clear of the surviving raster pass.")

		device->EndFrame();
	});
}

void FrameGraphTestSuite::TestSplitAliasAcquire()
{
	const TShared<GpuDevice> device = GetTestDevice();
	const TShared<GpuComputePipelineState> pipeline = device != nullptr ? CreateTestDispatchPipeline(*device) : nullptr;
	if(pipeline == nullptr)
		return;

	LoggingScope logs(*this);
	logs.IgnoreWarning(kBarrierOnlyWarning);

	RunOnRenderThread("FrameGraphTestSuite::TestSplitAliasAcquire", [this, &device, &pipeline]()
	{
		const FrameGraphTestContext test(*device, pipeline);
		const GpuBufferCreateInformation bufferInformation = GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32), kSize * kSize,
			GpuBufferFlag::StoreOnGPU | GpuBufferFlag::AllowUnorderedAccessOnTheGPU);

		TShared<render::GpuBuffer> readbacks[4];
		for(u32 readbackIndex = 0; readbackIndex < 4; readbackIndex++)
			readbacks[readbackIndex] = test.CreateReadback(readbackIndex % 2 == 0 ? kSize * kSize * (u32)sizeof(u32) : test.TextureReadbackSize);

		const TShared<render::GpuBuffer> unrelatedBuffer = test.CreateBuffer("Unrelated buffer");
		const TShared<render::Texture> unrelatedTexture = test.CreateTexture("Unrelated texture");

		{
			FrameGraph graph(*test.Context, kGraphName);

			// The second pair of transients can take the memory the first pair releases after its read, with a pass in between
			const TShared<render::GpuBuffer> firstBuffer = graph.CreateBuffer("First buffer", bufferInformation);
			const TShared<render::Texture> firstTexture = graph.CreateTexture("First texture", test.TextureInformation);
			test.AddDispatch(graph, "First write", firstBuffer, firstTexture);
			test.AddReadback(graph, "First read", firstBuffer, firstTexture, readbacks[0], readbacks[1]);

			graph.ImportBuffer(unrelatedBuffer);
			graph.ImportTexture(unrelatedTexture);
			test.AddDispatch(graph, "Unrelated", unrelatedBuffer, unrelatedTexture);

			const TShared<render::GpuBuffer> secondBuffer = graph.CreateBuffer("Second buffer", bufferInformation);
			const TShared<render::Texture> secondTexture = graph.CreateTexture("Second texture", test.TextureInformation);
			test.AddDispatch(graph, "Second write", secondBuffer, secondTexture);
			test.AddReadback(graph, "Second read", secondBuffer, secondTexture, readbacks[2], readbacks[3]);

			graph.Compile();
			B3D_TEST_ASSERT(graph.GetStatistics().AliasAcquireCount == 4u)
			graph.Execute();

			// The second pair's alias acquires are released after the first read where it shares memory with the first pair, and at the
			// start of the graph otherwise
			B3D_TEST_ASSERT(graph.GetStatistics().SplitAliasAcquireCount == 2u)
		}

		device->WaitUntilIdle();
		B3D_TEST_ASSERT_MSG(test.HasDispatchValues(readbacks[0], false), "The first transient buffer does not hold its values.")
		B3D_TEST_ASSERT_MSG(test.HasDispatchValues(readbacks[1], true), "The first transient texture does not hold its values.")
		B3D_TEST_ASSERT_MSG(test.HasDispatchValues(readbacks[2], false), "The second transient buffer does not hold its values.")
		B3D_TEST_ASSERT_MSG(test.HasDispatchValues(readbacks[3], true), "The second transient texture does not hold its values.")

		device->EndFrame();
	});
}

void FrameGraphTestSuite::TestCrossGraphOrdering()
{
	const TShared<GpuDevice> device = GetTestDevice();
	const TShared<GpuComputePipelineState> pipeline = device != nullptr ? CreateTestDispatchPipeline(*device) : nullptr;
	if(pipeline == nullptr)
		return;

	LoggingScope logs(*this);
	logs.IgnoreWarning(kBarrierOnlyWarning);

	RunOnRenderThread("FrameGraphTestSuite::TestCrossGraphOrdering", [this, &device, &pipeline]()
	{
		const FrameGraphTestContext test(*device, pipeline);
		const GpuBufferCreateInformation bufferInformation = GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32), kSize * kSize,
			GpuBufferFlag::StoreOnGPU | GpuBufferFlag::AllowUnorderedAccessOnTheGPU);

		TShared<render::GpuBuffer> readbacks[4];
		for(u32 readbackIndex = 0; readbackIndex < 4; readbackIndex++)
			readbacks[readbackIndex] = test.CreateReadback(readbackIndex % 2 == 0 ? kSize * kSize * (u32)sizeof(u32) : test.TextureReadbackSize);

		const TShared<render::GpuBuffer> unrelatedBuffer = test.CreateBuffer("Unrelated buffer");
		const TShared<render::Texture> unrelatedTexture = test.CreateTexture("Unrelated texture");

		// The first graph's last pass reads its transients. Their first use is the graph's first pass, so their acquires wait for earlier work
		// right there.
		{
			FrameGraph graph(*test.Context, kGraphName);
			const TShared<render::GpuBuffer> buffer = graph.CreateBuffer("First buffer", bufferInformation);
			const TShared<render::Texture> texture = graph.CreateTexture("First texture", test.TextureInformation);
			test.AddDispatch(graph, "First write", buffer, texture);
			test.AddReadback(graph, "First read", buffer, texture, readbacks[0], readbacks[1]);

			graph.Compile();
			B3D_TEST_ASSERT(graph.GetStatistics().AliasAcquireCount == 2u)
			B3D_TEST_ASSERT(graph.GetStatistics().SplitAliasAcquireCount == 0)
			graph.Execute();
		}

		// The second graph, submitted right after, gets the cached transients back on the same memory, and writes them after an unrelated pass.
		// Nothing orders the submissions, so the acquires wait for all earlier work, released at the start of the graph.
		{
			FrameGraph graph(*test.Context, kGraphName);
			graph.ImportBuffer(unrelatedBuffer);
			graph.ImportTexture(unrelatedTexture);
			test.AddDispatch(graph, "Unrelated", unrelatedBuffer, unrelatedTexture);

			const TShared<render::GpuBuffer> buffer = graph.CreateBuffer("Second buffer", bufferInformation);
			const TShared<render::Texture> texture = graph.CreateTexture("Second texture", test.TextureInformation);
			test.AddDispatch(graph, "Second write", buffer, texture);
			test.AddReadback(graph, "Second read", buffer, texture, readbacks[2], readbacks[3]);

			graph.Compile();
			B3D_TEST_ASSERT(graph.GetStatistics().AliasAcquireCount == 2u)
			B3D_TEST_ASSERT(graph.GetStatistics().SplitAliasAcquireCount == 2u)
			graph.Execute();
		}

		device->WaitUntilIdle();
		B3D_TEST_ASSERT_MSG(test.HasDispatchValues(readbacks[0], false), "The first graph's transient buffer does not hold its values.")
		B3D_TEST_ASSERT_MSG(test.HasDispatchValues(readbacks[1], true), "The first graph's transient texture does not hold its values.")
		B3D_TEST_ASSERT_MSG(test.HasDispatchValues(readbacks[2], false), "The second graph's transient buffer does not hold its values.")
		B3D_TEST_ASSERT_MSG(test.HasDispatchValues(readbacks[3], true), "The second graph's transient texture does not hold its values.")

		device->EndFrame();
	});
}

void FrameGraphTestSuite::TestExtract()
{
	const TShared<GpuDevice> device = GetTestDevice();
	const TShared<GpuComputePipelineState> pipeline = device != nullptr ? CreateTestDispatchPipeline(*device) : nullptr;
	if(pipeline == nullptr)
		return;

	LoggingScope logs(*this);
	logs.IgnoreWarning(kBarrierOnlyWarning);

	RunOnRenderThread("FrameGraphTestSuite::TestExtract", [this, &device, &pipeline]()
	{
		const FrameGraphTestContext test(*device, pipeline);
		const GpuBufferCreateInformation bufferInformation = GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32), kSize * kSize,
			GpuBufferFlag::StoreOnGPU | GpuBufferFlag::AllowUnorderedAccessOnTheGPU);

		// An exported transient returns to rest after its pass, and the end of the graph finds it there
		for(const bool isExported : { false, true })
		{
			TShared<render::GpuBuffer> extractedBuffer;
			TShared<render::Texture> extractedTexture;
			{
				FrameGraph graph(*test.Context, kGraphName);
				extractedBuffer = graph.CreateBuffer("Extracted buffer", bufferInformation);
				extractedTexture = graph.CreateTexture("Extracted texture", test.TextureInformation);
				test.AddDispatch(graph, "Write", extractedBuffer, extractedTexture, isExported);
				graph.ExtractBuffer(extractedBuffer);
				graph.ExtractTexture(extractedTexture);

				graph.Compile();
				B3D_TEST_ASSERT(!graph.IsPassCulled(0))
				B3D_TEST_ASSERT(graph.GetStatistics().AliasAcquireCount == 0)
				graph.Execute();
			}

			// Extracted resources live in persistent memory, which no transient shares
			B3D_TEST_ASSERT(!extractedBuffer->HasFixedLocation() && !extractedTexture->HasFixedLocation())
			B3D_TEST_ASSERT(extractedBuffer->GetFrameGraphResourceIndex() == ~0u && extractedTexture->GetFrameGraphResourceIndex() == ~0u)

			device->EndFrame();

			const TShared<render::GpuBuffer> bufferReadback = test.CreateReadback(kSize * kSize * sizeof(u32));
			const TShared<render::GpuBuffer> textureReadback = test.CreateReadback(test.TextureReadbackSize);
			{
				FrameGraph graph(*test.Context, kGraphName);
				graph.ImportBuffer(extractedBuffer);
				graph.ImportTexture(extractedTexture);
				test.AddReadback(graph, "Read", extractedBuffer, extractedTexture, bufferReadback, textureReadback);

				graph.Compile();
				graph.Execute();
			}

			device->WaitUntilIdle();
			B3D_TEST_ASSERT_MSG(test.HasDispatchValues(bufferReadback, false), isExported ? "The extracted and exported buffer does not hold the values written before its extraction."
				: "The extracted buffer does not hold the values written before its extraction.")
			B3D_TEST_ASSERT_MSG(test.HasDispatchValues(textureReadback, true), isExported ? "The extracted and exported texture does not hold the values written before its extraction."
				: "The extracted texture does not hold the values written before its extraction.")

			device->EndFrame();
		}
	});
}

void FrameGraphTestSuite::TestExport()
{
	const TShared<GpuDevice> device = GetTestDevice();
	const TShared<GpuComputePipelineState> pipeline = device != nullptr ? CreateTestDispatchPipeline(*device) : nullptr;
	if(pipeline == nullptr)
		return;

	LoggingScope logs(*this);
	logs.IgnoreWarning(kBarrierOnlyWarning);

	RunOnRenderThread("FrameGraphTestSuite::TestExport", [this, &device, &pipeline]()
	{
		const FrameGraphTestContext test(*device, pipeline);
		const TShared<render::GpuBuffer> buffer = test.CreateBuffer("Buffer at graph end");
		const TShared<render::Texture> texture = test.CreateTexture("Exported texture");

		{
			// The texture returns to rest right after its pass, the buffer at the end of the graph
			FrameGraph graph(*test.Context, kGraphName);
			graph.ImportBuffer(buffer);
			graph.ImportTexture(texture);

			FrameGraphComputePass pass;
			const TShared<render::GpuParameterSet> parameters = test.Context->GetParameterSetPool().Create(pipeline->GetParameterLayout()->GetSet(0), 0);
			pass.AddParameterSet(parameters);
			parameters->SetStorageBuffer("OutputBuffer", buffer);
			parameters->SetStorageTexture("OutputTexture", texture, TextureSurface());
			pass.Export(texture);

			graph.AddComputePass("Write", std::move(pass), [&pipeline, parameters](FrameGraphPassContext& passContext)
			{
				render::GpuCommandBuffer& commands = passContext.GetCommandBuffer();
				commands.SetGpuComputePipelineState(pipeline);
				commands.SetGpuParameterSet(parameters);
				commands.DispatchCompute(1, 1, 1);
			});

			const TShared<render::GpuBuffer> unrelatedBuffer = test.CreateBuffer("Unrelated buffer");
			const TShared<render::Texture> unrelatedTexture = test.CreateTexture("Unrelated texture");
			graph.ImportBuffer(unrelatedBuffer);
			graph.ImportTexture(unrelatedTexture);
			test.AddDispatch(graph, "Unrelated", unrelatedBuffer, unrelatedTexture);
			graph.Compile();
			graph.Execute();
		}

		device->EndFrame();

		// A barrier from rest of a resource in another state is reported by the backends' validation
		const TShared<render::GpuBuffer> bufferReadback = test.CreateReadback(kSize * kSize * sizeof(u32));
		const TShared<render::GpuBuffer> textureReadback = test.CreateReadback(test.TextureReadbackSize);
		{
			FrameGraph graph(*test.Context, kGraphName);
			graph.ImportBuffer(buffer);
			graph.ImportTexture(texture);
			test.AddReadback(graph, "Read", buffer, texture, bufferReadback, textureReadback);

			graph.Compile();
			graph.Execute();
		}

		device->WaitUntilIdle();
		B3D_TEST_ASSERT_MSG(test.HasDispatchValues(bufferReadback, false), "The buffer returned to rest at the end of the graph does not hold its values.")
		B3D_TEST_ASSERT_MSG(test.HasDispatchValues(textureReadback, true), "The exported texture does not hold its values.")

		device->EndFrame();
	});
}

void FrameGraphTestSuite::TestRestingReads()
{
	const TShared<GpuDevice> device = GetTestDevice();
	const TShared<GpuComputePipelineState> pipeline = device != nullptr ? CreateTestDispatchPipeline(*device) : nullptr;
	const TShared<GpuComputePipelineState> samplePipeline = device != nullptr ? CreateTestSamplePipeline(*device) : nullptr;
	if(pipeline == nullptr || samplePipeline == nullptr)
		return;

	LoggingScope logs(*this);
	logs.IgnoreWarning(kBarrierOnlyWarning);

	// Undeclared accesses are checked against the resting state
	const bool validateRestingStates = gGpuValidateRestingStates;
	gGpuValidateRestingStates.Set(true);

	RunOnRenderThread("FrameGraphTestSuite::TestRestingReads", [this, &device, &pipeline, &samplePipeline]()
	{
		const FrameGraphTestContext test(*device, pipeline, samplePipeline);

		// The upload takes the texture from rest and returns it there
		const TShared<render::Texture> uploaded = test.CreateTexture("Uploaded texture", TextureUsageFlag::Default);
		const TShared<PixelData> pixels = PixelData::Create(kSize, kSize, 1, PF_R32U);
		for(u32 row = 0; row < kSize; row++)
		{
			u32* const rowElements = reinterpret_cast<u32*>(pixels->GetData() + row * pixels->GetRowPitch());
			for(u32 column = 0; column < kSize; column++)
				rowElements[column] = kDispatchPattern | (row * kSize + column);
		}

		const TShared<render::GpuCommandBuffer> uploadCommands = test.Pool->Create(GpuCommandBufferCreateInformation::Create("Upload"));
		TextureUtility::Write(*test.Context, uploaded, *pixels, 0, 0, TextureWriteFlag::Normal, uploadCommands);
		test.Context->SubmitCommandBuffer(uploadCommands, GpuQueueMask::kNone);

		// A graph writes a render target texture, and returns it to rest at its end
		const TShared<render::Texture> rendered = test.CreateTexture("Rendered texture", TextureUsageFlag::RenderTarget | TextureUsageFlag::AllowUnorderedAccessOnTheGPU);
		const TShared<render::GpuBuffer> renderedBuffer = test.CreateBuffer("Rendered buffer");
		{
			FrameGraph graph(*test.Context, kGraphName);
			graph.ImportBuffer(renderedBuffer);
			graph.ImportTexture(rendered);
			test.AddDispatch(graph, "Render", renderedBuffer, rendered);

			graph.Compile();
			graph.Execute();
		}

		device->EndFrame();

		// A later frame samples both without importing them
		const TShared<render::GpuBuffer> uploadedValues = test.CreateBuffer("Uploaded values");
		const TShared<render::GpuBuffer> renderedValues = test.CreateBuffer("Rendered values");
		const TShared<render::GpuBuffer> uploadedReadback = test.CreateReadback(kSize * kSize * sizeof(u32));
		const TShared<render::GpuBuffer> renderedReadback = test.CreateReadback(kSize * kSize * sizeof(u32));
		{
			FrameGraph graph(*test.Context, kGraphName);
			graph.ImportBuffer(uploadedValues);
			graph.ImportBuffer(renderedValues);
			test.AddSample(graph, "Sample upload", uploaded, uploadedValues);
			test.AddSample(graph, "Sample render", rendered, renderedValues);
			test.AddReadback(graph, "Read upload", uploadedValues, uploadedReadback);
			test.AddReadback(graph, "Read render", renderedValues, renderedReadback);

			graph.Compile();
			graph.Execute();
		}

		device->WaitUntilIdle();
		B3D_TEST_ASSERT_MSG(test.HasDispatchValues(uploadedReadback, false), "Sampling at rest does not read the uploaded texels.")
		B3D_TEST_ASSERT_MSG(test.HasDispatchValues(renderedReadback, false), "Sampling at rest does not read what the earlier graph wrote.")

		device->EndFrame();
	});

	gGpuValidateRestingStates.Set(validateRestingStates);
}

void FrameGraphTestSuite::TestLateMemory()
{
	const TShared<GpuDevice> device = GetTestDevice();
	const TShared<GpuComputePipelineState> pipeline = device != nullptr ? CreateTestDispatchPipeline(*device) : nullptr;
	if(pipeline == nullptr)
		return;

	LoggingScope logs(*this);
	logs.IgnoreWarning(kBarrierOnlyWarning);

	RunOnRenderThread("FrameGraphTestSuite::TestLateMemory", [this, &device, &pipeline]()
	{
		const FrameGraphTestContext test(*device, pipeline);
		const GpuBufferCreateInformation bufferInformation = GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32), kSize * kSize,
			GpuBufferFlag::StoreOnGPU | GpuBufferFlag::AllowUnorderedAccessOnTheGPU);

		// The first graph places the texture alone. The second places a transient buffer before it, so the cached texture moves to other
		// memory after the dispatch's parameter set took it.
		for(const bool isMoved : { false, true })
		{
			const TShared<render::GpuBuffer> bufferReadback = test.CreateReadback(kSize * kSize * sizeof(u32));
			const TShared<render::GpuBuffer> textureReadback = test.CreateReadback(test.TextureReadbackSize);
			{
				FrameGraph graph(*test.Context, kGraphName);

				TShared<render::GpuBuffer> buffer;
				if(isMoved)
					buffer = graph.CreateBuffer("Late buffer", bufferInformation);
				else
				{
					buffer = test.CreateBuffer("Late buffer");
					graph.ImportBuffer(buffer);
				}

				const TShared<render::Texture> texture = graph.CreateTexture("Late texture", test.TextureInformation);
				test.AddDispatch(graph, "Write", buffer, texture);
				test.AddReadback(graph, "Read", buffer, texture, bufferReadback, textureReadback);

				graph.Compile();
				graph.Execute();
			}

			if(isMoved)
				B3D_TEST_ASSERT(test.Context->GetTransientResourceAllocator().GetStatistics().Relocations >= 1)

			device->WaitUntilIdle();
			B3D_TEST_ASSERT_MSG(test.HasDispatchValues(textureReadback, true), isMoved ? "The moved transient texture does not hold the dispatch's values."
				: "The transient texture does not hold the dispatch's values.")

			device->EndFrame();
		}
	});
}

void FrameGraphTestSuite::TestReadback()
{
	const TShared<GpuDevice> device = GetTestDevice();
	const TShared<GpuComputePipelineState> pipeline = device != nullptr ? CreateTestDispatchPipeline(*device) : nullptr;
	if(pipeline == nullptr)
		return;

	LoggingScope logs(*this);
	logs.IgnoreWarning(kBarrierOnlyWarning);

	RunOnRenderThread("FrameGraphTestSuite::TestReadback", [this, &device, &pipeline]()
	{
		const FrameGraphTestContext test(*device, pipeline);
		const GpuBufferCreateInformation bufferInformation = GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32), kSize * kSize,
			GpuBufferFlag::StoreOnGPU | GpuBufferFlag::AllowUnorderedAccessOnTheGPU);

		TAsyncOp<TShared<MemoryDataStream>> bufferRead;
		TAsyncOp<TShared<PixelData>> textureRead;
		{
			FrameGraph graph(*test.Context, kGraphName);
			const TShared<render::GpuBuffer> buffer = graph.CreateBuffer("Read buffer", bufferInformation);
			const TShared<render::Texture> texture = graph.CreateTexture("Read texture", test.TextureInformation);
			test.AddDispatch(graph, "Write", buffer, texture);

			bufferRead = render::GpuBufferUtility::ReadAsync(graph, buffer, 0, kSize * kSize * sizeof(u32));
			textureRead = render::TextureUtility::ReadAsync(graph, texture);

			// The staging buffers are imported, so the copy passes and the dispatch survive
			graph.Compile();
			B3D_TEST_ASSERT(graph.GetStatistics().CulledPassCount == 0)
			graph.Execute();
		}

		// The command buffer's completion callbacks run on this thread while it waits
		device->WaitUntilIdle();
		B3D_TEST_ASSERT_MSG(bufferRead.HasCompleted() && textureRead.HasCompleted(), "The reads did not complete with the graph's command buffer.")
		if(!bufferRead.HasCompleted() || !textureRead.HasCompleted())
			return;

		const TShared<MemoryDataStream> bufferData = bufferRead.GetReturnValue();
		const TShared<PixelData> textureData = textureRead.GetReturnValue();
		B3D_TEST_ASSERT(bufferData != nullptr && textureData != nullptr)
		if(bufferData == nullptr || textureData == nullptr)
			return;

		B3D_TEST_ASSERT_MSG(test.HasDispatchValues(reinterpret_cast<const u32*>(bufferData->Data()), kSize), "The buffer read does not hold the dispatch's values.")
		B3D_TEST_ASSERT_MSG(test.HasDispatchValues(reinterpret_cast<const u32*>(textureData->GetData()), textureData->GetRowPitch() / sizeof(u32)),
			"The texture read does not hold the dispatch's values.")

		device->EndFrame();
	});
}

void FrameGraphTestSuite::TestProfiling()
{
	const TShared<GpuDevice> device = GetTestDevice();
	const TShared<GpuComputePipelineState> pipeline = device != nullptr ? CreateTestDispatchPipeline(*device) : nullptr;
	if(pipeline == nullptr || !device->GetCapabilities().HasCapability(RSC_TIMER_QUERIES))
		return;

	LoggingScope logs(*this);
	logs.IgnoreWarning(kBarrierOnlyWarning);

	RunOnRenderThread("FrameGraphTestSuite::TestProfiling", [this, &device, &pipeline]()
	{
		constexpr const char* kProfiledGraphName = "Profiled frame graph";

		const FrameGraphTestContext test(*device, pipeline);
		const TShared<render::GpuBuffer> buffer = test.CreateBuffer("Written buffer");
		const TShared<render::Texture> texture = test.CreateTexture("Written texture");
		const TShared<render::GpuBuffer> readback = test.CreateReadback(kSize * kSize * sizeof(u32));

		{
			FrameGraph graph(*test.Context, kProfiledGraphName);
			graph.ImportBuffer(buffer);
			graph.ImportTexture(texture);
			test.AddDispatch(graph, "Write", buffer, texture);

			// Nothing reads the transients, so the pass is culled and records no sample
			const TShared<render::GpuBuffer> unreadBuffer = graph.CreateBuffer("Unread buffer", GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32),
				kSize * kSize, GpuBufferFlag::StoreOnGPU | GpuBufferFlag::AllowUnorderedAccessOnTheGPU));
			const TShared<render::Texture> unreadTexture = graph.CreateTexture("Unread texture", test.TextureInformation);
			test.AddDispatch(graph, "Culled", unreadBuffer, unreadTexture);

			graph.ImportBuffer(readback);

			FrameGraphCopyPass pass;
			pass.AddSource(buffer);
			pass.AddDestination(readback);

			graph.AddCopyPass("Read", std::move(pass), [this, buffer, readback](FrameGraphPassContext& passContext)
			{
				render::GpuCommandBuffer& commands = passContext.GetCommandBuffer();
				const TShared<GpuCommandBufferProfiler>& profiler = passContext.GetProfiler();
				B3D_TEST_ASSERT(profiler != nullptr)

				if(profiler != nullptr)
					profiler->BeginSample(commands, "Copy");

				commands.CopyBufferToBuffer(buffer, readback, 0, 0, kSize * kSize * sizeof(u32));

				if(profiler != nullptr)
					profiler->EndSample(commands);
			});

			graph.Compile();
			B3D_TEST_ASSERT(graph.GetStatistics().CulledPassCount == 1)
			graph.Execute();
		}

		device->WaitUntilIdle();
		GetGpuProfiler().Update();

		// The graph's sample holds a sample for each surviving pass, in execution order, and the pass's own samples nest in its sample
		const TOptional<GpuProfilerResults> results = GetGpuProfiler().GetResults(kProfiledGraphName);
		B3D_TEST_ASSERT_MSG(results.has_value(), "The GPU profiler did not resolve the graph's samples.")
		if(!results.has_value())
			return;

		B3D_TEST_ASSERT(results->Samples.Size() == 1)
		if(results->Samples.Size() != 1)
			return;

		const GpuProfilerSample& graphSample = results->Samples[0];
		B3D_TEST_ASSERT(graphSample.Name == kProfiledGraphName)
		B3D_TEST_ASSERT(graphSample.ChildSamples.Size() == 2)
		if(graphSample.ChildSamples.Size() != 2)
			return;

		B3D_TEST_ASSERT(graphSample.ChildSamples[0].Name == "Write" && graphSample.ChildSamples[0].ChildSamples.Empty())
		B3D_TEST_ASSERT(graphSample.ChildSamples[1].Name == "Read")
		B3D_TEST_ASSERT(graphSample.ChildSamples[1].ChildSamples.Size() == 1 && graphSample.ChildSamples[1].ChildSamples[0].Name == "Copy")

		device->EndFrame();
	});
}

void FrameGraphTestSuite::TestRasterPass()
{
	const TShared<GpuDevice> device = GetTestDevice();
	if(device == nullptr)
		return;

	LoggingScope logs(*this);
	logs.IgnoreWarning(kBarrierOnlyWarning);

	RunOnRenderThread("FrameGraphTestSuite::TestRasterPass", [this, &device]()
	{
		const TShared<GpuWorkContext> context = GpuWorkContext::Create(*device);
		const ShaderVariationParameters variation(TInlineArray<ShaderVariationParameter, 4>({
			ShaderVariationParameter("MSAA_COUNT", 1),
			ShaderVariationParameter("MODE", 0),
			ShaderVariationParameter("BLEND", 0),
			ShaderVariationParameter("WRITE_ALPHA", 0),
			ShaderVariationParameter("SRGB_ENCODE", 0),
			ShaderVariationParameter("OUTPUT_32BIT", false),
		}));

		BlitMat* const blitMaterial = BlitMat::Get(*context, variation);
		B3D_TEST_ASSERT(blitMaterial != nullptr)
		if(blitMaterial == nullptr)
			return;

		// Every texel of the source differs, and alpha is opaque
		const TShared<PixelData> pixels = PixelData::Create(kSize, kSize, 1, PF_RGBA8);
		const auto fnExpectedTexel = [](u32 row, u32 column) { return 0xFF000000 | (row << 16) | (column << 8) | (row * kSize + column); };
		for(u32 row = 0; row < kSize; row++)
		{
			u32* const rowTexels = reinterpret_cast<u32*>(pixels->GetData() + row * pixels->GetRowPitch());
			for(u32 column = 0; column < kSize; column++)
				rowTexels[column] = fnExpectedTexel(row, column);
		}

		TextureCreateInformation information;
		information.Name = "Blit source";
		information.Width = kSize;
		information.Height = kSize;
		information.Format = PF_RGBA8;
		information.Usage = TextureUsageFlag::Default;

		// The upload takes the source from rest and returns it there
		const TShared<render::Texture> source = device->CreateTexture(information);

		const TShared<GpuCommandBufferPool> pool = GpuBarrierTestUtility::CreateCommandBufferPool(*device);

		const TShared<render::GpuCommandBuffer> uploadCommands = pool->Create(GpuCommandBufferCreateInformation::Create("Upload"));
		TextureUtility::Write(*context, source, *pixels, 0, 0, TextureWriteFlag::Normal, uploadCommands);
		context->SubmitCommandBuffer(uploadCommands, GpuQueueMask::kNone);

		information.Usage = TextureUsageFlag::RenderTarget;
		const Area2 sourceArea(0.0f, 0.0f, (float)kSize, (float)kSize);

		TAsyncOp<TShared<PixelData>> passSetRead;
		TAsyncOp<TShared<PixelData>> undeclaredSetRead;
		{
			FrameGraph graph(*context, kGraphName);

			// The first pass binds a set it declares
			const TShared<render::Texture> passSetTarget = graph.CreateTexture("Pass set target", information);

			FrameGraphRasterPass passSetPass;
			passSetPass.SetColorAttachment(0, passSetTarget, FrameGraphAttachmentLoad::DontCare);
			const TShared<render::GpuParameterSet> passSet = blitMaterial->Prepare(passSetPass, source);

			graph.AddRasterPass("Blit with a pass set", std::move(passSetPass), [blitMaterial, passSet, sourceArea](FrameGraphPassContext& passContext)
			{
				passContext.GetCommandBuffer().SetViewport(Area2(0.0f, 0.0f, 1.0f, 1.0f));
				blitMaterial->Execute(passContext, passSet, sourceArea, false);
			});

			// The second binds a set the graph does not declare, which holds the source at rest
			const TShared<render::Texture> undeclaredSetTarget = graph.CreateTexture("Undeclared set target", information);

			FrameGraphRasterPass undeclaredSetPass;
			undeclaredSetPass.SetColorAttachment(0, undeclaredSetTarget, FrameGraphAttachmentLoad::DontCare);
			const TShared<render::GpuParameterSet> undeclaredSet = blitMaterial->Prepare(source);

			graph.AddRasterPass("Blit with an undeclared set", std::move(undeclaredSetPass), [blitMaterial, undeclaredSet, sourceArea](FrameGraphPassContext& passContext)
			{
				passContext.GetCommandBuffer().SetViewport(Area2(0.0f, 0.0f, 1.0f, 1.0f));
				blitMaterial->Execute(passContext, undeclaredSet, sourceArea, false);
			});

			passSetRead = TextureUtility::ReadAsync(graph, passSetTarget);
			undeclaredSetRead = TextureUtility::ReadAsync(graph, undeclaredSetTarget);

			graph.Compile();
			B3D_TEST_ASSERT(graph.GetStatistics().CulledPassCount == 0)
			graph.Execute();
		}

		// The command buffer's completion callbacks run on this thread while it waits
		device->WaitUntilIdle();
		B3D_TEST_ASSERT_MSG(passSetRead.HasCompleted() && undeclaredSetRead.HasCompleted(), "The reads did not complete with the graph's command buffer.")
		if(!passSetRead.HasCompleted() || !undeclaredSetRead.HasCompleted())
			return;

		const auto fnHasSourceTexels = [&fnExpectedTexel](const TShared<PixelData>& data)
		{
			if(data == nullptr)
				return false;

			for(u32 row = 0; row < kSize; row++)
			{
				const u32* const rowTexels = reinterpret_cast<const u32*>(data->GetData() + row * data->GetRowPitch());
				for(u32 column = 0; column < kSize; column++)
				{
					if(rowTexels[column] != fnExpectedTexel(row, column))
						return false;
				}
			}

			return true;
		};

		B3D_TEST_ASSERT_MSG(fnHasSourceTexels(passSetRead.GetReturnValue()), "The blit through a pass set does not hold the source's texels.")
		B3D_TEST_ASSERT_MSG(fnHasSourceTexels(undeclaredSetRead.GetReturnValue()), "The blit through an undeclared set does not hold the source's texels.")

		device->EndFrame();
	});
}

void FrameGraphTestSuite::TestDepthReadOnly()
{
	const TShared<GpuDevice> device = GetTestDevice();
	if(device == nullptr)
		return;

	LoggingScope logs(*this);
	logs.IgnoreWarning(kBarrierOnlyWarning);

	RunOnRenderThread("FrameGraphTestSuite::TestDepthReadOnly", [this, &device]()
	{
		const TShared<GpuWorkContext> context = GpuWorkContext::Create(*device);
		const ShaderVariationParameters variation(TInlineArray<ShaderVariationParameter, 4>({
			ShaderVariationParameter("MSAA_COUNT", 1),
			ShaderVariationParameter("MODE", 0),
			ShaderVariationParameter("BLEND", 0),
			ShaderVariationParameter("WRITE_ALPHA", 0),
			ShaderVariationParameter("SRGB_ENCODE", 0),
			ShaderVariationParameter("OUTPUT_32BIT", false),
		}));

		BlitMat* const blitMaterial = BlitMat::Get(*context, variation);
		B3D_TEST_ASSERT(blitMaterial != nullptr)
		if(blitMaterial == nullptr)
			return;

		TAsyncOp<TShared<PixelData>> colorRead;
		{
			FrameGraph graph(*context, kGraphName);

			TextureCreateInformation depthInformation;
			depthInformation.Width = kSize;
			depthInformation.Height = kSize;
			depthInformation.Format = PF_D32_S8X24;
			depthInformation.Usage = TextureUsageFlag::DepthStencil;
			const TShared<render::Texture> depth = graph.CreateTexture("Depth", depthInformation);

			FrameGraphRasterPass clearPass;
			clearPass.SetDepthStencilAttachment(depth, FrameGraphDepthStencilMode::Write, FrameGraphAttachmentLoad::Clear);
			graph.AddRasterPass("Clear depth", std::move(clearPass), nullptr);

			TextureCreateInformation colorInformation;
			colorInformation.Width = kSize;
			colorInformation.Height = kSize;
			colorInformation.Format = PF_RGBA8;
			colorInformation.Usage = TextureUsageFlag::RenderTarget;
			const TShared<render::Texture> color = graph.CreateTexture("Color", colorInformation);

			// The depth the pass samples is the depth its depth tests read, in one layout with the stencil the pass may write
			FrameGraphRasterPass samplePass;
			samplePass.SetColorAttachment(0, color, FrameGraphAttachmentLoad::Clear);
			samplePass.SetDepthStencilAttachment(depth, FrameGraphDepthStencilMode::DepthReadOnly, FrameGraphAttachmentLoad::Load);
			blitMaterial->Prepare(samplePass, depth);
			graph.AddRasterPass("Sample depth", std::move(samplePass), nullptr);

			colorRead = TextureUtility::ReadAsync(graph, color);

			graph.Compile();
			B3D_TEST_ASSERT(graph.GetStatistics().CulledPassCount == 0)
			graph.Execute();
		}

		// The command buffer's completion callbacks run on this thread while it waits
		device->WaitUntilIdle();
		B3D_TEST_ASSERT_MSG(colorRead.HasCompleted(), "The read did not complete with the graph's command buffer.")
		if(!colorRead.HasCompleted())
			return;

		const TShared<PixelData> colorData = colorRead.GetReturnValue();
		bool isCleared = colorData != nullptr;
		for(u32 row = 0; row < kSize && isCleared; row++)
		{
			const u32* const rowTexels = reinterpret_cast<const u32*>(colorData->GetData() + row * colorData->GetRowPitch());
			for(u32 column = 0; column < kSize; column++)
				isCleared &= rowTexels[column] == 0;
		}

		B3D_TEST_ASSERT_MSG(isCleared, "The color attachment does not hold its clear value.")

		device->EndFrame();
	});
}

void FrameGraphTestSuite::TestTwoAspectAliasAcquire()
{
	const TShared<GpuDevice> device = GetTestDevice();
	if(device == nullptr)
		return;

	LoggingScope logs(*this);
	logs.IgnoreWarning(kBarrierOnlyWarning);

	RunOnRenderThread("FrameGraphTestSuite::TestTwoAspectAliasAcquire", [this, &device]()
	{
		const TShared<GpuWorkContext> context = GpuWorkContext::Create(*device);

		TextureCreateInformation colorInformation;
		colorInformation.Width = kSize;
		colorInformation.Height = kSize;
		colorInformation.Format = PF_RGBA8;
		colorInformation.Usage = TextureUsageFlag::RenderTarget;

		TextureCreateInformation depthInformation = colorInformation;
		depthInformation.Format = PF_D32_S8X24;
		depthInformation.Usage = TextureUsageFlag::DepthStencil;

		TAsyncOp<TShared<PixelData>> reads[2];
		{
			FrameGraph graph(*context, kGraphName);

			// The first color texture lives until the end of the graph, so no memory is released within it
			const TShared<render::Texture> firstColor = graph.CreateTexture("First color", colorInformation);
			FrameGraphRasterPass firstPass;
			firstPass.SetColorAttachment(0, firstColor, FrameGraphAttachmentLoad::Clear);
			graph.AddRasterPass("Clear first color", std::move(firstPass), nullptr);

			const TShared<render::Texture> secondColor = graph.CreateTexture("Second color", colorInformation);
			const TShared<render::Texture> depth = graph.CreateTexture("Depth", depthInformation);
			FrameGraphRasterPass secondPass;
			secondPass.SetColorAttachment(0, secondColor, FrameGraphAttachmentLoad::Clear);
			secondPass.SetDepthStencilAttachment(depth, FrameGraphDepthStencilMode::Write, FrameGraphAttachmentLoad::Clear);
			graph.AddRasterPass("Clear second color and depth", std::move(secondPass), nullptr);

			reads[0] = TextureUtility::ReadAsync(graph, firstColor);
			reads[1] = TextureUtility::ReadAsync(graph, secondColor);

			graph.Compile();
			B3D_TEST_ASSERT(graph.GetStatistics().CulledPassCount == 0)
			B3D_TEST_ASSERT(graph.GetStatistics().AliasAcquireCount == 3u)
			B3D_TEST_ASSERT(graph.GetStatistics().SplitAliasAcquireCount == 1u)
			graph.Execute();
		}

		// The command buffer's completion callbacks run on this thread while it waits
		device->WaitUntilIdle();
		for(const TAsyncOp<TShared<PixelData>>& read : reads)
		{
			B3D_TEST_ASSERT_MSG(read.HasCompleted(), "The read did not complete with the graph's command buffer.")
			if(!read.HasCompleted())
				return;

			const TShared<PixelData> colorData = read.GetReturnValue();
			bool isCleared = colorData != nullptr;
			for(u32 row = 0; row < kSize && isCleared; row++)
			{
				const u32* const rowTexels = reinterpret_cast<const u32*>(colorData->GetData() + row * colorData->GetRowPitch());
				for(u32 column = 0; column < kSize; column++)
					isCleared &= rowTexels[column] == 0;
			}

			B3D_TEST_ASSERT_MSG(isCleared, "The color attachment does not hold its clear value.")
		}

		device->EndFrame();
	});
}

void FrameGraphTestSuite::TestParameterHandles()
{
	const TShared<GpuDevice> device = GetTestDevice();
	const TShared<GpuComputePipelineState> pipeline = device != nullptr ? CreateTestDispatchPipeline(*device) : nullptr;
	if(pipeline == nullptr)
		return;

	LoggingScope logs(*this);
	logs.IgnoreWarning(kBarrierOnlyWarning);

	RunOnRenderThread("FrameGraphTestSuite::TestParameterHandles", [this, &device, &pipeline]()
	{
		const FrameGraphTestContext test(*device, pipeline);
		const TShared<GpuPipelineParameterSetLayout> layout = pipeline->GetParameterLayout()->GetSet(0);

		// The handles are looked up on one set and write the set of the pass
		const TShared<render::GpuParameterSet> handleParameters = test.Context->GetParameterSetPool().Create(layout, 0);
		render::GpuParameterStorageBuffer bufferParameter;
		render::GpuParameterStorageTexture textureParameter;
		handleParameters->GetStorageBufferParameter("OutputBuffer", bufferParameter);
		handleParameters->GetStorageTextureParameter("OutputTexture", textureParameter);

		const TShared<render::GpuBuffer> bufferReadback = test.CreateReadback(kSize * kSize * sizeof(u32));
		const TShared<render::GpuBuffer> textureReadback = test.CreateReadback(test.TextureReadbackSize);
		{
			FrameGraph graph(*test.Context, kGraphName);
			const TShared<render::GpuBuffer> buffer = graph.CreateBuffer("Handle buffer", GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32), kSize * kSize,
				GpuBufferFlag::StoreOnGPU | GpuBufferFlag::AllowUnorderedAccessOnTheGPU));
			const TShared<render::Texture> texture = graph.CreateTexture("Handle texture", test.TextureInformation);

			FrameGraphComputePass pass;
			const TShared<render::GpuParameterSet> parameters = test.Context->GetParameterSetPool().Create(layout, 0);
			bufferParameter.Set(parameters, buffer);
			textureParameter.Set(parameters, texture);
			pass.AddParameterSet(parameters);

			B3D_TEST_ASSERT_MSG(bufferParameter.Get(handleParameters) == nullptr && textureParameter.Get(handleParameters) == nullptr,
				"Setting another set changed the set the handles were looked up on.")

			graph.AddComputePass("Write", std::move(pass), [pipeline, parameters](FrameGraphPassContext& passContext)
			{
				render::GpuCommandBuffer& commands = passContext.GetCommandBuffer();
				commands.SetGpuComputePipelineState(pipeline);
				commands.SetGpuParameterSet(parameters);
				commands.DispatchCompute(1, 1, 1);
			});

			// The pass declared the transients it got through the handles, so the copy reads what the dispatch wrote
			test.AddReadback(graph, "Read", buffer, texture, bufferReadback, textureReadback);
			graph.Compile();
			B3D_TEST_ASSERT(graph.GetStatistics().CulledPassCount == 0)
			graph.Execute();
		}

		device->WaitUntilIdle();
		B3D_TEST_ASSERT_MSG(test.HasDispatchValues(bufferReadback, false), "The buffer set through the handle does not hold the dispatch's values.")
		B3D_TEST_ASSERT_MSG(test.HasDispatchValues(textureReadback, true), "The texture set through the handle does not hold the dispatch's values.")

		device->EndFrame();
	});
}

void FrameGraphTestSuite::TestExecuteCallbacks()
{
	const TShared<GpuDevice> device = GetTestDevice();
	const TShared<GpuComputePipelineState> pipeline = device != nullptr ? CreateTestDispatchPipeline(*device) : nullptr;
	if(pipeline == nullptr)
		return;

	LoggingScope logs(*this);
	logs.IgnoreWarning(kBarrierOnlyWarning);

	RunOnRenderThread("FrameGraphTestSuite::TestExecuteCallbacks", [this, &device, &pipeline]()
	{
		const FrameGraphTestContext test(*device, pipeline);
		const TShared<render::GpuBuffer> buffer = test.CreateBuffer("Buffer");
		const TShared<render::GpuBuffer> readback = test.CreateReadback(kSize * kSize * (u32)sizeof(u32));

		{
			bool isPassRecorded = false;
			Vector<u32> callbackOrder;

			FrameGraph graph(*test.Context, kGraphName);
			graph.ImportBuffer(buffer);
			graph.ImportBuffer(readback);

			FrameGraphCopyPass pass;
			pass.AddSource(buffer);
			pass.AddDestination(readback);
			graph.AddCopyPass("Copy", std::move(pass), [buffer, readback, &isPassRecorded](FrameGraphPassContext& passContext)
			{
				passContext.GetCommandBuffer().CopyBufferToBuffer(buffer, readback, 0, 0, kSize * kSize * sizeof(u32));
				isPassRecorded = true;
			});

			for(u32 callbackIndex = 0; callbackIndex < 2; callbackIndex++)
			{
				graph.AddExecuteCallback([this, callbackIndex, &isPassRecorded, &callbackOrder]()
				{
					B3D_TEST_ASSERT_MSG(isPassRecorded, "An execute callback ran before the pass functions.")
					callbackOrder.push_back(callbackIndex);
				});
			}

			graph.Compile();
			B3D_TEST_ASSERT(callbackOrder.empty())

			graph.Execute();
			B3D_TEST_ASSERT(callbackOrder.size() == 2 && callbackOrder[0] == 0 && callbackOrder[1] == 1)
		}

		{
			u32 callbackCount = 0;

			FrameGraph graph(*test.Context, kGraphName);
			graph.AddExecuteCallback([&callbackCount]() { callbackCount++; });
			graph.Compile();
			graph.Execute();
			B3D_TEST_ASSERT(callbackCount == 1)
		}

		device->WaitUntilIdle();
		device->EndFrame();
	});
}

#if B3D_BUILD_TYPE_DEVELOPMENT
void FrameGraphTestSuite::TestValidation()
{
	const TShared<GpuDevice> device = GetTestDevice();
	const TShared<GpuComputePipelineState> pipeline = device != nullptr ? CreateTestDispatchPipeline(*device) : nullptr;
	const TShared<GpuComputePipelineState> samplePipeline = device != nullptr ? CreateTestSamplePipeline(*device) : nullptr;
	if(pipeline == nullptr || samplePipeline == nullptr)
		return;

	const String drawDependency = "The resource was accessed by a draw or dispatch after another draw or dispatch wrote it, without a barrier in between.";

	LoggingScope logs(*this);
	logs.IgnoreWarning(kBarrierOnlyWarning);
	logs.ExpectError("Pass 'Resting write' writes 'Resting buffer' through slot 'OutputBuffer', but it is not a resource of the graph. Import it first.");
	logs.ExpectError("Pass 'Resting write' writes 'Resting texture' through slot 'OutputTexture', but it is not a resource of the graph. Import it first.");
	logs.ExpectError("Pass 'Conflict' accesses the same subresource of 'Conflict texture' in two layouts. No barrier can separate them, so split the pass.");
	logs.ExpectError("Pass 'After export' writes 'Exported buffer' after it was exported.");
	logs.ExpectError(drawDependency + " Buffer: 'Twice buffer'.");
	logs.ExpectError(drawDependency + " Image: 'Twice texture'.");
	logs.ExpectError(drawDependency + " Buffer: 'Undeclared buffer'.");
	logs.ExpectError(drawDependency + " Image: 'Undeclared texture'.");
	logs.ExpectError("The image was accessed in a different layout than its last barrier declares. Image: 'Unexported texture'.");
	logs.ExpectError("The resource was used while it is unplaced, without memory from its transient resource allocator. Place it first. Image: 'Held texture'.");

	RunOnRenderThread("FrameGraphTestSuite::TestValidation", [&device, &pipeline, &samplePipeline]()
	{
		const FrameGraphTestContext test(*device, pipeline, samplePipeline);

		// A storage slot can only hold resources of the graph
		{
			FrameGraph graph(*test.Context, kGraphName);
			test.AddDispatch(graph, "Resting write", test.CreateBuffer("Resting buffer"), test.CreateTexture("Resting texture"));
			graph.Compile();
			graph.Execute();
		}

		// No barrier can separate a copy's source and destination on the same subresource
		{
			FrameGraph graph(*test.Context, kGraphName);
			const TShared<render::Texture> texture = graph.CreateTexture("Conflict texture", test.TextureInformation);

			FrameGraphCopyPass pass;
			pass.AddSource(texture);
			pass.AddDestination(texture);
			graph.AddCopyPass("Conflict", std::move(pass), nullptr);
			graph.Compile();
			graph.Execute();
		}

		// Readers of an exported resource rely on its resting state
		{
			FrameGraph graph(*test.Context, kGraphName);
			const TShared<render::GpuBuffer> buffer = test.CreateBuffer("Exported buffer");
			const TShared<render::Texture> texture = test.CreateTexture("Exported texture");
			graph.ImportBuffer(buffer);
			graph.ImportTexture(texture);

			FrameGraphComputePass pass;
			const TShared<render::GpuParameterSet> parameters = test.Context->GetParameterSetPool().Create(pipeline->GetParameterLayout()->GetSet(0), 0);
			pass.AddParameterSet(parameters);
			parameters->SetStorageBuffer("OutputBuffer", buffer);
			parameters->SetStorageTexture("OutputTexture", texture, TextureSurface());
			pass.Export(buffer);

			const TShared<GpuComputePipelineState> dispatchPipeline = pipeline;
			graph.AddComputePass("Export", std::move(pass), [dispatchPipeline, parameters](FrameGraphPassContext& passContext)
			{
				passContext.GetCommandBuffer().SetGpuComputePipelineState(dispatchPipeline);
				passContext.GetCommandBuffer().SetGpuParameterSet(parameters);
				passContext.GetCommandBuffer().DispatchCompute(1, 1, 1);
			});

			test.AddDispatch(graph, "After export", buffer, texture);
			graph.Compile();
			graph.Execute();
		}

		// Two dispatches of one pass have no barrier between them
		{
			FrameGraph graph(*test.Context, kGraphName);
			const TShared<render::GpuBuffer> buffer = test.CreateBuffer("Twice buffer");
			const TShared<render::Texture> texture = test.CreateTexture("Twice texture");
			graph.ImportBuffer(buffer);
			graph.ImportTexture(texture);

			FrameGraphComputePass pass;
			const TShared<render::GpuParameterSet> parameters = test.Context->GetParameterSetPool().Create(pipeline->GetParameterLayout()->GetSet(0), 0);
			pass.AddParameterSet(parameters);
			parameters->SetStorageBuffer("OutputBuffer", buffer);
			parameters->SetStorageTexture("OutputTexture", texture, TextureSurface());

			const TShared<GpuComputePipelineState> dispatchPipeline = pipeline;
			graph.AddComputePass("Twice", std::move(pass), [dispatchPipeline, parameters](FrameGraphPassContext& passContext)
			{
				render::GpuCommandBuffer& commands = passContext.GetCommandBuffer();
				commands.SetGpuComputePipelineState(dispatchPipeline);
				commands.SetGpuParameterSet(parameters);
				commands.DispatchCompute(1, 1, 1);
				commands.DispatchCompute(1, 1, 1);
			});

			graph.Compile();
			graph.Execute();
		}

		// A set the pass did not create declares nothing, so the graph places no barrier for its resources
		{
			FrameGraph graph(*test.Context, kGraphName);
			const TShared<render::GpuBuffer> buffer = test.CreateBuffer("Undeclared buffer");
			const TShared<render::Texture> texture = test.CreateTexture("Undeclared texture");
			graph.ImportBuffer(buffer);
			graph.ImportTexture(texture);
			test.AddDispatch(graph, "Declared", buffer, texture);

			const TShared<render::GpuParameterSet> undeclared = test.Context->GetParameterSetPool().Create(pipeline->GetParameterLayout()->GetSet(0), 0);
			undeclared->SetStorageBuffer("OutputBuffer", buffer);
			undeclared->SetStorageTexture("OutputTexture", texture, TextureSurface());

			// The pass declares writes of its own resources, so it survives
			const TShared<render::GpuBuffer> otherBuffer = test.CreateBuffer("Other buffer");
			const TShared<render::Texture> otherTexture = test.CreateTexture("Other texture");
			graph.ImportBuffer(otherBuffer);
			graph.ImportTexture(otherTexture);

			FrameGraphComputePass pass;
			const TShared<render::GpuParameterSet> parameters = test.Context->GetParameterSetPool().Create(pipeline->GetParameterLayout()->GetSet(0), 0);
			pass.AddParameterSet(parameters);
			parameters->SetStorageBuffer("OutputBuffer", otherBuffer);
			parameters->SetStorageTexture("OutputTexture", otherTexture, TextureSurface());

			const TShared<GpuComputePipelineState> dispatchPipeline = pipeline;
			graph.AddComputePass("Undeclared", std::move(pass), [dispatchPipeline, parameters, undeclared](FrameGraphPassContext& passContext)
			{
				render::GpuCommandBuffer& commands = passContext.GetCommandBuffer();
				commands.SetGpuComputePipelineState(dispatchPipeline);
				commands.SetGpuParameterSet(parameters);
				commands.DispatchCompute(1, 1, 1);
				commands.SetGpuParameterSet(undeclared);
				commands.DispatchCompute(1, 1, 1);
			});

			graph.Compile();
			graph.Execute();
		}

		// Undeclared readers of a written resource rely on its export to return it to rest
		for(const bool isExported : { true, false })
		{
			FrameGraph graph(*test.Context, kGraphName);
			const TShared<render::GpuBuffer> buffer = test.CreateBuffer("Written buffer");
			const TShared<render::Texture> texture = test.CreateTexture(isExported ? "Exported texture" : "Unexported texture");
			const TShared<render::GpuBuffer> values = test.CreateBuffer("Read values");
			graph.ImportBuffer(buffer);
			graph.ImportTexture(texture);
			graph.ImportBuffer(values);
			test.AddDispatch(graph, "Write", buffer, texture, isExported);

			// The pass declares its write of the values, then samples the texture through a set it did not add
			FrameGraphComputePass pass;
			const TShared<render::GpuParameterSet> parameters = test.Context->GetParameterSetPool().Create(samplePipeline->GetParameterLayout()->GetSet(0), 0);
			pass.AddParameterSet(parameters);
			parameters->SetSampledTexture("InputTexture", test.CreateTexture("Resting texture"));
			parameters->SetStorageBuffer("OutputBuffer", values);

			const TShared<render::GpuParameterSet> undeclared = test.Context->GetParameterSetPool().Create(samplePipeline->GetParameterLayout()->GetSet(0), 0);
			undeclared->SetSampledTexture("InputTexture", texture);
			undeclared->SetStorageBuffer("OutputBuffer", values);

			const TShared<GpuComputePipelineState> dispatchPipeline = samplePipeline;
			graph.AddComputePass("Undeclared read", std::move(pass), [dispatchPipeline, undeclared](FrameGraphPassContext& passContext)
			{
				render::GpuCommandBuffer& commands = passContext.GetCommandBuffer();
				commands.SetGpuComputePipelineState(dispatchPipeline);
				commands.SetGpuParameterSet(undeclared);
				commands.DispatchCompute(1, 1, 1);
			});

			graph.Compile();
			graph.Execute();
		}

		// A transient that only culled passes use goes back to its allocator unplaced, and must not be used after the graph. The first
		// graph gives the cached texture memory and leaves it in the sampled layout, so the backends see a valid image.
		{
			const TShared<render::GpuBuffer> buffer = test.CreateBuffer("Held buffer");
			const TShared<render::GpuBuffer> values = test.CreateBuffer("Held values");
			{
				FrameGraph graph(*test.Context, kGraphName);
				graph.ImportBuffer(buffer);
				graph.ImportBuffer(values);
				const TShared<render::Texture> texture = graph.CreateTexture("Held texture", test.TextureInformation);
				test.AddDispatch(graph, "Place", buffer, texture);
				test.AddSample(graph, "Sample", texture, values);
				graph.Compile();
				graph.Execute();
			}

			TShared<render::Texture> held;
			{
				FrameGraph graph(*test.Context, kGraphName);
				held = graph.CreateTexture("Held texture", test.TextureInformation);
				test.AddDispatch(graph, "Culled", graph.CreateBuffer("Culled buffer", GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32), kSize * kSize,
					GpuBufferFlag::StoreOnGPU | GpuBufferFlag::AllowUnorderedAccessOnTheGPU)), held);
				graph.Compile();
				graph.Execute();
			}

			{
				FrameGraph graph(*test.Context, kGraphName);
				graph.ImportBuffer(values);
				test.AddSample(graph, "Held read", held, values);
				graph.Compile();
				graph.Execute();
			}
		}

		device->WaitUntilIdle();
		device->EndFrame();
	});
}
#endif

//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DFrameGraphTestSuite.h"
#include "Utility/B3DUnitTestGpuPipelines.h"
#include "Utility/B3DGpuBarrierTestUtility.h"
#include "CoreObject/B3DRenderThread.h"
#include "FrameGraph/B3DFrameGraph.h"
#include "GpuBackend/Allocators/B3DGpuAliasingAllocator.h"
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
	B3D_ADD_TEST(FrameGraphTestSuite::TestAsyncComputeProducer)
	B3D_ADD_TEST(FrameGraphTestSuite::TestAsyncComputeConsumer)
	B3D_ADD_TEST(FrameGraphTestSuite::TestSplitAcrossSubmissions)
	B3D_ADD_TEST(FrameGraphTestSuite::TestAliasAcquireAcrossSubmissions)
	B3D_ADD_TEST(FrameGraphTestSuite::TestSharedReaders)
	B3D_ADD_TEST(FrameGraphTestSuite::TestOwnershipTransfer)
	B3D_ADD_TEST(FrameGraphTestSuite::TestIndependentCompute)
	B3D_ADD_TEST(FrameGraphTestSuite::TestTransientAliasingAcrossQueues)
	B3D_ADD_TEST(FrameGraphTestSuite::TestAsyncComputeExtract)
	B3D_ADD_TEST(FrameGraphTestSuite::TestAsyncComputeDisabled)
	B3D_ADD_TEST(FrameGraphTestSuite::TestAsyncComputeExport)
	B3D_ADD_TEST(FrameGraphTestSuite::TestAsyncCopy)
	B3D_ADD_TEST(FrameGraphTestSuite::TestParallelRecording)
	B3D_ADD_TEST(FrameGraphTestSuite::TestParallelRecordingStress)

#if B3D_BUILD_TYPE_DEVELOPMENT
	B3D_ADD_TEST(FrameGraphTestSuite::TestValidation)
	B3D_ADD_TEST(FrameGraphTestSuite::TestAsyncComputeResting)
	B3D_ADD_TEST(FrameGraphTestSuite::TestInterleavedGraphs)
	B3D_ADD_TEST(FrameGraphTestSuite::TestAsyncRasterPass)
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

	/** Options of the graphs the tests create. TestParallelRecording() makes them record in parallel. */
	FrameGraphFlags gTestGraphFlags = FrameGraphFlag::None;

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

		/**
		 * Adds a pass with @p flags that dispatches the test pipeline into @p buffer and @p texture, and exports both after it if @p exports
		 * is true.
		 */
		void AddDispatch(FrameGraph& graph, const char* name, const TShared<render::GpuBuffer>& buffer, const TShared<render::Texture>& texture, bool exports = false,
			FrameGraphPassFlags flags = FrameGraphPassFlag::None) const
		{
			FrameGraphComputePass pass(flags);
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

		/** Adds a pass with @p flags that samples @p texture with the test sample pipeline, and writes its texels into @p buffer. */
		void AddSample(FrameGraph& graph, const char* name, const TShared<render::Texture>& texture, const TShared<render::GpuBuffer>& buffer,
			FrameGraphPassFlags flags = FrameGraphPassFlag::None) const
		{
			FrameGraphComputePass pass(flags);
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

		/**
		 * Adds a pass with @p flags that samples @p texture with the test sample pipeline through a parameter set it does not declare, so
		 * the texture is read at rest, and writes its texels into @p buffer, which it declares.
		 */
		void AddUndeclaredSample(FrameGraph& graph, const char* name, const TShared<render::Texture>& texture, const TShared<render::GpuBuffer>& buffer, FrameGraphPassFlags flags) const
		{
			FrameGraphComputePass pass(flags);

			// The declared set holds the buffer, and a texture both queues can read that the pass never binds
			const TShared<render::GpuParameterSet> declared = Context->GetParameterSetPool().Create(SamplePipeline->GetParameterLayout()->GetSet(0), 0);
			pass.AddParameterSet(declared);
			declared->SetSampledTexture("InputTexture", CreateTexture("Unbound texture", TextureUsageFlag::Default));
			declared->SetStorageBuffer("OutputBuffer", buffer);

			const TShared<render::GpuParameterSet> undeclared = Context->GetParameterSetPool().Create(SamplePipeline->GetParameterLayout()->GetSet(0), 0);
			undeclared->SetSampledTexture("InputTexture", texture);
			undeclared->SetStorageBuffer("OutputBuffer", buffer);

			// The graph flushes only the sets passes declare, and graphs that record in parallel bind sets on other threads
			undeclared->Flush();

			const TShared<GpuComputePipelineState> pipeline = SamplePipeline;
			graph.AddComputePass(name, std::move(pass), [pipeline, undeclared](FrameGraphPassContext& passContext)
			{
				render::GpuCommandBuffer& commands = passContext.GetCommandBuffer();
				commands.SetGpuComputePipelineState(pipeline);
				commands.SetGpuParameterSet(undeclared);
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

		/**
		 * Adds a pass with @p flags that copies @p buffer and @p texture into @p bufferReadback and @p textureReadback, which the graph
		 * imports.
		 */
		void AddReadback(FrameGraph& graph, const char* name, const TShared<render::GpuBuffer>& buffer, const TShared<render::Texture>& texture, const TShared<render::GpuBuffer>& bufferReadback,
			const TShared<render::GpuBuffer>& textureReadback, FrameGraphPassFlags flags = FrameGraphPassFlag::None) const
		{
			graph.ImportBuffer(bufferReadback);
			graph.ImportBuffer(textureReadback);

			FrameGraphCopyPass pass(flags);
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

	/** Returns true if @p device has a compute queue, which the async compute tests need. */
	bool HasComputeQueue(GpuDevice& device)
	{
		return device.GetQueueCount(GQT_COMPUTE) > 0;
	}

	/** Returns a buffer the test dispatch can write, created by @p graph as a transient. */
	TShared<render::GpuBuffer> CreateTransientBuffer(FrameGraph& graph, const char* name)
	{
		return graph.CreateBuffer(name, GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32), kSize * kSize, GpuBufferFlag::StoreOnGPU | GpuBufferFlag::AllowUnorderedAccessOnTheGPU));
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
			FrameGraph graph(*test.Context, kGraphName, gTestGraphFlags);
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
			FrameGraph graph(*test.Context, kGraphName, gTestGraphFlags);
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
			FrameGraph graph(*test.Context, kGraphName, gTestGraphFlags);

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
			FrameGraph graph(*test.Context, kGraphName, gTestGraphFlags);
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
			FrameGraph graph(*test.Context, kGraphName, gTestGraphFlags);
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
				FrameGraph graph(*test.Context, kGraphName, gTestGraphFlags);
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
				FrameGraph graph(*test.Context, kGraphName, gTestGraphFlags);
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
			FrameGraph graph(*test.Context, kGraphName, gTestGraphFlags);
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
			FrameGraph graph(*test.Context, kGraphName, gTestGraphFlags);
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
			FrameGraph graph(*test.Context, kGraphName, gTestGraphFlags);
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
			FrameGraph graph(*test.Context, kGraphName, gTestGraphFlags);
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
				FrameGraph graph(*test.Context, kGraphName, gTestGraphFlags);

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
			FrameGraph graph(*test.Context, kGraphName, gTestGraphFlags);
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
			FrameGraph graph(*test.Context, kProfiledGraphName, gTestGraphFlags);
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
		if(!HasComputeQueue(*device))
			return;

		// The async pass's command buffer is submitted first, then the graphics pass that does not depend on it, then the reader of its
		// results, each with a sample of its own
		constexpr const char* kAsyncGraphName = "Async profiled frame graph";
		const TShared<render::GpuBuffer> asyncReadback = test.CreateReadback(kSize * kSize * sizeof(u32));
		{
			FrameGraph graph(*test.Context, kAsyncGraphName, gTestGraphFlags);
			const TShared<render::GpuBuffer> asyncBuffer = CreateTransientBuffer(graph, "Async buffer");
			test.AddDispatch(graph, "Async write", asyncBuffer, graph.CreateTexture("Async texture", test.TextureInformation), false, FrameGraphPassFlag::AsyncCompute);

			graph.ImportBuffer(buffer);
			graph.ImportTexture(texture);
			test.AddDispatch(graph, "Graphics write", buffer, texture);
			test.AddReadback(graph, "Async read", asyncBuffer, asyncReadback);

			graph.Compile();
			B3D_TEST_ASSERT(graph.GetStatistics().SubmissionCount == 3)
			graph.Execute();
		}

		device->WaitUntilIdle();
		GetGpuProfiler().Update();

		const auto fnCheckCommandBufferSample = [this](const String& name, const char* passName)
		{
			const TOptional<GpuProfilerResults> commandBufferResults = GetGpuProfiler().GetResults(name.c_str());
			B3D_TEST_ASSERT_MSG(commandBufferResults.has_value(), "The GPU profiler did not resolve the samples of a command buffer of the graph.")
			if(!commandBufferResults.has_value())
				return;

			B3D_TEST_ASSERT(commandBufferResults->Samples.Size() == 1)
			if(commandBufferResults->Samples.Size() != 1)
				return;

			const GpuProfilerSample& commandBufferSample = commandBufferResults->Samples[0];
			B3D_TEST_ASSERT(commandBufferSample.Name == name.c_str())
			B3D_TEST_ASSERT(commandBufferSample.ChildSamples.Size() == 1 && commandBufferSample.ChildSamples[0].Name == passName)
		};

		fnCheckCommandBufferSample(String(kAsyncGraphName) + " Compute 0", "Async write");
		fnCheckCommandBufferSample(String(kAsyncGraphName) + " Graphics 0", "Graphics write");
		fnCheckCommandBufferSample(String(kAsyncGraphName) + " Graphics 1", "Async read");

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
			FrameGraph graph(*test.Context, kGraphName, gTestGraphFlags);
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

			FrameGraph graph(*test.Context, kGraphName, gTestGraphFlags);
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

			FrameGraph graph(*test.Context, kGraphName, gTestGraphFlags);
			graph.AddExecuteCallback([&callbackCount]() { callbackCount++; });
			graph.Compile();
			graph.Execute();
			B3D_TEST_ASSERT(callbackCount == 1)
		}

		device->WaitUntilIdle();
		device->EndFrame();
	});
}

void FrameGraphTestSuite::TestAsyncComputeProducer()
{
	const TShared<GpuDevice> device = GetTestDevice();
	const TShared<GpuComputePipelineState> pipeline = device != nullptr && HasComputeQueue(*device) ? CreateTestDispatchPipeline(*device) : nullptr;
	if(pipeline == nullptr)
		return;

	LoggingScope logs(*this);
	logs.IgnoreWarning(kBarrierOnlyWarning);

	RunOnRenderThread("FrameGraphTestSuite::TestAsyncComputeProducer", [this, &device, &pipeline]()
	{
		const FrameGraphTestContext test(*device, pipeline);
		const TShared<render::GpuBuffer> buffer = test.CreateBuffer("Async buffer");
		const TShared<render::Texture> texture = test.CreateTexture("Async texture");
		const TShared<render::GpuBuffer> bufferReadback = test.CreateReadback(kSize * kSize * sizeof(u32));
		const TShared<render::GpuBuffer> textureReadback = test.CreateReadback(test.TextureReadbackSize);

		{
			FrameGraph graph(*test.Context, kGraphName, gTestGraphFlags);
			graph.ImportBuffer(buffer);
			graph.ImportTexture(texture);
			test.AddDispatch(graph, "Producer", buffer, texture, false, FrameGraphPassFlag::AsyncCompute);
			test.AddReadback(graph, "Consumer", buffer, texture, bufferReadback, textureReadback);
			graph.Compile();

			// The prologue releases both resources to the compute queue, and the producer releases them back to the consumer. Each
			// submission waits for the one before it.
			const FrameGraphStatistics& statistics = graph.GetStatistics();
			B3D_TEST_ASSERT(statistics.SubmissionCount == 3)
			B3D_TEST_ASSERT(statistics.QueueWaitCount == 2)
			B3D_TEST_ASSERT(statistics.QueueTransferCount == 4)
			B3D_TEST_ASSERT(statistics.BarrierBatchCount == 2)

			graph.Execute();
		}

		device->WaitUntilIdle();
		B3D_TEST_ASSERT_MSG(test.HasDispatchValues(bufferReadback, false), "The graphics pass does not read what the async compute pass wrote into the buffer.")
		B3D_TEST_ASSERT_MSG(test.HasDispatchValues(textureReadback, true), "The graphics pass does not read what the async compute pass wrote into the texture.")

		device->EndFrame();
	});
}

void FrameGraphTestSuite::TestAsyncComputeConsumer()
{
	const TShared<GpuDevice> device = GetTestDevice();
	const bool hasComputeQueue = device != nullptr && HasComputeQueue(*device);
	const TShared<GpuComputePipelineState> pipeline = hasComputeQueue ? CreateTestDispatchPipeline(*device) : nullptr;
	const TShared<GpuComputePipelineState> samplePipeline = hasComputeQueue ? CreateTestSamplePipeline(*device) : nullptr;
	const TShared<GpuComputePipelineState> depthSamplePipeline = hasComputeQueue ? CreateTestDepthSamplePipeline(*device) : nullptr;
	if(pipeline == nullptr || samplePipeline == nullptr || depthSamplePipeline == nullptr)
		return;

	LoggingScope logs(*this);
	logs.IgnoreWarning(kBarrierOnlyWarning);

	RunOnRenderThread("FrameGraphTestSuite::TestAsyncComputeConsumer", [this, &device, &pipeline, &samplePipeline, &depthSamplePipeline]()
	{
		constexpr float kClearDepth = 0.5f;

		const FrameGraphTestContext test(*device, pipeline, samplePipeline);
		const TShared<render::GpuBuffer> buffer = test.CreateBuffer("Rendered buffer");
		const TShared<render::Texture> rendered = test.CreateTexture("Rendered texture", TextureUsageFlag::RenderTarget | TextureUsageFlag::AllowUnorderedAccessOnTheGPU);
		const TShared<render::GpuBuffer> colorReadback = test.CreateReadback(kSize * kSize * sizeof(u32));
		const TShared<render::GpuBuffer> depthReadback = test.CreateReadback(kSize * kSize * sizeof(u32));

		{
			FrameGraph graph(*test.Context, kGraphName, gTestGraphFlags);
			graph.ImportBuffer(buffer);
			graph.ImportTexture(rendered);
			test.AddDispatch(graph, "Render", buffer, rendered);

			TextureCreateInformation depthInformation;
			depthInformation.Width = kSize;
			depthInformation.Height = kSize;
			depthInformation.Format = PF_D32_S8X24;
			depthInformation.Usage = TextureUsageFlag::DepthStencil;
			depthInformation.ClearDepth = kClearDepth;
			const TShared<render::Texture> depth = graph.CreateTexture("Depth", depthInformation);

			FrameGraphRasterPass clearPass;
			clearPass.SetDepthStencilAttachment(depth, FrameGraphDepthStencilMode::Write, FrameGraphAttachmentLoad::Clear);
			graph.AddRasterPass("Clear depth", std::move(clearPass), nullptr);

			const TShared<render::GpuBuffer> colorValues = CreateTransientBuffer(graph, "Color values");
			test.AddSample(graph, "Sample color", rendered, colorValues, FrameGraphPassFlag::AsyncCompute);

			const TShared<render::GpuBuffer> depthValues = CreateTransientBuffer(graph, "Depth values");
			FrameGraphComputePass depthPass(FrameGraphPassFlag::AsyncCompute);
			const TShared<render::GpuParameterSet> depthParameters = test.Context->GetParameterSetPool().Create(depthSamplePipeline->GetParameterLayout()->GetSet(0), 0);
			depthPass.AddParameterSet(depthParameters);
			depthParameters->SetSampledTexture("InputTexture", depth);
			depthParameters->SetStorageBuffer("OutputBuffer", depthValues);

			graph.AddComputePass("Sample depth", std::move(depthPass), [&depthSamplePipeline, depthParameters](FrameGraphPassContext& passContext)
			{
				render::GpuCommandBuffer& commands = passContext.GetCommandBuffer();
				commands.SetGpuComputePipelineState(depthSamplePipeline);
				commands.SetGpuParameterSet(depthParameters);
				commands.DispatchCompute(1, 1, 1);
			});

			test.AddReadback(graph, "Read color", colorValues, colorReadback);
			test.AddReadback(graph, "Read depth", depthValues, depthReadback);
			graph.Compile();

			// The rendered texture and the depth go to the compute queue, the values back to the graphics queue, and the rendered
			// texture back to rest on the graphics queue at the end
			const FrameGraphStatistics& statistics = graph.GetStatistics();
			B3D_TEST_ASSERT(statistics.CulledPassCount == 0)
			B3D_TEST_ASSERT(statistics.SubmissionCount == 3)
			B3D_TEST_ASSERT(statistics.QueueTransferCount == 5)

			graph.Execute();
		}

		device->WaitUntilIdle();
		B3D_TEST_ASSERT_MSG(test.HasDispatchValues(colorReadback, false), "The async compute pass does not sample what the graphics dispatch wrote.")

		const render::GpuBufferMappedScope mapping = depthReadback->Map(GpuMapOption::Read);
		B3D_TEST_ASSERT(mapping.IsValid())
		if(mapping.IsValid())
		{
			u32 clearDepthBits = 0;
			std::memcpy(&clearDepthBits, &kClearDepth, sizeof(clearDepthBits));

			bool isCleared = true;
			const u32* const depthBits = static_cast<const u32*>(mapping.GetMappedMemory());
			for(u32 element = 0; element < kSize * kSize; element++)
				isCleared &= depthBits[element] == clearDepthBits;

			B3D_TEST_ASSERT_MSG(isCleared, "The async compute pass does not sample the depth the raster pass cleared.")
		}

		device->EndFrame();
	});
}

void FrameGraphTestSuite::TestSplitAcrossSubmissions()
{
	const TShared<GpuDevice> device = GetTestDevice();
	const TShared<GpuComputePipelineState> pipeline = device != nullptr && HasComputeQueue(*device) ? CreateTestDispatchPipeline(*device) : nullptr;
	if(pipeline == nullptr)
		return;

	LoggingScope logs(*this);
	logs.IgnoreWarning(kBarrierOnlyWarning);

	RunOnRenderThread("FrameGraphTestSuite::TestSplitAcrossSubmissions", [this, &device, &pipeline]()
	{
		const FrameGraphTestContext test(*device, pipeline);
		const TShared<render::GpuBuffer> buffer = test.CreateBuffer("Graphics buffer");
		const TShared<render::Texture> texture = test.CreateTexture("Graphics texture");

		TShared<render::GpuBuffer> readbacks[4];
		for(u32 readbackIndex = 0; readbackIndex < 4; readbackIndex++)
			readbacks[readbackIndex] = test.CreateReadback(readbackIndex % 2 == 0 ? kSize * kSize * (u32)sizeof(u32) : test.TextureReadbackSize);

		{
			FrameGraph graph(*test.Context, kGraphName, gTestGraphFlags);
			graph.ImportBuffer(buffer);
			graph.ImportTexture(texture);
			test.AddDispatch(graph, "Graphics write", buffer, texture);

			const TShared<render::GpuBuffer> asyncBuffer = CreateTransientBuffer(graph, "Async buffer");
			const TShared<render::Texture> asyncTexture = graph.CreateTexture("Async texture", test.TextureInformation);
			test.AddDispatch(graph, "Async write", asyncBuffer, asyncTexture, false, FrameGraphPassFlag::AsyncCompute);

			// The first read waits for the async pass, which starts a graphics submission. The second read's barrier is released in the
			// submission before it.
			test.AddReadback(graph, "Async read", asyncBuffer, asyncTexture, readbacks[2], readbacks[3]);
			test.AddReadback(graph, "Graphics read", buffer, texture, readbacks[0], readbacks[1]);
			graph.Compile();

			const FrameGraphStatistics& statistics = graph.GetStatistics();
			B3D_TEST_ASSERT(statistics.SubmissionCount == 3)
			B3D_TEST_ASSERT(statistics.QueueWaitCount == 1)
			B3D_TEST_ASSERT(statistics.QueueTransferCount == 2)
			B3D_TEST_ASSERT(statistics.SplitBarrierCount >= 4)

			graph.Execute();
		}

		device->WaitUntilIdle();
		B3D_TEST_ASSERT_MSG(test.HasDispatchValues(readbacks[0], false), "The graphics buffer does not hold its values after a split barrier across submissions.")
		B3D_TEST_ASSERT_MSG(test.HasDispatchValues(readbacks[1], true), "The graphics texture does not hold its values after a split barrier across submissions.")
		B3D_TEST_ASSERT_MSG(test.HasDispatchValues(readbacks[2], false), "The async buffer does not hold its values.")
		B3D_TEST_ASSERT_MSG(test.HasDispatchValues(readbacks[3], true), "The async texture does not hold its values.")

		device->EndFrame();
	});
}

void FrameGraphTestSuite::TestAliasAcquireAcrossSubmissions()
{
	const TShared<GpuDevice> device = GetTestDevice();
	const TShared<GpuComputePipelineState> pipeline = device != nullptr && HasComputeQueue(*device) ? CreateTestDispatchPipeline(*device) : nullptr;
	if(pipeline == nullptr)
		return;

	LoggingScope logs(*this);
	logs.IgnoreWarning(kBarrierOnlyWarning);

	const bool isAliasingEnabled = gGpuTransientAliasing;
	for(const bool isAliasing : { true, false })
	{
		gGpuTransientAliasing.Set(isAliasing);
		RunOnRenderThread("FrameGraphTestSuite::TestAliasAcquireAcrossSubmissions", [this, &device, &pipeline, isAliasing]()
		{
			const FrameGraphTestContext test(*device, pipeline);

			TShared<render::GpuBuffer> buffers[3];
			for(u32 bufferIndex = 0; bufferIndex < 3; bufferIndex++)
				buffers[bufferIndex] = test.CreateBuffer("Buffer");

			const TShared<render::Texture> asyncTexture = test.CreateTexture("Async texture");

			TShared<render::GpuBuffer> readbacks[6];
			for(u32 readbackIndex = 0; readbackIndex < 6; readbackIndex++)
				readbacks[readbackIndex] = test.CreateReadback(readbackIndex % 2 == 0 ? kSize * kSize * (u32)sizeof(u32) : test.TextureReadbackSize);

			{
				FrameGraph graph(*test.Context, kGraphName, gTestGraphFlags);
				for(const TShared<render::GpuBuffer>& buffer : buffers)
					graph.ImportBuffer(buffer);

				graph.ImportTexture(asyncTexture);

				const TShared<render::Texture> firstTexture = graph.CreateTexture("First texture", test.TextureInformation);
				test.AddDispatch(graph, "First write", buffers[0], firstTexture);
				test.AddReadback(graph, "First read", buffers[0], firstTexture, readbacks[0], readbacks[1]);

				// The read of the async pass's results starts a graphics submission
				test.AddDispatch(graph, "Async write", buffers[1], asyncTexture, false, FrameGraphPassFlag::AsyncCompute);
				test.AddReadback(graph, "Async read", buffers[1], asyncTexture, readbacks[2], readbacks[3]);

				// The second texture takes the memory of the first one, which the previous graphics submission released
				const TShared<render::Texture> secondTexture = graph.CreateTexture("Second texture", test.TextureInformation);
				test.AddDispatch(graph, "Second write", buffers[2], secondTexture);
				test.AddReadback(graph, "Second read", buffers[2], secondTexture, readbacks[4], readbacks[5]);

				graph.Compile();
				B3D_TEST_ASSERT(graph.GetStatistics().SubmissionCount == 4)
				if(isAliasing)
				{
					B3D_TEST_ASSERT(graph.GetStatistics().AliasAcquireCount == 2u)
					B3D_TEST_ASSERT(graph.GetStatistics().SplitAliasAcquireCount == 1u)
				}

				graph.Execute();
			}

			device->WaitUntilIdle();
			for(u32 readbackIndex = 0; readbackIndex < 6; readbackIndex++)
				B3D_TEST_ASSERT_MSG(test.HasDispatchValues(readbacks[readbackIndex], readbackIndex % 2 == 1), "A resource does not hold its values.")

			device->EndFrame();
		});
	}

	gGpuTransientAliasing.Set(isAliasingEnabled);
}

void FrameGraphTestSuite::TestSharedReaders()
{
	const TShared<GpuDevice> device = GetTestDevice();
	const bool hasComputeQueue = device != nullptr && HasComputeQueue(*device);
	const TShared<GpuComputePipelineState> pipeline = hasComputeQueue ? CreateTestDispatchPipeline(*device) : nullptr;
	const TShared<GpuComputePipelineState> samplePipeline = hasComputeQueue ? CreateTestSamplePipeline(*device) : nullptr;
	if(pipeline == nullptr || samplePipeline == nullptr)
		return;

	LoggingScope logs(*this);
	logs.IgnoreWarning(kBarrierOnlyWarning);

	RunOnRenderThread("FrameGraphTestSuite::TestSharedReaders", [this, &device, &pipeline, &samplePipeline]()
	{
		const FrameGraphTestContext test(*device, pipeline, samplePipeline);
		for(const bool isWrittenAfter : { false, true })
		{
			const TShared<render::GpuBuffer> buffer = test.CreateBuffer("Shared buffer");
			const TShared<render::Texture> texture = test.CreateTexture("Shared texture", TextureUsageFlag::AllowUnorderedAccessOnTheGPU | TextureUsageFlag::AllowConcurrentQueueReads);
			const TShared<render::GpuBuffer> graphicsReadback = test.CreateReadback(kSize * kSize * sizeof(u32));
			const TShared<render::GpuBuffer> asyncReadback = test.CreateReadback(kSize * kSize * sizeof(u32));

			{
				FrameGraph graph(*test.Context, kGraphName, gTestGraphFlags);
				graph.ImportBuffer(buffer);
				graph.ImportTexture(texture);
				test.AddDispatch(graph, "Write", buffer, texture);

				// Both reads share the read state the barrier before the graphics read establishes
				const TShared<render::GpuBuffer> graphicsValues = CreateTransientBuffer(graph, "Graphics values");
				const TShared<render::GpuBuffer> asyncValues = CreateTransientBuffer(graph, "Async values");
				test.AddSample(graph, "Graphics read", texture, graphicsValues);
				test.AddSample(graph, "Async read", texture, asyncValues, FrameGraphPassFlag::AsyncCompute);

				// The write waits for the async read, and its barrier orders the graphics read
				if(isWrittenAfter)
					test.AddDispatch(graph, "Write after reads", buffer, texture);

				test.AddReadback(graph, "Read async values", asyncValues, asyncReadback);
				test.AddReadback(graph, "Read graphics values", graphicsValues, graphicsReadback);
				graph.Compile();

				// Only the async values are transferred, from the compute queue to the graphics queue
				const FrameGraphStatistics& statistics = graph.GetStatistics();
				B3D_TEST_ASSERT(statistics.SubmissionCount == 3)
				B3D_TEST_ASSERT(statistics.QueueWaitCount == 2)
				B3D_TEST_ASSERT(statistics.QueueTransferCount == 1)

				graph.Execute();
			}

			device->WaitUntilIdle();
			B3D_TEST_ASSERT_MSG(test.HasDispatchValues(graphicsReadback, false), "The graphics read of the shared texture does not see the write.")
			B3D_TEST_ASSERT_MSG(test.HasDispatchValues(asyncReadback, false), "The async compute read of the shared texture does not see the write.")

			device->EndFrame();
		}
	});
}

void FrameGraphTestSuite::TestOwnershipTransfer()
{
	const TShared<GpuDevice> device = GetTestDevice();
	const bool hasComputeQueue = device != nullptr && HasComputeQueue(*device);
	const TShared<GpuComputePipelineState> pipeline = hasComputeQueue ? CreateTestDispatchPipeline(*device) : nullptr;
	const TShared<GpuComputePipelineState> samplePipeline = hasComputeQueue ? CreateTestSamplePipeline(*device) : nullptr;
	if(pipeline == nullptr || samplePipeline == nullptr)
		return;

	LoggingScope logs(*this);
	logs.IgnoreWarning(kBarrierOnlyWarning);

	// The later graph's undeclared reads are checked against the resting state
	const bool validateRestingStates = gGpuValidateRestingStates;
	gGpuValidateRestingStates.Set(true);

	RunOnRenderThread("FrameGraphTestSuite::TestOwnershipTransfer", [this, &device, &pipeline, &samplePipeline]()
	{
		const FrameGraphTestContext test(*device, pipeline, samplePipeline);
		const TShared<render::GpuBuffer> buffer = test.CreateBuffer("Rendered buffer");
		const TShared<render::Texture> rendered = test.CreateTexture("Rendered texture", TextureUsageFlag::RenderTarget | TextureUsageFlag::AllowUnorderedAccessOnTheGPU);
		const TShared<render::GpuBuffer> asyncValues = test.CreateBuffer("Async values");

		{
			FrameGraph graph(*test.Context, kGraphName, gTestGraphFlags);
			graph.ImportBuffer(buffer);
			graph.ImportTexture(rendered);
			graph.ImportBuffer(asyncValues);
			test.AddDispatch(graph, "Render", buffer, rendered);
			test.AddSample(graph, "Async sample", rendered, asyncValues, FrameGraphPassFlag::AsyncCompute);
			graph.Compile();

			// A prologue releases the values to the compute queue, and an epilogue acquires both resources back at the end
			const FrameGraphStatistics& statistics = graph.GetStatistics();
			B3D_TEST_ASSERT(statistics.SubmissionCount == 4)
			B3D_TEST_ASSERT(statistics.QueueWaitCount == 2)
			B3D_TEST_ASSERT(statistics.QueueTransferCount == 4)

			graph.Execute();
		}

		device->EndFrame();

		// A later frame samples the texture at rest without importing it, and reads the values
		const TShared<render::GpuBuffer> sampledValues = test.CreateBuffer("Sampled values");
		const TShared<render::GpuBuffer> asyncReadback = test.CreateReadback(kSize * kSize * sizeof(u32));
		const TShared<render::GpuBuffer> sampledReadback = test.CreateReadback(kSize * kSize * sizeof(u32));
		{
			FrameGraph graph(*test.Context, kGraphName, gTestGraphFlags);
			graph.ImportBuffer(asyncValues);
			graph.ImportBuffer(sampledValues);
			test.AddSample(graph, "Sample at rest", rendered, sampledValues);
			test.AddReadback(graph, "Read async values", asyncValues, asyncReadback);
			test.AddReadback(graph, "Read sampled values", sampledValues, sampledReadback);

			graph.Compile();
			graph.Execute();
		}

		device->WaitUntilIdle();
		B3D_TEST_ASSERT_MSG(test.HasDispatchValues(asyncReadback, false), "The async compute pass does not sample what the graphics dispatch wrote.")
		B3D_TEST_ASSERT_MSG(test.HasDispatchValues(sampledReadback, false), "The texture is not at rest on the graphics queue after the graph.")

		device->EndFrame();
	});

	gGpuValidateRestingStates.Set(validateRestingStates);
}

void FrameGraphTestSuite::TestIndependentCompute()
{
	const TShared<GpuDevice> device = GetTestDevice();
	const TShared<GpuComputePipelineState> pipeline = device != nullptr && HasComputeQueue(*device) ? CreateTestDispatchPipeline(*device) : nullptr;
	if(pipeline == nullptr)
		return;

	LoggingScope logs(*this);
	logs.IgnoreWarning(kBarrierOnlyWarning);

	RunOnRenderThread("FrameGraphTestSuite::TestIndependentCompute", [this, &device, &pipeline]()
	{
		const FrameGraphTestContext test(*device, pipeline);
		const TShared<render::GpuBuffer> buffer = test.CreateBuffer("Graphics buffer");
		const TShared<render::Texture> texture = test.CreateTexture("Graphics texture");
		const TShared<render::GpuBuffer> bufferReadback = test.CreateReadback(kSize * kSize * sizeof(u32));
		const TShared<render::GpuBuffer> textureReadback = test.CreateReadback(test.TextureReadbackSize);

		{
			FrameGraph graph(*test.Context, kGraphName, gTestGraphFlags);
			graph.ImportBuffer(buffer);
			graph.ImportTexture(texture);
			test.AddDispatch(graph, "Graphics write", buffer, texture);

			const TShared<render::GpuBuffer> asyncBuffer = CreateTransientBuffer(graph, "Async buffer");
			const TShared<render::Texture> asyncTexture = graph.CreateTexture("Async texture", test.TextureInformation);
			test.AddDispatch(graph, "Async write", asyncBuffer, asyncTexture, false, FrameGraphPassFlag::AsyncCompute);
			test.AddReadback(graph, "Async read", asyncBuffer, asyncTexture, bufferReadback, textureReadback);
			graph.Compile();

			// Only the reader waits for a submission of the graph
			const FrameGraphStatistics& statistics = graph.GetStatistics();
			B3D_TEST_ASSERT(statistics.SubmissionCount == 3)
			B3D_TEST_ASSERT(statistics.QueueWaitCount == 1)
			B3D_TEST_ASSERT(statistics.QueueTransferCount == 2)

			graph.Execute();
		}

		device->WaitUntilIdle();
		B3D_TEST_ASSERT_MSG(test.HasDispatchValues(bufferReadback, false), "The async buffer does not hold its values.")
		B3D_TEST_ASSERT_MSG(test.HasDispatchValues(textureReadback, true), "The async texture does not hold its values.")

		device->EndFrame();
	});
}

void FrameGraphTestSuite::TestTransientAliasingAcrossQueues()
{
	const TShared<GpuDevice> device = GetTestDevice();
	const TShared<GpuComputePipelineState> pipeline = device != nullptr && HasComputeQueue(*device) ? CreateTestDispatchPipeline(*device) : nullptr;
	if(pipeline == nullptr)
		return;

	LoggingScope logs(*this);
	logs.IgnoreWarning(kBarrierOnlyWarning);

	const bool isAliasingEnabled = gGpuTransientAliasing;
	for(const bool isAliasing : { true, false })
	{
		gGpuTransientAliasing.Set(isAliasing);
		RunOnRenderThread("FrameGraphTestSuite::TestTransientAliasingAcrossQueues", [this, &device, &pipeline]()
		{
			const FrameGraphTestContext test(*device, pipeline);

			TShared<render::GpuBuffer> readbacks[6];
			for(u32 readbackIndex = 0; readbackIndex < 6; readbackIndex++)
				readbacks[readbackIndex] = test.CreateReadback(readbackIndex % 2 == 0 ? kSize * kSize * (u32)sizeof(u32) : test.TextureReadbackSize);

			{
				FrameGraph graph(*test.Context, kGraphName, gTestGraphFlags);

				// The first async and graphics transients overlap in time on different queues. The second async transients come after
				// the graphics reads on the compute queue, but nothing orders them after those reads.
				const TShared<render::GpuBuffer> firstAsyncBuffer = CreateTransientBuffer(graph, "First async buffer");
				const TShared<render::Texture> firstAsyncTexture = graph.CreateTexture("First async texture", test.TextureInformation);
				test.AddDispatch(graph, "First async write", firstAsyncBuffer, firstAsyncTexture, false, FrameGraphPassFlag::AsyncCompute);

				const TShared<render::GpuBuffer> graphicsBuffer = CreateTransientBuffer(graph, "Graphics buffer");
				const TShared<render::Texture> graphicsTexture = graph.CreateTexture("Graphics texture", test.TextureInformation);
				test.AddDispatch(graph, "Graphics write", graphicsBuffer, graphicsTexture);

				test.AddReadback(graph, "First async read", firstAsyncBuffer, firstAsyncTexture, readbacks[0], readbacks[1]);
				test.AddReadback(graph, "Graphics read", graphicsBuffer, graphicsTexture, readbacks[2], readbacks[3]);

				const TShared<render::GpuBuffer> secondAsyncBuffer = CreateTransientBuffer(graph, "Second async buffer");
				const TShared<render::Texture> secondAsyncTexture = graph.CreateTexture("Second async texture", test.TextureInformation);
				test.AddDispatch(graph, "Second async write", secondAsyncBuffer, secondAsyncTexture, false, FrameGraphPassFlag::AsyncCompute);
				test.AddReadback(graph, "Second async read", secondAsyncBuffer, secondAsyncTexture, readbacks[4], readbacks[5]);

				graph.Compile();
				B3D_TEST_ASSERT(graph.GetStatistics().CulledPassCount == 0)
				graph.Execute();
			}

			device->WaitUntilIdle();
			for(u32 readbackIndex = 0; readbackIndex < 6; readbackIndex++)
				B3D_TEST_ASSERT_MSG(test.HasDispatchValues(readbacks[readbackIndex], readbackIndex % 2 == 1), "A transient does not hold its values.")

			device->EndFrame();
		});
	}

	gGpuTransientAliasing.Set(isAliasingEnabled);
}

void FrameGraphTestSuite::TestAsyncComputeExtract()
{
	const TShared<GpuDevice> device = GetTestDevice();
	const TShared<GpuComputePipelineState> pipeline = device != nullptr && HasComputeQueue(*device) ? CreateTestDispatchPipeline(*device) : nullptr;
	if(pipeline == nullptr)
		return;

	LoggingScope logs(*this);
	logs.IgnoreWarning(kBarrierOnlyWarning);

	RunOnRenderThread("FrameGraphTestSuite::TestAsyncComputeExtract", [this, &device, &pipeline]()
	{
		const FrameGraphTestContext test(*device, pipeline);

		TShared<render::GpuBuffer> extractedBuffer;
		TShared<render::Texture> extractedTexture;
		{
			FrameGraph graph(*test.Context, kGraphName, gTestGraphFlags);
			extractedBuffer = CreateTransientBuffer(graph, "Extracted buffer");
			extractedTexture = graph.CreateTexture("Extracted texture", test.TextureInformation);
			test.AddDispatch(graph, "Async write", extractedBuffer, extractedTexture, false, FrameGraphPassFlag::AsyncCompute);
			graph.ExtractBuffer(extractedBuffer);
			graph.ExtractTexture(extractedTexture);
			graph.Compile();

			// The epilogue acquires both back to rest on the graphics queue
			const FrameGraphStatistics& statistics = graph.GetStatistics();
			B3D_TEST_ASSERT(statistics.SubmissionCount == 2)
			B3D_TEST_ASSERT(statistics.QueueWaitCount == 1)
			B3D_TEST_ASSERT(statistics.QueueTransferCount == 2)

			graph.Execute();
		}

		device->EndFrame();

		const TShared<render::GpuBuffer> bufferReadback = test.CreateReadback(kSize * kSize * sizeof(u32));
		const TShared<render::GpuBuffer> textureReadback = test.CreateReadback(test.TextureReadbackSize);
		{
			FrameGraph graph(*test.Context, kGraphName, gTestGraphFlags);
			graph.ImportBuffer(extractedBuffer);
			graph.ImportTexture(extractedTexture);
			test.AddReadback(graph, "Read", extractedBuffer, extractedTexture, bufferReadback, textureReadback);
			graph.Compile();
			graph.Execute();
		}

		device->WaitUntilIdle();
		B3D_TEST_ASSERT_MSG(test.HasDispatchValues(bufferReadback, false), "The extracted buffer does not hold the values the async compute pass wrote.")
		B3D_TEST_ASSERT_MSG(test.HasDispatchValues(textureReadback, true), "The extracted texture does not hold the values the async compute pass wrote.")

		device->EndFrame();
	});
}

void FrameGraphTestSuite::TestAsyncComputeDisabled()
{
	const TShared<GpuDevice> device = GetTestDevice();
	const TShared<GpuComputePipelineState> pipeline = device != nullptr ? CreateTestDispatchPipeline(*device) : nullptr;
	if(pipeline == nullptr)
		return;

	LoggingScope logs(*this);
	logs.IgnoreWarning(kBarrierOnlyWarning);

	const bool isAsyncComputeEnabled = gFrameGraphAsyncCompute;
	gFrameGraphAsyncCompute.Set(false);

	RunOnRenderThread("FrameGraphTestSuite::TestAsyncComputeDisabled", [this, &device, &pipeline]()
	{
		const FrameGraphTestContext test(*device, pipeline);
		const TShared<render::GpuBuffer> buffer = test.CreateBuffer("Async buffer");
		const TShared<render::Texture> texture = test.CreateTexture("Async texture");
		const TShared<render::GpuBuffer> bufferReadback = test.CreateReadback(kSize * kSize * sizeof(u32));
		const TShared<render::GpuBuffer> textureReadback = test.CreateReadback(test.TextureReadbackSize);

		{
			FrameGraph graph(*test.Context, kGraphName, gTestGraphFlags);
			graph.ImportBuffer(buffer);
			graph.ImportTexture(texture);
			test.AddDispatch(graph, "Producer", buffer, texture, false, FrameGraphPassFlag::AsyncCompute);
			test.AddReadback(graph, "Consumer", buffer, texture, bufferReadback, textureReadback);
			graph.Compile();

			const FrameGraphStatistics& statistics = graph.GetStatistics();
			B3D_TEST_ASSERT(statistics.SubmissionCount == 1)
			B3D_TEST_ASSERT(statistics.QueueWaitCount == 0)
			B3D_TEST_ASSERT(statistics.QueueTransferCount == 0)

			graph.Execute();
		}

		device->WaitUntilIdle();
		B3D_TEST_ASSERT_MSG(test.HasDispatchValues(bufferReadback, false), "The consumer does not read what the producer wrote into the buffer.")
		B3D_TEST_ASSERT_MSG(test.HasDispatchValues(textureReadback, true), "The consumer does not read what the producer wrote into the texture.")

		device->EndFrame();
	});

	gFrameGraphAsyncCompute.Set(isAsyncComputeEnabled);
}

void FrameGraphTestSuite::TestAsyncComputeExport()
{
	const TShared<GpuDevice> device = GetTestDevice();
	const bool hasComputeQueue = device != nullptr && HasComputeQueue(*device);
	const TShared<GpuComputePipelineState> pipeline = hasComputeQueue ? CreateTestDispatchPipeline(*device) : nullptr;
	const TShared<GpuComputePipelineState> samplePipeline = hasComputeQueue ? CreateTestSamplePipeline(*device) : nullptr;
	if(pipeline == nullptr || samplePipeline == nullptr)
		return;

	LoggingScope logs(*this);
	logs.IgnoreWarning(kBarrierOnlyWarning);

	RunOnRenderThread("FrameGraphTestSuite::TestAsyncComputeExport", [this, &device, &pipeline, &samplePipeline]()
	{
		const FrameGraphTestContext test(*device, pipeline, samplePipeline);

		// Resources both queues can read rest the same way on either queue after the export
		const TShared<render::GpuBuffer> sharedBuffer = test.CreateBuffer("Exported buffer");
		const TShared<render::Texture> sharedTexture = test.CreateTexture("Exported shared texture", TextureUsageFlag::AllowUnorderedAccessOnTheGPU | TextureUsageFlag::AllowConcurrentQueueReads);
		const TShared<render::GpuBuffer> asyncValues = test.CreateBuffer("Async values");
		const TShared<render::GpuBuffer> graphicsValues = test.CreateBuffer("Graphics values");
		const TShared<render::GpuBuffer> asyncReadback = test.CreateReadback(kSize * kSize * sizeof(u32));
		const TShared<render::GpuBuffer> graphicsReadback = test.CreateReadback(kSize * kSize * sizeof(u32));
		{
			FrameGraph graph(*test.Context, kGraphName, gTestGraphFlags);
			graph.ImportBuffer(sharedBuffer);
			graph.ImportTexture(sharedTexture);
			graph.ImportBuffer(asyncValues);
			graph.ImportBuffer(graphicsValues);
			test.AddDispatch(graph, "Async export", sharedBuffer, sharedTexture, true, FrameGraphPassFlag::AsyncCompute);
			test.AddUndeclaredSample(graph, "Async read", sharedTexture, asyncValues, FrameGraphPassFlag::AsyncCompute);

			// The read of the async values waits for the async passes, so the graphics read after it sees the export
			test.AddReadback(graph, "Read async values", asyncValues, asyncReadback);
			test.AddUndeclaredSample(graph, "Graphics read", sharedTexture, graphicsValues, FrameGraphPassFlag::None);
			test.AddReadback(graph, "Read graphics values", graphicsValues, graphicsReadback);

			graph.Compile();
			graph.Execute();
		}

		device->WaitUntilIdle();
		B3D_TEST_ASSERT_MSG(test.HasDispatchValues(asyncReadback, false), "The async compute read of the exported shared texture does not see the export.")
		B3D_TEST_ASSERT_MSG(test.HasDispatchValues(graphicsReadback, false), "The graphics read of the exported shared texture does not see the export.")

		device->EndFrame();

		// A texture that rests on the graphics queue returns there before the first graphics pass after the wait for the export
		const TShared<render::Texture> exclusiveTexture = test.CreateTexture("Exported texture");
		const TShared<render::GpuBuffer> sampledValues = test.CreateBuffer("Sampled values");
		const TShared<render::GpuBuffer> bufferReadback = test.CreateReadback(kSize * kSize * sizeof(u32));
		const TShared<render::GpuBuffer> sampledReadback = test.CreateReadback(kSize * kSize * sizeof(u32));
		{
			FrameGraph graph(*test.Context, kGraphName, gTestGraphFlags);
			graph.ImportTexture(exclusiveTexture);
			graph.ImportBuffer(sampledValues);

			const TShared<render::GpuBuffer> asyncBuffer = CreateTransientBuffer(graph, "Async buffer");
			FrameGraphComputePass pass(FrameGraphPassFlag::AsyncCompute);
			const TShared<render::GpuParameterSet> parameters = test.Context->GetParameterSetPool().Create(pipeline->GetParameterLayout()->GetSet(0), 0);
			pass.AddParameterSet(parameters);
			parameters->SetStorageBuffer("OutputBuffer", asyncBuffer);
			parameters->SetStorageTexture("OutputTexture", exclusiveTexture, TextureSurface());
			pass.Export(exclusiveTexture);

			const TShared<GpuComputePipelineState> dispatchPipeline = pipeline;
			graph.AddComputePass("Async export", std::move(pass), [dispatchPipeline, parameters](FrameGraphPassContext& passContext)
			{
				render::GpuCommandBuffer& commands = passContext.GetCommandBuffer();
				commands.SetGpuComputePipelineState(dispatchPipeline);
				commands.SetGpuParameterSet(parameters);
				commands.DispatchCompute(1, 1, 1);
			});

			test.AddReadback(graph, "Read async buffer", asyncBuffer, bufferReadback);
			test.AddUndeclaredSample(graph, "Graphics read", exclusiveTexture, sampledValues, FrameGraphPassFlag::None);
			test.AddReadback(graph, "Read sampled values", sampledValues, sampledReadback);

			graph.Compile();
			graph.Execute();
		}

		device->WaitUntilIdle();
		B3D_TEST_ASSERT_MSG(test.HasDispatchValues(bufferReadback, false), "The async buffer does not hold its values.")
		B3D_TEST_ASSERT_MSG(test.HasDispatchValues(sampledReadback, false), "The graphics read of the exported texture does not see the export.")

		device->EndFrame();
	});
}

void FrameGraphTestSuite::TestAsyncCopy()
{
	const TShared<GpuDevice> device = GetTestDevice();
	const TShared<GpuComputePipelineState> pipeline = device != nullptr && HasComputeQueue(*device) ? CreateTestDispatchPipeline(*device) : nullptr;
	if(pipeline == nullptr)
		return;

	LoggingScope logs(*this);
	logs.IgnoreWarning(kBarrierOnlyWarning);

	RunOnRenderThread("FrameGraphTestSuite::TestAsyncCopy", [this, &device, &pipeline]()
	{
		const FrameGraphTestContext test(*device, pipeline);
		const TShared<render::GpuBuffer> buffer = test.CreateBuffer("Copied buffer");
		const TShared<render::Texture> texture = test.CreateTexture("Copied texture");
		const TShared<render::GpuBuffer> bufferReadback = test.CreateReadback(kSize * kSize * sizeof(u32));
		const TShared<render::GpuBuffer> textureReadback = test.CreateReadback(test.TextureReadbackSize);

		{
			FrameGraph graph(*test.Context, kGraphName, gTestGraphFlags);
			graph.ImportBuffer(buffer);
			graph.ImportTexture(texture);
			test.AddDispatch(graph, "Graphics write", buffer, texture);
			test.AddReadback(graph, "Async copy", buffer, texture, bufferReadback, textureReadback, FrameGraphPassFlag::AsyncCompute);
			graph.Compile();

			// A prologue releases the readbacks to the compute queue, and an epilogue returns them to rest after the copy
			const FrameGraphStatistics& statistics = graph.GetStatistics();
			B3D_TEST_ASSERT(statistics.SubmissionCount == 4)
			B3D_TEST_ASSERT(statistics.QueueTransferCount >= 4)

			graph.Execute();
		}

		device->WaitUntilIdle();
		B3D_TEST_ASSERT_MSG(test.HasDispatchValues(bufferReadback, false), "The async copy does not read what the graphics dispatch wrote into the buffer.")
		B3D_TEST_ASSERT_MSG(test.HasDispatchValues(textureReadback, true), "The async copy does not read what the graphics dispatch wrote into the texture.")

		device->EndFrame();
	});
}

void FrameGraphTestSuite::TestParallelRecording()
{
	gTestGraphFlags = FrameGraphFlag::RecordInParallel;

	TestProfiling();
	TestAsyncComputeProducer();
	TestAsyncComputeConsumer();
	TestSplitAcrossSubmissions();
	TestAliasAcquireAcrossSubmissions();
	TestSharedReaders();
	TestOwnershipTransfer();
	TestIndependentCompute();
	TestTransientAliasingAcrossQueues();
	TestAsyncComputeExtract();
	TestAsyncComputeExport();
	TestAsyncCopy();

	gTestGraphFlags = FrameGraphFlag::None;
}

void FrameGraphTestSuite::TestParallelRecordingStress()
{
	const TShared<GpuDevice> device = GetTestDevice();
	const bool hasComputeQueue = device != nullptr && HasComputeQueue(*device);
	const TShared<GpuComputePipelineState> pipeline = hasComputeQueue ? CreateTestDispatchPipeline(*device) : nullptr;
	const TShared<GpuComputePipelineState> samplePipeline = hasComputeQueue ? CreateTestSamplePipeline(*device) : nullptr;
	if(pipeline == nullptr || samplePipeline == nullptr)
		return;

	LoggingScope logs(*this);
	logs.IgnoreWarning(kBarrierOnlyWarning);

	RunOnRenderThread("FrameGraphTestSuite::TestParallelRecordingStress", [this, &device, &pipeline, &samplePipeline]()
	{
		constexpr u32 kFrameCount = 4;
		constexpr u32 kSamplePassCount = 9;

		const FrameGraphTestContext test(*device, pipeline, samplePipeline);
		const TShared<render::GpuBuffer> buffer = test.CreateBuffer("Written buffer");
		const TShared<render::Texture> texture = test.CreateTexture("Written texture", TextureUsageFlag::RenderTarget | TextureUsageFlag::AllowUnorderedAccessOnTheGPU);
		const TShared<render::GpuBuffer> values = test.CreateBuffer("Sampled values");

		const TShared<render::GpuParameterSet> sampleParameters = test.Context->GetParameterSetPool().Create(samplePipeline->GetParameterLayout()->GetSet(0), 0);
		sampleParameters->SetSampledTexture("InputTexture", texture);
		sampleParameters->SetStorageBuffer("OutputBuffer", values);

		Vector<TShared<render::GpuBuffer>> readbacks;
		for(u32 frameIndex = 0; frameIndex < kFrameCount; frameIndex++)
		{
			FrameGraph graph(*test.Context, kGraphName, FrameGraphFlag::RecordInParallel);
			graph.ImportBuffer(buffer);
			graph.ImportTexture(texture);
			graph.ImportBuffer(values);
			test.AddDispatch(graph, "Write", buffer, texture);

			// The sample passes read and write the values, so each waits for the one before it, on the other queue
			for(u32 passIndex = 0; passIndex < kSamplePassCount; passIndex++)
			{
				FrameGraphComputePass pass(passIndex % 2 == 1 ? FrameGraphPassFlag::AsyncCompute : FrameGraphPassFlag::None);
				pass.AddParameterSet(sampleParameters);

				graph.AddComputePass("Sample " + ToString(passIndex), std::move(pass), [&samplePipeline, sampleParameters](FrameGraphPassContext& passContext)
				{
					render::GpuCommandBuffer& commands = passContext.GetCommandBuffer();
					commands.SetGpuComputePipelineState(samplePipeline);
					commands.SetGpuParameterSet(sampleParameters);
					commands.DispatchCompute(1, 1, 1);
				});
			}

			const TShared<render::GpuBuffer> readback = test.CreateReadback(kSize * kSize * sizeof(u32));
			test.AddReadback(graph, "Read values", values, readback);
			readbacks.push_back(readback);
			graph.Compile();

			// The write shares a submission with the first sample pass, and the readback with the last
			B3D_TEST_ASSERT(graph.GetStatistics().CulledPassCount == 0)
			B3D_TEST_ASSERT(graph.GetStatistics().SubmissionCount == kSamplePassCount)
			graph.Execute();
		}

		device->WaitUntilIdle();
		for(const TShared<render::GpuBuffer>& readback : readbacks)
			B3D_TEST_ASSERT_MSG(test.HasDispatchValues(readback, false), "A pass recorded in parallel does not sample what the dispatch wrote.")

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
			FrameGraph graph(*test.Context, kGraphName, gTestGraphFlags);
			test.AddDispatch(graph, "Resting write", test.CreateBuffer("Resting buffer"), test.CreateTexture("Resting texture"));
			graph.Compile();
			graph.Execute();
		}

		// No barrier can separate a copy's source and destination on the same subresource
		{
			FrameGraph graph(*test.Context, kGraphName, gTestGraphFlags);
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
			FrameGraph graph(*test.Context, kGraphName, gTestGraphFlags);
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
			FrameGraph graph(*test.Context, kGraphName, gTestGraphFlags);
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
			FrameGraph graph(*test.Context, kGraphName, gTestGraphFlags);
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
			FrameGraph graph(*test.Context, kGraphName, gTestGraphFlags);
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
				FrameGraph graph(*test.Context, kGraphName, gTestGraphFlags);
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
				FrameGraph graph(*test.Context, kGraphName, gTestGraphFlags);
				held = graph.CreateTexture("Held texture", test.TextureInformation);
				test.AddDispatch(graph, "Culled", graph.CreateBuffer("Culled buffer", GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32), kSize * kSize,
					GpuBufferFlag::StoreOnGPU | GpuBufferFlag::AllowUnorderedAccessOnTheGPU)), held);
				graph.Compile();
				graph.Execute();
			}

			{
				FrameGraph graph(*test.Context, kGraphName, gTestGraphFlags);
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

void FrameGraphTestSuite::TestAsyncComputeResting()
{
	const TShared<GpuDevice> device = GetTestDevice();
	const TShared<GpuComputePipelineState> pipeline = device != nullptr ? CreateTestDispatchPipeline(*device) : nullptr;
	const TShared<GpuComputePipelineState> samplePipeline = device != nullptr ? CreateTestSamplePipeline(*device) : nullptr;
	if(pipeline == nullptr || samplePipeline == nullptr)
		return;

	LoggingScope logs(*this);
	logs.ExpectError("Pass 'Async resting read' reads 'Resting texture' through slot 'InputTexture' on the async compute queue, but 'Resting texture' rests on the graphics queue. Import it first.");

	RunOnRenderThread("FrameGraphTestSuite::TestAsyncComputeResting", [&device, &pipeline, &samplePipeline]()
	{
		const FrameGraphTestContext test(*device, pipeline, samplePipeline);

		// The check uses the pass's preference, so it reports the same with the async compute queue disabled. The graph is never
		// compiled, so nothing reads the texture.
		FrameGraph graph(*test.Context, kGraphName, gTestGraphFlags);
		const TShared<render::GpuBuffer> values = test.CreateBuffer("Values");
		graph.ImportBuffer(values);
		test.AddSample(graph, "Async resting read", test.CreateTexture("Resting texture"), values, FrameGraphPassFlag::AsyncCompute);
		test.AddSample(graph, "Shared resting read", test.CreateTexture("Shared texture", TextureUsageFlag::Default), values, FrameGraphPassFlag::AsyncCompute);
	});
}

void FrameGraphTestSuite::TestInterleavedGraphs()
{
	const TShared<GpuDevice> device = GetTestDevice();
	const TShared<GpuComputePipelineState> pipeline = device != nullptr ? CreateTestDispatchPipeline(*device) : nullptr;
	if(pipeline == nullptr)
		return;

	LoggingScope logs(*this);
	logs.IgnoreWarning(kBarrierOnlyWarning);
	logs.ExpectError("Frame graph 'Inner frame graph' executes while another frame graph executes. Execute frame graphs one at a time, so their submissions do not interleave.");

	RunOnRenderThread("FrameGraphTestSuite::TestInterleavedGraphs", [&device, &pipeline]()
	{
		const FrameGraphTestContext test(*device, pipeline);
		const TShared<render::GpuBuffer> buffer = test.CreateBuffer("Buffer");
		const TShared<render::GpuBuffer> readback = test.CreateReadback(kSize * kSize * sizeof(u32));
		const TShared<GpuWorkContext> innerContext = GpuWorkContext::Create(*device);

		{
			FrameGraph graph(*test.Context, kGraphName, gTestGraphFlags);
			graph.ImportBuffer(buffer);
			graph.ImportBuffer(readback);

			FrameGraphCopyPass pass;
			pass.AddSource(buffer);
			pass.AddDestination(readback);
			graph.AddCopyPass("Copy", std::move(pass), [buffer, readback, &innerContext](FrameGraphPassContext& passContext)
			{
				passContext.GetCommandBuffer().CopyBufferToBuffer(buffer, readback, 0, 0, kSize * kSize * sizeof(u32));

				FrameGraph innerGraph(*innerContext, "Inner frame graph");
				innerGraph.Compile();
				innerGraph.Execute();
			});

			graph.Compile();
			graph.Execute();
		}

		// The outer graph's execution ended, so a later graph executes without a report
		{
			FrameGraph graph(*test.Context, kGraphName, gTestGraphFlags);
			graph.Compile();
			graph.Execute();
		}

		device->WaitUntilIdle();
		device->EndFrame();
	});
}

void FrameGraphTestSuite::TestAsyncRasterPass()
{
	const TShared<GpuDevice> device = GetTestDevice();
	if(device == nullptr)
		return;

	LoggingScope logs(*this);
	logs.IgnoreWarning(kBarrierOnlyWarning);
	logs.ExpectError("Raster pass 'Async clear' is created with FrameGraphPassFlag::AsyncCompute, which raster passes do not support. It runs on the graphics queue.");

	RunOnRenderThread("FrameGraphTestSuite::TestAsyncRasterPass", [this, &device]()
	{
		const TShared<GpuWorkContext> context = GpuWorkContext::Create(*device);

		TextureCreateInformation colorInformation;
		colorInformation.Width = kSize;
		colorInformation.Height = kSize;
		colorInformation.Format = PF_RGBA8;
		colorInformation.Usage = TextureUsageFlag::RenderTarget;
		colorInformation.ClearColor = Color(1.0f, 0.0f, 0.0f, 1.0f);

		TAsyncOp<TShared<PixelData>> colorRead;
		{
			FrameGraph graph(*context, kGraphName);
			const TShared<render::Texture> color = graph.CreateTexture("Color", colorInformation);

			FrameGraphRasterPass pass(FrameGraphPassFlag::AsyncCompute);
			pass.SetColorAttachment(0, color, FrameGraphAttachmentLoad::Clear);
			graph.AddRasterPass("Async clear", std::move(pass), nullptr);
			colorRead = TextureUtility::ReadAsync(graph, color);

			graph.Compile();
			B3D_TEST_ASSERT(graph.GetStatistics().SubmissionCount == 1)
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
				isCleared &= rowTexels[column] == 0xFF0000FF;
		}

		B3D_TEST_ASSERT_MSG(isCleared, "The raster pass does not clear its attachment on the graphics queue.")

		device->EndFrame();
	});
}
#endif

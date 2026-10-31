//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DGpuBindlessTestSuite.h"
#include "Utility/B3DGpuBarrierTestUtility.h"
#include "Utility/B3DUnitTestGpuPipelines.h"
#include "CoreObject/B3DRenderThread.h"
#include "GpuBackend/B3DGpuBarriers.h"
#include "GpuBackend/Allocators/B3DGpuResource.h"
#include "GpuBackend/B3DGpuBackend.h"
#include "GpuBackend/B3DGpuBackendUtility.h"
#include "GpuBackend/B3DGpuBindlessHeap.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuCompletionTracker.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuDeviceCapabilities.h"
#include "GpuBackend/B3DGpuParameterSet.h"
#include "GpuBackend/B3DGpuParameterSetPool.h"
#include "GpuBackend/B3DGpuPipelineParameterLayout.h"
#include "GpuBackend/B3DGpuPipelineState.h"
#include "GpuBackend/B3DGpuProgram.h"
#include "GpuBackend/B3DGpuWorkContext.h"
#include "GpuBackend/B3DSamplerState.h"
#include "Image/B3DTexture.h"

using namespace b3d;
using namespace b3d::render;

namespace
{
	/** Native heap stub that records the slots written into it. */
	class RecordingBindlessHeap : public IGpuBindlessHeap
	{
	public:
		RecordingBindlessHeap() = default;

		void WriteResourceSlot(u32 slotIndex, const GpuBindlessResourceView& view) override { ResourceWrites.Add(slotIndex); }
		void WriteSamplerSlot(u32 slotIndex, const SamplerState& sampler) override { SamplerWrites.Add(slotIndex); }
		void ClearSlot(GpuBindlessHeapType heap, u32 slotIndex) override { ClearedSlots.Add(slotIndex); }

		TArray<u32> ResourceWrites;
		TArray<u32> SamplerWrites;
		TArray<u32> ClearedSlots;
	};

	/** Completion tracker whose GPU progress is set by the test. */
	class ManualCompletionTracker : public IGpuCompletionTracker
	{
	public:
		ManualCompletionTracker() = default;

		u64 GetCurrentMarker() const override { return CurrentMarker; }
		bool IsMarkerComplete(u64 marker) const override { return marker <= CompletedMarker; }

		u64 CurrentMarker = 1;
		u64 CompletedMarker = 0;
	};

	class PinnedTestBuffer : public IGpuBufferResource
	{
	public:
		PinnedTestBuffer() = default;
	};
}

GpuBindlessTestSuite::GpuBindlessTestSuite()
	: TestSuite("GpuBindlessTestSuite")
{
	B3D_ADD_TEST(GpuBindlessTestSuite::TestHeapAllocation)
	B3D_ADD_TEST(GpuBindlessTestSuite::TestResourceIndices)
	B3D_ADD_TEST(GpuBindlessTestSuite::TestHeapAccess)
}

void GpuBindlessTestSuite::TestHeapAllocation()
{
	GpuBackend& backend = GpuBackend::Instance();
	if(backend.GetDeviceCount() == 0)
		return;

	const TShared<GpuDevice> device = backend.GetDevice(0);
	const TShared<render::GpuBuffer> buffer = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32), 4));
	B3D_TEST_ASSERT(buffer != nullptr)
	if(buffer == nullptr)
		return;

	RecordingBindlessHeap nativeHeap;
	ManualCompletionTracker completionTracker;
	GpuBindlessHeaps heaps(*device, nativeHeap, completionTracker, 4, 16);

	const GpuBindlessResourceView view(*buffer, BF_UNKNOWN);
	const int owners[4] = {};

	// Slot 0 is the invalid index, so a heap of four slots holds three views
	u32 slots[3];
	for(u32 slotIndex = 0; slotIndex < 3; slotIndex++)
	{
		slots[slotIndex] = heaps.AllocateResourceSlot(view, &owners[slotIndex]);
		B3D_TEST_ASSERT(slots[slotIndex] == slotIndex + 1)
	}

	B3D_TEST_ASSERT(nativeHeap.ResourceWrites.Size() == 3)

	{
		LoggingScope logs(*this);
		logs.ExpectError("The bindless resource heap is full (4 slots).");
		B3D_TEST_ASSERT(heaps.AllocateResourceSlot(view, &owners[3]) == kGpuBindlessInvalidIndex)
	}

	// A released slot is reused once the GPU completes the frame of the release, and not before
	completionTracker.CurrentMarker = 5;
	heaps.ReleaseSlot(GpuBindlessHeapType::Resource, slots[1], &owners[1]);

	GpuBindlessHeapStatistics statistics = heaps.GetStatistics(GpuBindlessHeapType::Resource);
	B3D_TEST_ASSERT(statistics.Capacity == 4)
	B3D_TEST_ASSERT(statistics.AllocatedCount == 2)
	B3D_TEST_ASSERT(statistics.HighWaterMark == 3)
	B3D_TEST_ASSERT(statistics.PendingReleaseCount == 1)

	completionTracker.CompletedMarker = 4;
	heaps.ProcessReleases();
	{
		LoggingScope logs(*this);
		logs.ExpectError("The bindless resource heap is full (4 slots).");
		B3D_TEST_ASSERT(heaps.AllocateResourceSlot(view, &owners[3]) == kGpuBindlessInvalidIndex)
	}

	completionTracker.CompletedMarker = 5;
	heaps.ProcessReleases();
	B3D_TEST_ASSERT(heaps.GetStatistics(GpuBindlessHeapType::Resource).PendingReleaseCount == 0)

#if B3D_BUILD_TYPE_DEVELOPMENT
	B3D_TEST_ASSERT(nativeHeap.ClearedSlots.Size() == 1 && nativeHeap.ClearedSlots[0] == slots[1])
#endif

	const u32 reusedSlot = heaps.AllocateResourceSlot(view, &owners[3]);
	B3D_TEST_ASSERT(reusedSlot == slots[1])

#if B3D_BUILD_TYPE_DEVELOPMENT
	{
		LoggingScope logs(*this);
		logs.ExpectError("released by an object other than its owner.");
		heaps.ReleaseSlot(GpuBindlessHeapType::Resource, slots[0], &owners[2]);
	}
#endif

	heaps.ReleaseSlot(GpuBindlessHeapType::Resource, slots[0], &owners[0]);

#if B3D_BUILD_TYPE_DEVELOPMENT
	{
		LoggingScope logs(*this);
		logs.ExpectError("released twice, or never allocated.");
		heaps.ReleaseSlot(GpuBindlessHeapType::Resource, slots[0], &owners[0]);
	}
#endif

	// A pin keeps the resource bound until the frame of its release completes
	PinnedTestBuffer pinnedResource;
	heaps.PinResource(pinnedResource);
	B3D_TEST_ASSERT(pinnedResource.GetBoundCount() == 1)

	completionTracker.CurrentMarker = 6;
	heaps.UnpinResource(pinnedResource);
	heaps.ProcessReleases();
	B3D_TEST_ASSERT(pinnedResource.GetBoundCount() == 1)

	completionTracker.CompletedMarker = 6;
	heaps.ProcessReleases();
	B3D_TEST_ASSERT(pinnedResource.GetBoundCount() == 0)

	// The sampler heap has its own index space
	const TShared<SamplerState> sampler = device->FindOrCreateSamplerState(SamplerStateCreateInformation());
	const u32 samplerSlot = heaps.AllocateSamplerSlot(*sampler, &owners[0]);
	B3D_TEST_ASSERT(samplerSlot == 1)
	B3D_TEST_ASSERT(nativeHeap.SamplerWrites.Size() == 1 && nativeHeap.SamplerWrites[0] == samplerSlot)

	heaps.ReleaseSlot(GpuBindlessHeapType::Sampler, samplerSlot, &owners[0]);
	heaps.ReleaseSlot(GpuBindlessHeapType::Resource, slots[2], &owners[2]);
	heaps.ReleaseSlot(GpuBindlessHeapType::Resource, reusedSlot, &owners[3]);
	heaps.ProcessAllReleases();

	B3D_TEST_ASSERT(heaps.GetStatistics(GpuBindlessHeapType::Resource).AllocatedCount == 0)
	B3D_TEST_ASSERT(heaps.GetStatistics(GpuBindlessHeapType::Sampler).AllocatedCount == 0)
	B3D_TEST_ASSERT(heaps.GetStatistics(GpuBindlessHeapType::Resource).PendingReleaseCount == 0)
}

void GpuBindlessTestSuite::TestResourceIndices()
{
	GpuBackend& backend = GpuBackend::Instance();
	if(backend.GetDeviceCount() == 0)
		return;

	const TShared<GpuDevice> device = backend.GetDevice(0);
	const GpuBufferFlags bindlessFlags = GpuBufferFlag::StoreOnGPU | GpuBufferFlag::Bindless;

	if(device->GetBindlessHeaps() == nullptr)
	{
		B3D_TEST_ASSERT(!device->GetCapabilities().HasCapability(RSC_BINDLESS_RESOURCES))

		LoggingScope logs(*this);
		logs.ExpectError("the GPU device doesn't support bindless resources.");
		B3D_TEST_ASSERT(device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32), 4, bindlessFlags)) == nullptr)
		return;
	}

	B3D_TEST_ASSERT(device->GetCapabilities().HasCapability(RSC_BINDLESS_RESOURCES))

	{
		LoggingScope logs(*this);
		logs.ExpectError("A bindless buffer must be a simple or structured storage buffer.");
		B3D_TEST_ASSERT(device->CreateGpuBuffer(GpuBufferCreateInformation::CreateUniform(16, GpuBufferFlag::StoreOnCPUWithGPUAccess | GpuBufferFlag::Bindless)) == nullptr)
	}

	GetRenderThread().PostCommand([this, &device, bindlessFlags]()
	{
		GpuBindlessHeaps& heaps = *device->GetBindlessHeaps();
		const u32 initialAllocatedCount = heaps.GetStatistics(GpuBindlessHeapType::Resource).AllocatedCount;

		TextureCreateInformation textureInformation;
		textureInformation.Name = "Bindless index texture";
		textureInformation.Width = 4;
		textureInformation.Height = 4;
		textureInformation.Format = PF_RGBA8;
		textureInformation.Usage = TextureUsageFlag::Default | TextureUsageFlag::Bindless;

		TShared<render::Texture> texture = device->CreateTexture(textureInformation);
		TShared<render::GpuBuffer> structuredBuffer = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32), 16, bindlessFlags));
		TShared<render::GpuBuffer> typedBuffer = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateSimpleStorage(BF_32X1U, 16, bindlessFlags));
		B3D_TEST_ASSERT(texture != nullptr && structuredBuffer != nullptr && typedBuffer != nullptr)
		if(texture == nullptr || structuredBuffer == nullptr || typedBuffer == nullptr)
			return;

		// Every resource gets the slot of its default view on creation, and other views get their own slots on request
		const u32 textureIndex = texture->GetBindlessIndex();
		const u32 mipIndex = texture->GetOrCreateBindlessIndex(TextureSurface(0, 1, 0, 1));
		B3D_TEST_ASSERT(textureIndex != kGpuBindlessInvalidIndex)
		B3D_TEST_ASSERT(mipIndex != kGpuBindlessInvalidIndex && mipIndex != textureIndex)
		B3D_TEST_ASSERT(texture->GetOrCreateBindlessIndex(TextureSurface(0, 1, 0, 1)) == mipIndex)
		B3D_TEST_ASSERT(texture->GetOrCreateBindlessIndex(TextureSurface::kComplete) == textureIndex)

		B3D_TEST_ASSERT(structuredBuffer->GetBindlessIndex() != kGpuBindlessInvalidIndex)

		const u32 typedIndex = typedBuffer->GetBindlessIndex();
		const u32 reinterpretedIndex = typedBuffer->GetOrCreateBindlessIndex(BF_16X2U);
		B3D_TEST_ASSERT(typedIndex != kGpuBindlessInvalidIndex)
		B3D_TEST_ASSERT(reinterpretedIndex != kGpuBindlessInvalidIndex && reinterpretedIndex != typedIndex)
		B3D_TEST_ASSERT(typedBuffer->GetOrCreateBindlessIndex(BF_32X1U) == typedIndex)
		B3D_TEST_ASSERT(typedBuffer->GetOrCreateBindlessIndex(BF_UNKNOWN) == typedIndex)

		GpuBindlessHeapStatistics statistics = heaps.GetStatistics(GpuBindlessHeapType::Resource);
		B3D_TEST_ASSERT(statistics.AllocatedCount == initialAllocatedCount + 5)

		// Destruction releases every slot of the resource
		const u32 pendingReleaseCount = statistics.PendingReleaseCount;
		texture = nullptr;
		structuredBuffer = nullptr;
		typedBuffer = nullptr;

		statistics = heaps.GetStatistics(GpuBindlessHeapType::Resource);
		B3D_TEST_ASSERT(statistics.AllocatedCount == initialAllocatedCount)
		B3D_TEST_ASSERT(statistics.PendingReleaseCount >= pendingReleaseCount + 5)

		// Global samplers have permanent slots, and cached samplers get theirs on first request
		SamplerStateCreateInformation pointClampInformation;
		pointClampInformation.MinFilter = FO_POINT;
		pointClampInformation.MagFilter = FO_POINT;
		pointClampInformation.MipFilter = FO_POINT;
		pointClampInformation.AddressMode.U = TAM_CLAMP;
		pointClampInformation.AddressMode.V = TAM_CLAMP;
		pointClampInformation.AddressMode.W = TAM_CLAMP;

		const TShared<SamplerState> globalSampler = device->FindOrCreateSamplerState(pointClampInformation);
		B3D_TEST_ASSERT(globalSampler->GetBindlessIndex() == GetGlobalSamplerIndex(GpuGlobalSamplerFilter::Point, GpuGlobalSamplerAddress::Clamp))

		SamplerStateCreateInformation biasedInformation = pointClampInformation;
		biasedInformation.MipmapBias = 0.25f;

		const TShared<SamplerState> biasedSampler = device->FindOrCreateSamplerState(biasedInformation);
		const u32 biasedIndex = biasedSampler->GetBindlessIndex();
		B3D_TEST_ASSERT(biasedIndex > kGpuGlobalSamplerCount)
		B3D_TEST_ASSERT(biasedSampler->GetBindlessIndex() == biasedIndex)
		B3D_TEST_ASSERT(device->FindOrCreateSamplerState(biasedInformation)->GetBindlessIndex() == biasedIndex)
	}, "GpuBindlessTestSuite::TestResourceIndices", true);
}

void GpuBindlessTestSuite::TestHeapAccess()
{
	GpuBackend& backend = GpuBackend::Instance();
	if(backend.GetDeviceCount() == 0)
		return;

	const TShared<GpuDevice> device = backend.GetDevice(0);
	if(device->GetBindlessHeaps() == nullptr)
		return;

	// Shaders are resources, which are created outside of the render thread
	const TShared<GpuComputePipelineState> pipeline = CreateTestBindlessHeapAccessPipeline(*device);
	B3D_TEST_ASSERT(pipeline != nullptr)
	if(pipeline == nullptr)
		return;

	B3D_TEST_ASSERT(pipeline->GetParameterLayout()->GetProgramFeatures() == kGpuProgramBindlessFeatures)

	GetRenderThread().PostCommand([this, &device, &pipeline]()
	{
		const GpuBufferFlags inputFlags = GpuBufferFlag::StoreOnCPUWithGPUAccess | GpuBufferFlag::Bindless;
		const TShared<render::GpuBuffer> structuredBuffer = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32), 4, inputFlags));
		const TShared<render::GpuBuffer> typedBuffer = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateSimpleStorage(BF_32X1U, 4, inputFlags));
		B3D_TEST_ASSERT(structuredBuffer != nullptr && typedBuffer != nullptr)
		if(structuredBuffer == nullptr || typedBuffer == nullptr)
			return;

		// The host writes are made visible by the submission
		for(u32 bufferIndex = 0; bufferIndex < 2; bufferIndex++)
		{
			const TShared<render::GpuBuffer>& buffer = bufferIndex == 0 ? structuredBuffer : typedBuffer;
			const render::GpuBufferMappedScope mapping = buffer->Map(GpuMapOption::Write);
			B3D_TEST_ASSERT(mapping.IsValid())
			if(!mapping.IsValid())
				return;

			u32* elements = static_cast<u32*>(mapping.GetMappedMemory());
			for(u32 elementIndex = 0; elementIndex < 4; elementIndex++)
				elements[elementIndex] = (bufferIndex + 1) * 100 + elementIndex;
		}

		TextureCreateInformation textureInformation;
		textureInformation.Name = "Bindless access texture";
		textureInformation.Width = 2;
		textureInformation.Height = 1;
		textureInformation.Format = PF_RGBA8;
		textureInformation.Usage = TextureUsageFlag::Default | TextureUsageFlag::Bindless;

		const TShared<render::Texture> texture = device->CreateTexture(textureInformation);
		B3D_TEST_ASSERT(texture != nullptr)
		if(texture == nullptr)
			return;

		const ImageSubresourcePitch pitch = texture->GetStagingBufferPitchForSubresource(0, 0);
		const u32 stagingTexelCount = pitch.RowPitch * pitch.SliceHeight;
		const TShared<render::GpuBuffer> upload = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStagingWrite(stagingTexelCount * sizeof(u32)));
		{
			const render::GpuBufferMappedScope mapping = upload->Map(GpuMapOption::Write);
			B3D_TEST_ASSERT(mapping.IsValid())
			if(!mapping.IsValid())
				return;

			// Texel 0 is (10, 20, 30, 40) and texel 1 is (50, 60, 70, 80)
			u32* texels = static_cast<u32*>(mapping.GetMappedMemory());
			texels[0] = 10 | (20 << 8) | (30 << 16) | (40u << 24);
			texels[1] = 50 | (60 << 8) | (70 << 16) | (80u << 24);
		}

		SamplerStateCreateInformation samplerInformation;
		samplerInformation.MinFilter = FO_POINT;
		samplerInformation.MagFilter = FO_POINT;
		samplerInformation.MipFilter = FO_POINT;
		samplerInformation.MipmapBias = 0.25f;
		const TShared<SamplerState> cachedSampler = device->FindOrCreateSamplerState(samplerInformation);

		const u32 indices[] = { structuredBuffer->GetBindlessIndex(), typedBuffer->GetBindlessIndex(), texture->GetBindlessIndex() | (cachedSampler->GetBindlessIndex() << 16) };

		static constexpr u32 kOutputCount = 5;
		const TShared<GpuWorkContext> context = GpuWorkContext::Create(*device);
		const TShared<render::GpuBuffer> output = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32), kOutputCount, GpuBufferFlag::StoreOnGPU | GpuBufferFlag::AllowUnorderedAccessOnTheGPU));
		const TShared<render::GpuBuffer> readback = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStagingRead(kOutputCount * sizeof(u32)));
		const TShared<render::GpuParameterSet> parameterSet = context->GetParameterSetPool().Create(pipeline->GetParameterLayout()->GetSet(0), 0);
		parameterSet->SetStorageBuffer("Output", output);

		const GpuAccessState computeWrite(GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Write);
		const GpuAccessState transferWrite(GpuStageFlag::Transfer, GpuAccessFlag::Write);
		const GpuAccessState textureTransferWrite(GpuStageFlag::Transfer, GpuAccessFlag::Write, GpuImageLayout::TransferDestination);
		const GpuAccessState transferRead(GpuStageFlag::Transfer, GpuAccessFlag::Read);
		const GpuAccessState hostRead(GpuStageFlag::Host, GpuAccessFlag::Read);

		const TShared<GpuCommandBufferPool> pool = GpuBarrierTestUtility::CreateCommandBufferPool(*device);
		const TShared<GpuCommandBuffer> commands = pool->Create(GpuCommandBufferCreateInformation::Create("Bindless heap access"));

		// The texture returns to rest after its upload, which is where heaps read it without barriers
		commands->IssueBarriers(GpuTextureBarrier(texture, GpuBarrierTestUtility::GetDiscardedRestingState(*texture), textureTransferWrite));
		commands->CopyBufferToTexture(upload, texture, 0, 0, 0);
		commands->IssueBarriers(GpuTextureBarrier(texture, textureTransferWrite, GpuBackendUtility::GetRestingState(*texture, GQT_GRAPHICS)));

		commands->IssueBarriers(GpuBufferBarrier(output, GpuAccessState(), computeWrite));
		commands->SetGpuComputePipelineState(pipeline);
		commands->SetGpuParameterSet(parameterSet);
		commands->SetPushConstants(0, sizeof(indices), indices);
		commands->DispatchCompute(1);

		const GpuBufferBarrier copyBarriers[] = { GpuBufferBarrier(output, computeWrite, transferRead), GpuBufferBarrier(readback, GpuAccessState(), transferWrite) };
		commands->IssueBarriers(GpuBarriers(TArrayView<const GpuBufferBarrier>(copyBarriers, 2)));
		commands->CopyBufferToBuffer(output, readback, 0, 0, kOutputCount * sizeof(u32));
		commands->IssueBarriers(GpuBufferBarrier(readback, transferWrite, hostRead));
		context->SubmitCommandBuffer(commands);
		device->WaitUntilIdle();

		const render::GpuBufferMappedScope mapping = readback->Map(GpuMapOption::Read);
		B3D_TEST_ASSERT(mapping.IsValid())
		if(!mapping.IsValid())
			return;

		const u32 expected[kOutputCount] = { 101, 202, 50, 60, 30 };
		const char* names[kOutputCount] = { "structured buffer", "typed buffer", "texture load", "cached sampler", "global sampler" };
		const u32* actual = static_cast<const u32*>(mapping.GetMappedMemory());
		for(u32 outputIndex = 0; outputIndex < kOutputCount; outputIndex++)
			B3D_TEST_ASSERT_MSG(actual[outputIndex] == expected[outputIndex], StringUtility::Format("Read through the {0}: {1}, expected {2}.", names[outputIndex], actual[outputIndex], expected[outputIndex]))
	}, "GpuBindlessTestSuite::TestHeapAccess", true);
}

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
#include "GpuBackend/B3DGpuBindlessHeapPool.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuCompletionTracker.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuDeviceCapabilities.h"
#include "GpuBackend/B3DGpuMemoryDefragmenter.h"
#include "GpuBackend/B3DGpuParameterSet.h"
#include "GpuBackend/B3DGpuParameterSetPool.h"
#include "GpuBackend/B3DGpuPipelineParameterLayout.h"
#include "GpuBackend/B3DGpuPipelineState.h"
#include "GpuBackend/B3DGpuProgram.h"
#include "GpuBackend/B3DGpuResourceManager.h"
#include "GpuBackend/B3DGpuWorkContext.h"
#include "GpuBackend/B3DSamplerState.h"
#include "Image/B3DTexture.h"

using namespace b3d;
using namespace b3d::render;

namespace
{
	/** Resource manager of native heap stubs. */
	class TestResourceManager : public GpuResourceManager
	{
	public:
		using GpuResourceManager::GpuResourceManager;

		/** Creates a resource of type @p Type. Destroy it with IGpuResource::Destroy(). */
		template<class Type, class... Args>
		Type* Create(Args&&... args)
		{
			Type* resource = new(B3DAllocate(sizeof(Type))) Type(this, std::forward<Args>(args)...);
			RegisterResource(resource);
			return resource;
		}
	};

	/** Native heap stub that counts its destruction. */
	class CountingBindlessHeap : public GpuBindlessHeap
	{
	public:
		CountingBindlessHeap(GpuResourceManager* owner, u32 capacity, u32& destroyedCount)
			: GpuBindlessHeap(owner, capacity), mDestroyedCount(destroyedCount)
		{ }

		~CountingBindlessHeap() override { mDestroyedCount++; }

	private:
		u32& mDestroyedCount;
	};

	/** Bindless heaps over native heap stubs, which record the slots written into them. */
	class RecordingBindlessHeaps : public GpuBindlessHeaps
	{
	public:
		RecordingBindlessHeaps(GpuDevice& device, const IGpuCompletionTracker& completionTracker, const GpuBindlessHeapsCreateInformation& createInformation)
			: GpuBindlessHeaps(device, completionTracker, createInformation), mResourceManager(device)
		{
			const bool isInitialized = Initialize();
			B3D_ASSERT(isInitialized);
			(void)isInitialized;
		}

		~RecordingBindlessHeaps() override
		{
			Destroy();
		}

		/** Returns the information for heaps of the provided capacities that apply updates through PatchResourceSlots(). */
		static GpuBindlessHeapsCreateInformation GetCreateInformation(u32 resourceCapacity, u32 resourceMaximumCapacity, u32 samplerCapacity)
		{
			GpuBindlessHeapsCreateInformation createInformation;
			createInformation.ResourceCapacity = resourceCapacity;
			createInformation.ResourceMaximumCapacity = resourceMaximumCapacity;
			createInformation.SamplerCapacity = samplerCapacity;
			createInformation.UpdateMethod = GpuBindlessUpdateMethod::Patch;
			return createInformation;
		}

		TArray<u32> HeapCapacities; /**< Capacity of every native resource heap created, in creation order. */
		TArray<u32> ResourceWrites;
		TArray<u32> UpdatedSlots;
		TArray<u32> SamplerWrites;
		TArray<u32> ClearedSlots;
		TArray<GpuParameterObjectType> ClearedViewTypes;

	protected:
		GpuBindlessHeap* CreateResourceHeap(u32 capacity) override
		{
			HeapCapacities.Add(capacity);
			return mResourceManager.Create<GpuBindlessHeap>(capacity);
		}

		void CopyResourceSlots(GpuBindlessHeap& /*destination*/, TArrayView<const TIndexRange<u32>> /*ranges*/) override { }
		void WriteResourceSlots(GpuBindlessHeap& /*destination*/, TArrayView<const GpuBindlessSlotUpdate> /*updates*/) override { }
		void WriteResourceSlot(u32 slotIndex, const GpuBindlessResourceView& /*view*/) override { ResourceWrites.Add(slotIndex); }
		void WriteSamplerSlot(u32 slotIndex, const SamplerState& /*sampler*/) override { SamplerWrites.Add(slotIndex); }

		void ClearResourceSlot(u32 slotIndex, GpuParameterObjectType viewType) override
		{
			ClearedSlots.Add(slotIndex);
			ClearedViewTypes.Add(viewType);
		}

		bool PatchResourceSlots(TArrayView<const GpuBindlessSlotUpdate> updates) override
		{
			for(const GpuBindlessSlotUpdate& update : updates)
				UpdatedSlots.Add(update.SlotIndex);

			return true;
		}

	private:
		TestResourceManager mResourceManager;
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
	B3D_ADD_TEST(GpuBindlessTestSuite::TestHeapChanges)
	B3D_ADD_TEST(GpuBindlessTestSuite::TestHeapPool)
	B3D_ADD_TEST(GpuBindlessTestSuite::TestResourceIndices)
	B3D_ADD_TEST(GpuBindlessTestSuite::TestHeapAccess)
	B3D_ADD_TEST(GpuBindlessTestSuite::TestRelocation)
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

	ManualCompletionTracker completionTracker;
	RecordingBindlessHeaps heaps(*device, completionTracker, RecordingBindlessHeaps::GetCreateInformation(4, 4, 16));

	const GpuBindlessResourceView view(*buffer, GpuBufferViewType::Typed, BF_UNKNOWN);
	const int owners[4] = {};

	// Slot 0 is the invalid index, so a heap of four slots holds three views
	u32 slots[3];
	for(u32 slotIndex = 0; slotIndex < 3; slotIndex++)
	{
		slots[slotIndex] = heaps.AllocateResourceSlot(view, &owners[slotIndex]);
		B3D_TEST_ASSERT(slots[slotIndex] == slotIndex + 1)
	}

	B3D_TEST_ASSERT(heaps.ResourceWrites.Size() == 3)

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
	B3D_TEST_ASSERT(heaps.ClearedSlots.Size() == 1 && heaps.ClearedSlots[0] == slots[1])
	B3D_TEST_ASSERT(heaps.ClearedViewTypes[0] == GPOT_STRUCTURED_BUFFER)
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
	B3D_TEST_ASSERT(pinnedResource.GetPinCount() == 1)

	completionTracker.CurrentMarker = 6;
	heaps.UnpinResource(pinnedResource);
	heaps.ProcessReleases();
	B3D_TEST_ASSERT(pinnedResource.GetPinCount() == 1)

	completionTracker.CompletedMarker = 6;
	heaps.ProcessReleases();
	B3D_TEST_ASSERT(pinnedResource.GetPinCount() == 0)

	// The sampler heap has its own index space
	const TShared<SamplerState> sampler = device->FindOrCreateSamplerState(SamplerStateCreateInformation());
	const u32 samplerSlot = heaps.AllocateSamplerSlot(*sampler, &owners[0]);
	B3D_TEST_ASSERT(samplerSlot == 1)
	B3D_TEST_ASSERT(heaps.SamplerWrites.Size() == 1 && heaps.SamplerWrites[0] == samplerSlot)

	heaps.ReleaseSlot(GpuBindlessHeapType::Sampler, samplerSlot, &owners[0]);
	heaps.ReleaseSlot(GpuBindlessHeapType::Resource, slots[2], &owners[2]);
	heaps.ReleaseSlot(GpuBindlessHeapType::Resource, reusedSlot, &owners[3]);
	heaps.ProcessAllReleases();

	B3D_TEST_ASSERT(heaps.GetStatistics(GpuBindlessHeapType::Resource).AllocatedCount == 0)
	B3D_TEST_ASSERT(heaps.GetStatistics(GpuBindlessHeapType::Sampler).AllocatedCount == 0)
	B3D_TEST_ASSERT(heaps.GetStatistics(GpuBindlessHeapType::Resource).PendingReleaseCount == 0)
}

void GpuBindlessTestSuite::TestHeapChanges()
{
	GpuBackend& backend = GpuBackend::Instance();
	if(backend.GetDeviceCount() == 0)
		return;

	const TShared<GpuDevice> device = backend.GetDevice(0);
	const TShared<render::GpuBuffer> buffer = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32), 4));
	B3D_TEST_ASSERT(buffer != nullptr)
	if(buffer == nullptr)
		return;

	ManualCompletionTracker completionTracker;
	RecordingBindlessHeaps heaps(*device, completionTracker, RecordingBindlessHeaps::GetCreateInformation(8, 16, 16));
	B3D_TEST_ASSERT(heaps.HeapCapacities.Size() == 1 && heaps.HeapCapacities[0] == 8)

	const GpuBindlessResourceView view(*buffer, GpuBufferViewType::Typed, BF_UNKNOWN);
	const int owners[16] = {};

	// Seven usable slots, so growth starts once fewer than two are free
	u32 slots[16];
	for(u32 slotIndex = 0; slotIndex < 5; slotIndex++)
		slots[slotIndex] = heaps.AllocateResourceSlot(view, &owners[slotIndex]);

	heaps.ApplyPendingChanges();
	B3D_TEST_ASSERT(heaps.HeapCapacities.Size() == 1)

	slots[5] = heaps.AllocateResourceSlot(view, &owners[5]);

	heaps.ApplyPendingChanges();
	B3D_TEST_ASSERT(heaps.HeapCapacities.Size() == 2 && heaps.HeapCapacities[1] == 16)

	const GpuBindlessHeapStatistics statistics = heaps.GetStatistics(GpuBindlessHeapType::Resource);
	B3D_TEST_ASSERT(statistics.Capacity == 16 && statistics.GrowthCount == 1)

	// Slots above the old capacity are allocated right away
	for(u32 slotIndex = 6; slotIndex < 15; slotIndex++)
		slots[slotIndex] = heaps.AllocateResourceSlot(view, &owners[slotIndex]);

	B3D_TEST_ASSERT(slots[6] == 7 && slots[7] == 8 && slots[14] == 15)

	// The heap is at its maximum capacity
	heaps.ApplyPendingChanges();
	B3D_TEST_ASSERT(heaps.HeapCapacities.Size() == 2)
	{
		LoggingScope logs(*this);
		logs.ExpectError("Increase gpu.BindlessResourceMaximumCapacity.");
		B3D_TEST_ASSERT(heaps.AllocateResourceSlot(view, &owners[15]) == kGpuBindlessInvalidIndex)
	}

	// Updates apply once, at the end of the frame. A later update of the same slot replaces an earlier one, and the replaced update's
	// view is unpinned right away, while the descriptor being replaced is unpinned once the frame of the update completes.
	PinnedTestBuffer previousTarget;
	PinnedTestBuffer replacedTarget;
	heaps.PinResource(previousTarget);
	heaps.PinResource(replacedTarget);

	completionTracker.CurrentMarker = 2;
	heaps.UpdateResourceSlot(slots[0], view, &owners[0], &previousTarget);
	heaps.UpdateResourceSlot(slots[0], view, &owners[0], &replacedTarget);
	heaps.UpdateResourceSlot(slots[1], view, &owners[1], nullptr);

	// A released slot drops its pending update
	heaps.ReleaseSlot(GpuBindlessHeapType::Resource, slots[1], &owners[1]);

	completionTracker.CompletedMarker = 2;
	heaps.ProcessReleases();
	B3D_TEST_ASSERT(replacedTarget.GetPinCount() == 0)
	B3D_TEST_ASSERT(previousTarget.GetPinCount() == 1)

	completionTracker.CurrentMarker = 3;
	heaps.ApplyPendingChanges();
	B3D_TEST_ASSERT(heaps.UpdatedSlots.Size() == 1 && heaps.UpdatedSlots[0] == slots[0])
	B3D_TEST_ASSERT(heaps.GetStatistics(GpuBindlessHeapType::Resource).UpdateCount == 1)

	heaps.ApplyPendingChanges();
	B3D_TEST_ASSERT(heaps.UpdatedSlots.Size() == 1)

	heaps.ProcessReleases();
	B3D_TEST_ASSERT(previousTarget.GetPinCount() == 1)

	completionTracker.CompletedMarker = 3;
	heaps.ProcessReleases();
	B3D_TEST_ASSERT(previousTarget.GetPinCount() == 0)

	for(u32 slotIndex = 0; slotIndex < 15; slotIndex++)
	{
		if(slotIndex != 1)
			heaps.ReleaseSlot(GpuBindlessHeapType::Resource, slots[slotIndex], &owners[slotIndex]);
	}

	heaps.ProcessAllReleases();
	B3D_TEST_ASSERT(heaps.GetStatistics(GpuBindlessHeapType::Resource).AllocatedCount == 0)
}

void GpuBindlessTestSuite::TestHeapPool()
{
	GpuBackend& backend = GpuBackend::Instance();
	if(backend.GetDeviceCount() == 0)
		return;

	TestResourceManager resourceManager(*backend.GetDevice(0));
	u32 destroyedCount = 0;

	{
		GpuBindlessHeapPool pool(3);

		GpuBindlessHeap* first = resourceManager.Create<CountingBindlessHeap>(8, destroyedCount);
		pool.Initialize(first);
		B3D_TEST_ASSERT(pool.GetCurrent() == first && first->IsCurrent())
		B3D_TEST_ASSERT(pool.TakePooledHeap() == nullptr)

		// Switching retires the previous native heap, which lacks the updated slots and slots written afterwards
		GpuBindlessHeap* second = resourceManager.Create<CountingBindlessHeap>(8, destroyedCount);
		const GpuBindlessSlotUpdate updates[] = { GpuBindlessSlotUpdate(3, GpuBindlessResourceView()) };
		pool.MakeCurrent(second, TArrayView<const GpuBindlessSlotUpdate>(updates, std::size(updates)));
		pool.MarkSlotWritten(5);

		B3D_TEST_ASSERT(pool.GetCurrent() == second && second->IsCurrent() && !first->IsCurrent())
		B3D_TEST_ASSERT(second->GetVersion() != first->GetVersion())

		u32 heapCount = 0;
		pool.ForEachHeap([&heapCount](GpuBindlessHeap&) { heapCount++; });
		B3D_TEST_ASSERT(heapCount == 2)

		// A retiring native heap is only pooled once no command buffer binds it
		first->NotifyBound();
		pool.ProcessRetiredHeaps();
		B3D_TEST_ASSERT(pool.TakePooledHeap() == nullptr)

		first->NotifyUnbound();
		pool.ProcessRetiredHeaps();
		GpuBindlessHeap* pooled = pool.TakePooledHeap();
		B3D_TEST_ASSERT(pooled == first)

		u32 dirtySlotCount = 0;
		pool.GetDirtySlots(*first).ForEachSetRange(pool.GetDirtySlots(*first), 8, [&dirtySlotCount](const TIndexRange<u64>& range) { dirtySlotCount += (u32)range.Count; });
		B3D_TEST_ASSERT(dirtySlotCount == 2 && pool.GetDirtySlots(*first)[3] && pool.GetDirtySlots(*first)[5])

		// Reuse clears the dirty slots and gives a new version
		const u64 firstVersion = first->GetVersion();
		pool.MakeCurrent(first, {});
		B3D_TEST_ASSERT(first->IsCurrent() && first->GetVersion() != firstVersion && !pool.GetDirtySlots(*first)[3])

		pool.ProcessRetiredHeaps();
		B3D_TEST_ASSERT(destroyedCount == 0)

		// Growth destroys the pooled native heaps, which are too small, and smaller retiring ones once no command buffer binds them
		GpuBindlessHeap* grown = resourceManager.Create<CountingBindlessHeap>(16, destroyedCount);
		pool.MakeCurrent(grown, {});
		B3D_TEST_ASSERT(destroyedCount == 1)

		pool.ProcessRetiredHeaps();
		B3D_TEST_ASSERT(destroyedCount == 2 && pool.TakePooledHeap() == nullptr)
	}

	B3D_TEST_ASSERT(destroyedCount == 3)
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

		const u32 structuredIndex = structuredBuffer->GetBindlessIndex();
		const u32 rawIndex = structuredBuffer->GetOrCreateBindlessIndex(GpuBufferViewType::Raw);
		B3D_TEST_ASSERT(structuredIndex != kGpuBindlessInvalidIndex)
		B3D_TEST_ASSERT(rawIndex != kGpuBindlessInvalidIndex && rawIndex != structuredIndex)
		B3D_TEST_ASSERT(structuredBuffer->GetOrCreateBindlessIndex(GpuBufferViewType::Raw) == rawIndex)
		B3D_TEST_ASSERT(structuredBuffer->GetOrCreateBindlessIndex(GpuBufferViewType::Typed) == structuredIndex)

		const u32 typedIndex = typedBuffer->GetBindlessIndex();
		const u32 reinterpretedIndex = typedBuffer->GetOrCreateBindlessIndex(GpuBufferViewType::Typed, BF_16X2U);
		B3D_TEST_ASSERT(typedIndex != kGpuBindlessInvalidIndex)
		B3D_TEST_ASSERT(reinterpretedIndex != kGpuBindlessInvalidIndex && reinterpretedIndex != typedIndex)
		B3D_TEST_ASSERT(typedBuffer->GetOrCreateBindlessIndex(GpuBufferViewType::Typed, BF_32X1U) == typedIndex)
		B3D_TEST_ASSERT(typedBuffer->GetOrCreateBindlessIndex(GpuBufferViewType::Typed) == typedIndex)

		{
			LoggingScope logs(*this);
			logs.ExpectError("Raw bindless views are only supported for structured storage buffers.");
			B3D_TEST_ASSERT(typedBuffer->GetOrCreateBindlessIndex(GpuBufferViewType::Raw) == kGpuBindlessInvalidIndex)
		}

		{
			LoggingScope logs(*this);
			logs.ExpectError("Only typed bindless views of simple storage buffers can have an element format.");
			B3D_TEST_ASSERT(structuredBuffer->GetOrCreateBindlessIndex(GpuBufferViewType::Typed, BF_32X1U) == kGpuBindlessInvalidIndex)
		}

		GpuBindlessHeapStatistics statistics = heaps.GetStatistics(GpuBindlessHeapType::Resource);
		B3D_TEST_ASSERT(statistics.AllocatedCount == initialAllocatedCount + 6)

		// Destruction releases every slot of the resource
		const u32 pendingReleaseCount = statistics.PendingReleaseCount;
		texture = nullptr;
		structuredBuffer = nullptr;
		typedBuffer = nullptr;

		statistics = heaps.GetStatistics(GpuBindlessHeapType::Resource);
		B3D_TEST_ASSERT(statistics.AllocatedCount == initialAllocatedCount)
		B3D_TEST_ASSERT(statistics.PendingReleaseCount >= pendingReleaseCount + 6)

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

		// Metal ignores mip bias, so a mip clamp makes the sampler differ from the global ones
		SamplerStateCreateInformation clampedInformation = pointClampInformation;
		clampedInformation.MipMax = 8.0f;

		const TShared<SamplerState> clampedSampler = device->FindOrCreateSamplerState(clampedInformation);
		const u32 clampedIndex = clampedSampler->GetBindlessIndex();
		B3D_TEST_ASSERT(clampedIndex > kGpuGlobalSamplerCount)
		B3D_TEST_ASSERT(clampedSampler->GetBindlessIndex() == clampedIndex)
		B3D_TEST_ASSERT(device->FindOrCreateSamplerState(clampedInformation)->GetBindlessIndex() == clampedIndex)
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
		TShared<render::GpuBuffer> structuredBuffer = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32), 4, inputFlags));
		TShared<render::GpuBuffer> typedBuffer = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateSimpleStorage(BF_32X1U, 4, inputFlags));
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

		TShared<render::Texture> texture = device->CreateTexture(textureInformation);
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
		samplerInformation.MipMax = 8.0f;
		const TShared<SamplerState> cachedSampler = device->FindOrCreateSamplerState(samplerInformation);

		u32 indices[] =
		{
			structuredBuffer->GetBindlessIndex(),
			typedBuffer->GetBindlessIndex(),
			texture->GetBindlessIndex() | (cachedSampler->GetBindlessIndex() << 16),
			structuredBuffer->GetOrCreateBindlessIndex(GpuBufferViewType::Raw)
		};

		static constexpr u32 kOutputCount = 7;
		const TShared<GpuWorkContext> context = GpuWorkContext::Create(*device);
		const TShared<render::GpuBuffer> output = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32), kOutputCount, GpuBufferFlag::StoreOnGPU | GpuBufferFlag::AllowUnorderedAccessOnTheGPU));
		const TShared<render::GpuBuffer> readback = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStagingRead(kOutputCount * sizeof(u32)));
		// Set once, so the dispatches after the update and growth switches bind a table written while an earlier heap was current
		const TShared<render::GpuParameterSet> parameterSet = context->GetParameterSetPool().Create(pipeline->GetParameterLayout()->GetSet(0), 0);
		parameterSet->SetStorageBuffer("Output", output);

		const GpuAccessState computeWrite(GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Write);
		const GpuAccessState transferWrite(GpuStageFlag::Transfer, GpuAccessFlag::Write);
		const GpuAccessState textureTransferWrite(GpuStageFlag::Transfer, GpuAccessFlag::Write, GpuImageLayout::TransferDestination);
		const GpuAccessState transferRead(GpuStageFlag::Transfer, GpuAccessFlag::Read);
		const GpuAccessState hostRead(GpuStageFlag::Host, GpuAccessFlag::Read);

		const TShared<GpuCommandBufferPool> pool = GpuBarrierTestUtility::CreateCommandBufferPool(*device);
		const char* names[kOutputCount] = { "structured buffer", "typed buffer", "texture load", "cached sampler", "global sampler", "raw buffer", "raw buffer wide load" };

		// Runs the shader on the slots of indices and checks what it reads through each of them
		auto fnDispatch = [&](const char* name, const TShared<GpuCommandBuffer>& commands, const u32 (&expected)[kOutputCount])
		{
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

			const u32* actual = static_cast<const u32*>(mapping.GetMappedMemory());
			for(u32 outputIndex = 0; outputIndex < kOutputCount; outputIndex++)
			{
				B3D_TEST_ASSERT_MSG(actual[outputIndex] == expected[outputIndex], StringUtility::Format("Read through the {0} {1}: {2}, expected {3}.",
					name, names[outputIndex], actual[outputIndex], expected[outputIndex]))
			}
		};

		TShared<GpuCommandBuffer> commands = pool->Create(GpuCommandBufferCreateInformation::Create("Bindless heap access"));

		// The texture returns to rest after its upload, which is where heaps read it without barriers
		commands->IssueBarriers(GpuTextureBarrier(texture, GpuBarrierTestUtility::GetDiscardedRestingState(*texture), textureTransferWrite));
		commands->CopyBufferToTexture(upload, texture, 0, 0, 0);
		commands->IssueBarriers(GpuTextureBarrier(texture, textureTransferWrite, GpuBackendUtility::GetRestingState(*texture, GQT_GRAPHICS)));

		const u32 expected[kOutputCount] = { 101, 202, 50, 60, 30, 102, 103 };
		fnDispatch("live", commands, expected);
		commands = nullptr;

		// An update changes the view behind a slot once it is applied, at the end of a frame
		GpuBindlessHeaps& heaps = *device->GetBindlessHeaps();
		TShared<render::GpuBuffer> updatedBuffer = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32), 4, inputFlags));
		B3D_TEST_ASSERT(updatedBuffer != nullptr)
		if(updatedBuffer == nullptr)
			return;

		{
			const render::GpuBufferMappedScope mapping = updatedBuffer->Map(GpuMapOption::Write);
			B3D_TEST_ASSERT(mapping.IsValid())
			if(!mapping.IsValid())
				return;

			u32* elements = static_cast<u32*>(mapping.GetMappedMemory());
			for(u32 elementIndex = 0; elementIndex < 4; elementIndex++)
				elements[elementIndex] = 300 + elementIndex;
		}

		// Both buffers are pinned by their own registrations, so the test's slot can view either
		const int updatedSlotOwner = 0;
		const u32 updatedSlot = heaps.AllocateResourceSlot(GpuBindlessResourceView(*structuredBuffer, GpuBufferViewType::Typed, BF_UNKNOWN), &updatedSlotOwner);
		B3D_TEST_ASSERT(updatedSlot != kGpuBindlessInvalidIndex)

		indices[0] = updatedSlot;
		fnDispatch("allocated", pool->Create(GpuCommandBufferCreateInformation::Create("Bindless allocated slot access")), expected);

		heaps.UpdateResourceSlot(updatedSlot, GpuBindlessResourceView(*updatedBuffer, GpuBufferViewType::Typed, BF_UNKNOWN), &updatedSlotOwner, nullptr);
		fnDispatch("pending update", pool->Create(GpuCommandBufferCreateInformation::Create("Bindless pending update access")), expected);

		heaps.ApplyPendingChanges();

		u32 expectedUpdated[kOutputCount];
		std::copy(std::begin(expected), std::end(expected), std::begin(expectedUpdated));
		expectedUpdated[0] = 301;
		fnDispatch("updated", pool->Create(GpuCommandBufferCreateInformation::Create("Bindless updated slot access")), expectedUpdated);

		// Fill the resource heap past its growth headroom. Every slot keeps its view in the grown heap.
		const GpuBindlessHeapStatistics statisticsBeforeGrowth = heaps.GetStatistics(GpuBindlessHeapType::Resource);
		TArray<u32> fillerSlots;
		if(statisticsBeforeGrowth.Capacity < statisticsBeforeGrowth.MaximumCapacity)
		{
			const u32 headroomSlotCount = (u32)((float)statisticsBeforeGrowth.Capacity * gGpuBindlessResourceGrowthHeadroom.Get());
			const u32 freeSlotCount = statisticsBeforeGrowth.Capacity - 1 - statisticsBeforeGrowth.AllocatedCount - statisticsBeforeGrowth.PendingReleaseCount;
			const GpuBindlessResourceView fillerView(*structuredBuffer, GpuBufferViewType::Typed, BF_UNKNOWN);
			for(u32 fillerIndex = 0; fillerIndex + headroomSlotCount <= freeSlotCount; fillerIndex++)
				fillerSlots.Add(heaps.AllocateResourceSlot(fillerView, &updatedSlotOwner));

			heaps.ApplyPendingChanges();

			const GpuBindlessHeapStatistics statisticsAfterGrowth = heaps.GetStatistics(GpuBindlessHeapType::Resource);
			B3D_TEST_ASSERT(statisticsAfterGrowth.GrowthCount == statisticsBeforeGrowth.GrowthCount + 1)
			B3D_TEST_ASSERT(statisticsAfterGrowth.Capacity > statisticsBeforeGrowth.Capacity)

			fnDispatch("grown", pool->Create(GpuCommandBufferCreateInformation::Create("Bindless grown heap access")), expectedUpdated);

			// Slots above the old capacity are allocated right away, and only valid in the grown heap
			const GpuBindlessResourceView grownView(*updatedBuffer, GpuBufferViewType::Typed, BF_UNKNOWN);
			u32 grownSlot = kGpuBindlessInvalidIndex;
			while(fillerSlots.Size() < statisticsAfterGrowth.Capacity)
			{
				grownSlot = heaps.AllocateResourceSlot(grownView, &updatedSlotOwner);
				if(grownSlot == kGpuBindlessInvalidIndex)
					break;

				fillerSlots.Add(grownSlot);
				if(grownSlot >= statisticsBeforeGrowth.Capacity)
					break;
			}

			B3D_TEST_ASSERT(grownSlot != kGpuBindlessInvalidIndex && grownSlot >= statisticsBeforeGrowth.Capacity)

			indices[0] = grownSlot;
			fnDispatch("grown slot", pool->Create(GpuCommandBufferCreateInformation::Create("Bindless grown slot access")), expectedUpdated);
		}

		heaps.ReleaseSlot(GpuBindlessHeapType::Resource, updatedSlot, &updatedSlotOwner);
		for(u32 fillerSlot : fillerSlots)
			heaps.ReleaseSlot(GpuBindlessHeapType::Resource, fillerSlot, &updatedSlotOwner);

		indices[0] = structuredBuffer->GetBindlessIndex();
		updatedBuffer = nullptr;

#if B3D_BUILD_TYPE_DEVELOPMENT
		// Development builds clear retired slots with null descriptors matching the view type, so stale indices read zeros. Nothing
		// else allocates slots on this thread, so the slots stay retired.
		texture = nullptr;
		structuredBuffer = nullptr;
		typedBuffer = nullptr;
		device->GetBindlessHeaps()->ProcessAllReleases();

		const u32 expectedRetired[kOutputCount] = {};
		fnDispatch("retired", pool->Create(GpuCommandBufferCreateInformation::Create("Bindless retired slot access")), expectedRetired);
#endif
	}, "GpuBindlessTestSuite::TestHeapAccess", true);
}

void GpuBindlessTestSuite::TestRelocation()
{
	GpuBackend& backend = GpuBackend::Instance();
	if(backend.GetDeviceCount() == 0)
		return;

	const TShared<GpuDevice> device = backend.GetDevice(0);
	if(device->GetBindlessHeaps() == nullptr)
		return;

	const TShared<GpuComputePipelineState> pipeline = CreateTestBindlessHeapAccessPipeline(*device);
	B3D_TEST_ASSERT(pipeline != nullptr)
	if(pipeline == nullptr)
		return;

	GetRenderThread().PostCommand([this, &device, &pipeline]()
	{
		const GpuBufferFlags bufferFlags = GpuBufferFlag::StoreOnGPU | GpuBufferFlag::Bindless;
		const GpuBufferCreateInformation structuredInformation = GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32), 4, bufferFlags);
		const GpuBufferCreateInformation typedInformation = GpuBufferCreateInformation::CreateSimpleStorage(BF_32X1U, 4, bufferFlags);

		TextureCreateInformation textureInformation;
		textureInformation.Name = "Relocated texture";
		textureInformation.Width = 2;
		textureInformation.Height = 1;
		textureInformation.Format = PF_RGBA8;
		textureInformation.Usage = TextureUsageFlag::Default | TextureUsageFlag::Bindless;

		TextureCreateInformation renderTargetInformation = textureInformation;
		renderTargetInformation.Name = "Unmovable render target";
		renderTargetInformation.Usage = TextureUsageFlag::RenderTarget;

		const TShared<GpuWorkContext> context = GpuWorkContext::Create(*device);
		GpuBindlessHeaps& heaps = *device->GetBindlessHeaps();
		GpuMemoryDefragmenter& defragmenter = device->GetMemoryDefragmenter();

		// Runs defragmentation passes until fnIsDone() returns true, or a pass moves nothing
		const auto fnDefragment = [&context, &device, &heaps, &defragmenter](const auto& fnIsDone)
		{
			const bool wasEnabled = gGpuDefragmentation.Get();
			gGpuDefragmentation.Set(true);

			for(u32 passIndex = 0; passIndex < 256; passIndex++)
			{
				defragmenter.Run(*context);
				context->SubmitTransferCommandBuffers(true);
				heaps.ApplyPendingChanges();
				device->WaitUntilIdle();
				heaps.ProcessAllReleases();

				if(fnIsDone() || defragmenter.GetStatistics().LastPassMoveCount == 0)
					break;
			}

			gGpuDefragmentation.Set(wasEnabled);
		};

		// Memory freed by earlier tests is compacted first, so the resources created below are placed in order after the used memory.
		// Resources created before the tested ones and destroyed once they exist then leave free memory below them.
		fnDefragment([]() { return false; });

		TArray<TShared<render::GpuBuffer>> fillerBuffers;
		TArray<TShared<render::Texture>> fillerTextures;
		for(u32 fillerIndex = 0; fillerIndex < 16; fillerIndex++)
		{
			fillerBuffers.Add(device->CreateGpuBuffer(structuredInformation));
			fillerBuffers.Add(device->CreateGpuBuffer(typedInformation));
			fillerTextures.Add(device->CreateTexture(textureInformation));
		}

		const TShared<render::GpuBuffer> structuredBuffer = device->CreateGpuBuffer(structuredInformation);
		const TShared<render::GpuBuffer> typedBuffer = device->CreateGpuBuffer(typedInformation);
		const TShared<render::Texture> texture = device->CreateTexture(textureInformation);
		const TShared<render::GpuBuffer> cpuBuffer = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32), 4, GpuBufferFlag::StoreOnCPUWithGPUAccess));
		const TShared<render::Texture> renderTarget = device->CreateTexture(renderTargetInformation);
		B3D_TEST_ASSERT(structuredBuffer != nullptr && typedBuffer != nullptr && texture != nullptr && cpuBuffer != nullptr && renderTarget != nullptr)
		if(structuredBuffer == nullptr || typedBuffer == nullptr || texture == nullptr || cpuBuffer == nullptr || renderTarget == nullptr)
			return;

		// The buffers hold (100, 101, 102, 103) and (200, 201, 202, 203). Texel 0 is (10, 20, 30, 40) and texel 1 is (50, 60, 70, 80).
		const ImageSubresourcePitch pitch = texture->GetStagingBufferPitchForSubresource(0, 0);
		const TShared<render::GpuBuffer> uploads[] =
		{
			device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStagingWrite(4 * sizeof(u32))),
			device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStagingWrite(4 * sizeof(u32))),
			device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStagingWrite(pitch.RowPitch * pitch.SliceHeight * sizeof(u32)))
		};

		for(u32 uploadIndex = 0; uploadIndex < 3; uploadIndex++)
		{
			const render::GpuBufferMappedScope mapping = uploads[uploadIndex]->Map(GpuMapOption::Write);
			B3D_TEST_ASSERT(mapping.IsValid())
			if(!mapping.IsValid())
				return;

			u32* words = static_cast<u32*>(mapping.GetMappedMemory());
			if(uploadIndex < 2)
			{
				for(u32 elementIndex = 0; elementIndex < 4; elementIndex++)
					words[elementIndex] = (uploadIndex + 1) * 100 + elementIndex;
			}
			else
			{
				words[0] = 10 | (20 << 8) | (30 << 16) | (40u << 24);
				words[1] = 50 | (60 << 8) | (70 << 16) | (80u << 24);
			}
		}

		const GpuAccessState computeWrite(GpuStageFlag::ComputeShaderNonUniform, GpuAccessFlag::Write);
		const GpuAccessState transferWrite(GpuStageFlag::Transfer, GpuAccessFlag::Write);
		const GpuAccessState textureTransferWrite(GpuStageFlag::Transfer, GpuAccessFlag::Write, GpuImageLayout::TransferDestination);
		const GpuAccessState transferRead(GpuStageFlag::Transfer, GpuAccessFlag::Read);
		const GpuAccessState hostRead(GpuStageFlag::Host, GpuAccessFlag::Read);

		const TShared<GpuCommandBufferPool> pool = GpuBarrierTestUtility::CreateCommandBufferPool(*device);

		// Every resource returns to rest after its upload, which is where heaps read it and the defragmenter moves it
		{
			const TShared<GpuCommandBuffer> commands = pool->Create(GpuCommandBufferCreateInformation::Create("Relocation upload"));
			const GpuBufferBarrier uploadBarriers[] = { GpuBufferBarrier(structuredBuffer, GpuAccessState(), transferWrite), GpuBufferBarrier(typedBuffer, GpuAccessState(), transferWrite) };
			commands->IssueBarriers(GpuBarriers(TArrayView<const GpuBufferBarrier>(uploadBarriers, 2)));
			commands->IssueBarriers(GpuTextureBarrier(texture, GpuBarrierTestUtility::GetDiscardedRestingState(*texture), textureTransferWrite));

			commands->CopyBufferToBuffer(uploads[0], structuredBuffer, 0, 0, 4 * sizeof(u32));
			commands->CopyBufferToBuffer(uploads[1], typedBuffer, 0, 0, 4 * sizeof(u32));
			commands->CopyBufferToTexture(uploads[2], texture, 0, 0, 0);

			const GpuBufferBarrier restBarriers[] = { GpuBufferBarrier(structuredBuffer, transferWrite, GpuBackendUtility::GetRestingState(*structuredBuffer, GQT_GRAPHICS)),
				GpuBufferBarrier(typedBuffer, transferWrite, GpuBackendUtility::GetRestingState(*typedBuffer, GQT_GRAPHICS)) };
			commands->IssueBarriers(GpuBarriers(TArrayView<const GpuBufferBarrier>(restBarriers, 2)));
			commands->IssueBarriers(GpuTextureBarrier(texture, textureTransferWrite, GpuBackendUtility::GetRestingState(*texture, GQT_GRAPHICS)));

			context->SubmitCommandBuffer(commands);
			device->WaitUntilIdle();
		}

		fillerBuffers.Clear();
		fillerTextures.Clear();
		heaps.ProcessAllReleases();

		SamplerStateCreateInformation samplerInformation;
		samplerInformation.MinFilter = FO_POINT;
		samplerInformation.MagFilter = FO_POINT;
		samplerInformation.MipFilter = FO_POINT;
		samplerInformation.MipMax = 8.0f;
		const TShared<SamplerState> cachedSampler = device->FindOrCreateSamplerState(samplerInformation);

		const u32 indices[] =
		{
			structuredBuffer->GetBindlessIndex(),
			typedBuffer->GetBindlessIndex(),
			texture->GetBindlessIndex() | (cachedSampler->GetBindlessIndex() << 16),
			structuredBuffer->GetOrCreateBindlessIndex(GpuBufferViewType::Raw)
		};

		// Run passes until the tested resources move. Each pass moves the resources highest in memory first, which may be ones other
		// tests left behind.
		static constexpr u32 kMovedCount = 3;
		const IGpuResource* const originalNativeResources[kMovedCount] = { structuredBuffer->GetGpuResource(), typedBuffer->GetGpuResource(), texture->GetGpuResource() };
		const IGpuResource* const cpuBufferNativeResource = cpuBuffer->GetGpuResource();
		const IGpuResource* const renderTargetNativeResource = renderTarget->GetGpuResource();

		const u64 initialMoveCount = defragmenter.GetStatistics().MoveCount;

		bool isMoved[kMovedCount] = {};
		fnDefragment([&]()
		{
			const IGpuResource* const nativeResources[kMovedCount] = { structuredBuffer->GetGpuResource(), typedBuffer->GetGpuResource(), texture->GetGpuResource() };
			bool isEveryResourceMoved = true;
			for(u32 resourceIndex = 0; resourceIndex < kMovedCount; resourceIndex++)
			{
				isMoved[resourceIndex] |= nativeResources[resourceIndex] != originalNativeResources[resourceIndex];
				isEveryResourceMoved &= isMoved[resourceIndex];
			}

			return isEveryResourceMoved;
		});

		B3D_TEST_ASSERT(defragmenter.GetStatistics().MoveCount > initialMoveCount)
		B3D_TEST_ASSERT_MSG(isMoved[0] && isMoved[1] && isMoved[2], StringUtility::Format("Moved: structured buffer {0}, typed buffer {1}, texture {2}.",
			isMoved[0], isMoved[1], isMoved[2]))
		B3D_TEST_ASSERT(cpuBuffer->GetGpuResource() == cpuBufferNativeResource)
		B3D_TEST_ASSERT(renderTarget->GetGpuResource() == renderTargetNativeResource)

		// The moved resources keep their indices, and both bindless and directly bound accesses see their contents
		B3D_TEST_ASSERT(structuredBuffer->GetBindlessIndex() == indices[0])
		B3D_TEST_ASSERT(typedBuffer->GetBindlessIndex() == indices[1])
		B3D_TEST_ASSERT(texture->GetBindlessIndex() == (indices[2] & 0xFFFF))
		B3D_TEST_ASSERT(structuredBuffer->GetOrCreateBindlessIndex(GpuBufferViewType::Raw) == indices[3])

		static constexpr u32 kOutputCount = 7;
		const TShared<render::GpuBuffer> output = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(sizeof(u32), kOutputCount, GpuBufferFlag::StoreOnGPU | GpuBufferFlag::AllowUnorderedAccessOnTheGPU));
		const TShared<render::GpuBuffer> readback = device->CreateGpuBuffer(GpuBufferCreateInformation::CreateStagingRead((kOutputCount + 4) * sizeof(u32)));
		const TShared<render::GpuParameterSet> parameterSet = context->GetParameterSetPool().Create(pipeline->GetParameterLayout()->GetSet(0), 0);
		parameterSet->SetStorageBuffer("Output", output);

		const TShared<GpuCommandBuffer> commands = pool->Create(GpuCommandBufferCreateInformation::Create("Relocated resource access"));
		commands->IssueBarriers(GpuBufferBarrier(output, GpuAccessState(), computeWrite));
		commands->SetGpuComputePipelineState(pipeline);
		commands->SetGpuParameterSet(parameterSet);
		commands->SetPushConstants(0, sizeof(indices), indices);
		commands->DispatchCompute(1);

		const GpuAccessState structuredRestingState = GpuBackendUtility::GetRestingState(*structuredBuffer, GQT_GRAPHICS);
		const GpuBufferBarrier copyBarriers[] = { GpuBufferBarrier(output, computeWrite, transferRead), GpuBufferBarrier(structuredBuffer, structuredRestingState, transferRead),
			GpuBufferBarrier(readback, GpuAccessState(), transferWrite) };
		commands->IssueBarriers(GpuBarriers(TArrayView<const GpuBufferBarrier>(copyBarriers, 3)));
		commands->CopyBufferToBuffer(output, readback, 0, 0, kOutputCount * sizeof(u32));
		commands->CopyBufferToBuffer(structuredBuffer, readback, 0, kOutputCount * sizeof(u32), 4 * sizeof(u32));
		commands->IssueBarriers(GpuBufferBarrier(structuredBuffer, transferRead, structuredRestingState));
		commands->IssueBarriers(GpuBufferBarrier(readback, transferWrite, hostRead));
		context->SubmitCommandBuffer(commands);
		device->WaitUntilIdle();

		const render::GpuBufferMappedScope mapping = readback->Map(GpuMapOption::Read);
		B3D_TEST_ASSERT(mapping.IsValid())
		if(!mapping.IsValid())
			return;

		const char* names[kOutputCount + 4] = { "structured buffer", "typed buffer", "texture load", "cached sampler", "global sampler", "raw buffer",
			"raw buffer wide load", "copy element 0", "copy element 1", "copy element 2", "copy element 3" };
		const u32 expected[kOutputCount + 4] = { 101, 202, 50, 60, 30, 102, 103, 100, 101, 102, 103 };
		const u32* actual = static_cast<const u32*>(mapping.GetMappedMemory());
		for(u32 outputIndex = 0; outputIndex < kOutputCount + 4; outputIndex++)
		{
			B3D_TEST_ASSERT_MSG(actual[outputIndex] == expected[outputIndex], StringUtility::Format("Read through the {0} after relocation: {1}, expected {2}.",
				names[outputIndex], actual[outputIndex], expected[outputIndex]))
		}
	}, "GpuBindlessTestSuite::TestRelocation", true);
}

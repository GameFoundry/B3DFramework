//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GpuBackend/B3DGpuBindlessHeap.h"
#include "GpuBackend/B3DGpuBindlessHeapPool.h"
#include "GpuBackend/B3DGpuBackend.h"
#include "GpuBackend/B3DGpuBuffer.h"
#include "GpuBackend/B3DGpuCompletionTracker.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DSamplerState.h"
#include "GpuBackend/Allocators/B3DGpuResource.h"
#include "Image/B3DTexture.h"
#include "CoreObject/B3DRenderThread.h"
#include "Utility/B3DBitwise.h"

using namespace b3d;

#if B3D_BUILD_TYPE_DEVELOPMENT
/** Returns the parameter type that shaders read @p view as. */
static GpuParameterObjectType GetViewObjectType(const GpuBindlessResourceView& view)
{
	if(view.Buffer != nullptr)
	{
		if(view.Buffer->GetInformation().Type == GpuBufferType::AccelerationStructure)
			return GPOT_ACCELERATION_STRUCTURE;

		if(view.ViewType == GpuBufferViewType::Raw)
			return GPOT_BYTE_ADDRESS_BUFFER;

		return view.Buffer->GetInformation().Type == GpuBufferType::StructuredStorage ? GPOT_STRUCTURED_BUFFER : GPOT_TYPED_BUFFER;
	}

	const TextureProperties& properties = view.Texture->GetProperties();
	const TextureViewType viewType = properties.ResolveSurface(view.Surface, false).ViewType;
	return GpuObjectParameterTypeInformation::GetTextureObjectType(viewType, properties.SampleCount > 1);
}
#endif

GpuBindlessResourceView::GpuBindlessResourceView(render::Texture& texture, const TextureSurface& surface)
	: Texture(&texture), Surface(surface)
{ }

GpuBindlessResourceView::GpuBindlessResourceView(render::GpuBuffer& buffer, GpuBufferViewType viewType, GpuBufferFormat format)
	: Buffer(&buffer), ViewType(viewType), Format(format)
{ }

bool GpuBindlessResourceView::operator==(const GpuBindlessResourceView& other) const
{
	return Texture == other.Texture && Surface == other.Surface && Buffer == other.Buffer && ViewType == other.ViewType && Format == other.Format;
}

GpuBindlessHeaps::GpuBindlessHeaps(GpuDevice& device, const IGpuCompletionTracker& completionTracker, const GpuBindlessHeapsCreateInformation& createInformation)
	: mDevice(device), mCompletionTracker(completionTracker), mUpdateMethod(createInformation.UpdateMethod)
	, mMaximumResourceHeapCount(createInformation.MaximumResourceHeapCount)
{
	B3D_ASSERT(createInformation.SamplerCapacity > kGpuGlobalSamplerCount + 1 && createInformation.ResourceMaximumCapacity >= createInformation.ResourceCapacity);

	const u32 capacities[] = { createInformation.ResourceCapacity, createInformation.SamplerCapacity };
	const u32 maximumCapacities[] = { createInformation.ResourceMaximumCapacity, createInformation.SamplerCapacity };
	for(u32 heapIndex = 0; heapIndex < (u32)GpuBindlessHeapType::Count; heapIndex++)
	{
		Heap& heap = mHeaps[heapIndex];
		heap.Capacity = capacities[heapIndex];
		heap.MaximumCapacity = maximumCapacities[heapIndex];
		heap.LiveSlots.Resize(capacities[heapIndex]);
		heap.Statistics.Capacity = capacities[heapIndex];
		heap.Statistics.MaximumCapacity = maximumCapacities[heapIndex];

#if B3D_BUILD_TYPE_DEVELOPMENT
		heap.SlotRecords.Resize(capacities[heapIndex]);
#endif
	}
}

GpuBindlessHeaps::~GpuBindlessHeaps()
{
	B3D_ASSERT(mResourceHeapPool == nullptr && "The subclass destructor must call Destroy().");
}

bool GpuBindlessHeaps::Initialize()
{
	B3D_ASSERT(mResourceHeapPool == nullptr);

	GpuBindlessHeap* heap = CreateResourceHeap(mHeaps[(u32)GpuBindlessHeapType::Resource].Capacity);
	if(heap == nullptr)
	{
		B3D_LOG(Error, LogRenderBackend, "Failed to create the native heap of the bindless resource heap.");
		return false;
	}

	mResourceHeapPool = B3DMakeUnique<GpuBindlessHeapPool>(mMaximumResourceHeapCount);
	mResourceHeapPool->Initialize(heap);
	return true;
}

void GpuBindlessHeaps::Destroy()
{
	if(mResourceHeapPool == nullptr)
		return;

	// Destroying the global samplers releases their slots, which takes the lock
	mGlobalSamplers.Clear();

	for(const auto& entry : mPendingSlotUpdates)
	{
		if(entry.second.PreviousTarget != nullptr)
			mDevice.UnpinResource(*entry.second.PreviousTarget);
	}

	mPendingSlotUpdates.clear();
	ProcessAllReleases();

#if B3D_BUILD_TYPE_DEVELOPMENT
	// Report slots that were never released
	Lock lock(mMutex);

	const char* heapNames[] = { "resource", "sampler" };
	for(u32 heapIndex = 0; heapIndex < (u32)GpuBindlessHeapType::Count; heapIndex++)
	{
		const Heap& heap = mHeaps[heapIndex];

		// The global samplers are permanent
		const u32 firstSlotIndex = heapIndex == (u32)GpuBindlessHeapType::Sampler ? kGpuGlobalSamplerCount + 1 : 1;
		for(u32 slotIndex = firstSlotIndex; slotIndex < heap.NextUnusedSlotIndex; slotIndex++)
		{
			const SlotRecord& record = heap.SlotRecords[slotIndex];
			if(record.Owner == nullptr || record.IsPendingRelease)
				continue;

			B3D_LOG(Warning, LogRenderBackend, "Bindless {0} slot {1} of '{2}' was never released.", heapNames[heapIndex], slotIndex, record.OwnerName);
		}
	}
#endif

	mResourceHeapPool = nullptr;
}

void GpuBindlessHeaps::CreateGlobalSamplers()
{
	B3D_ASSERT(mGlobalSamplers.Empty() && mHeaps[(u32)GpuBindlessHeapType::Sampler].NextUnusedSlotIndex == 1);

	for(u32 filterIndex = 0; filterIndex < (u32)GpuGlobalSamplerFilter::Count; filterIndex++)
	{
		for(u32 addressIndex = 0; addressIndex < (u32)GpuGlobalSamplerAddress::Count; addressIndex++)
		{
			const auto filter = (GpuGlobalSamplerFilter)filterIndex;
			const TextureAddressingMode addressMode = (GpuGlobalSamplerAddress)addressIndex == GpuGlobalSamplerAddress::Wrap ? TAM_WRAP : TAM_CLAMP;

			SamplerStateCreateInformation createInformation;
			createInformation.AddressMode.U = addressMode;
			createInformation.AddressMode.V = addressMode;
			createInformation.AddressMode.W = addressMode;
			createInformation.MinFilter = filter == GpuGlobalSamplerFilter::Point ? FO_POINT : FO_LINEAR;
			createInformation.MagFilter = filter == GpuGlobalSamplerFilter::Point ? FO_POINT : FO_LINEAR;
			createInformation.MipFilter = filter == GpuGlobalSamplerFilter::Trilinear ? FO_LINEAR : FO_POINT;

			const TShared<SamplerState> sampler = mDevice.FindOrCreateSamplerState(createInformation);
			mGlobalSamplers.Add(sampler);

			// Global samplers are allocated before any other sampler slot, so they get the slots following the invalid slot in order
			const u32 slotIndex = sampler->GetBindlessIndex();
			B3D_ASSERT(slotIndex == GetGlobalSamplerIndex(filter, (GpuGlobalSamplerAddress)addressIndex));
			(void)slotIndex;
		}
	}
}

u32 GpuBindlessHeaps::TakeSlot(Heap& heap, const void* owner, const String& ownerName)
{
	u32 slotIndex;
	if(!heap.FreeSlotIndices.Empty())
	{
		slotIndex = heap.FreeSlotIndices.Back();
		heap.FreeSlotIndices.Pop();
	}
	else if(heap.NextUnusedSlotIndex < heap.Capacity)
		slotIndex = heap.NextUnusedSlotIndex++;
	else
		return kGpuBindlessInvalidIndex;

	heap.LiveSlots[slotIndex] = true;
	heap.Statistics.AllocatedCount++;
	heap.Statistics.HighWaterMark = std::max(heap.Statistics.HighWaterMark, heap.Statistics.AllocatedCount);

#if B3D_BUILD_TYPE_DEVELOPMENT
	SlotRecord& record = heap.SlotRecords[slotIndex];
	record.Owner = owner;
	record.OwnerName = ownerName;
	record.IsPendingRelease = false;
#else
	(void)owner;
	(void)ownerName;
#endif

	return slotIndex;
}

u32 GpuBindlessHeaps::AllocateResourceSlot(const GpuBindlessResourceView& view, const void* owner)
{
	const String& ownerName = view.Texture != nullptr ? view.Texture->GetName() : view.Buffer->GetName();
	if(view.Texture != nullptr && !view.Texture->GetProperties().ValidateSurface(view.Surface, false))
		return kGpuBindlessInvalidIndex;

	Lock lock(mMutex);

	Heap& heap = mHeaps[(u32)GpuBindlessHeapType::Resource];
	const u32 slotIndex = TakeSlot(heap, owner, ownerName);
	if(slotIndex == kGpuBindlessInvalidIndex)
	{
		if(heap.Capacity < heap.MaximumCapacity)
		{
			B3D_LOG(Error, LogRenderBackend, "The bindless resource heap is full ({0} slots). Cannot allocate a slot for '{1}'. The heap grows at the end of a frame, "
				"but more slots were allocated in one frame than its growth headroom. Increase gpu.BindlessResourceCapacity or gpu.BindlessResourceGrowthHeadroom.",
				heap.Capacity, ownerName);
		}
		else
		{
			B3D_LOG(Error, LogRenderBackend, "The bindless resource heap is full ({0} slots). Cannot allocate a slot for '{1}'. Increase gpu.BindlessResourceMaximumCapacity.",
				heap.Capacity, ownerName);
		}

		return kGpuBindlessInvalidIndex;
	}

#if B3D_BUILD_TYPE_DEVELOPMENT
	heap.SlotRecords[slotIndex].ViewType = GetViewObjectType(view);
#endif

	// The writes are serialized, since native descriptor writes into one native heap are not thread safe
	WriteResourceSlot(slotIndex, view);
	return slotIndex;
}

u32 GpuBindlessHeaps::AllocateSamplerSlot(const SamplerState& sampler, const void* owner)
{
	Heap& heap = mHeaps[(u32)GpuBindlessHeapType::Sampler];

	bool shouldEvict;
	{
		Lock lock(mMutex);
		shouldEvict = (float)(heap.Statistics.AllocatedCount + 1) > (float)heap.Capacity * gGpuBindlessSamplerEvictionThreshold;
	}

	// Evicted samplers release their slots through this object, so eviction must run without the lock held
	if(shouldEvict)
	{
		const u32 evictedCount = mDevice.EvictUnusedSamplerStates();

		Lock lock(mMutex);
		heap.Statistics.EvictionCount += evictedCount;
	}

	static const String kSamplerOwnerName = "Sampler";

	Lock lock(mMutex);

	const u32 slotIndex = TakeSlot(heap, owner, kSamplerOwnerName);
	if(slotIndex == kGpuBindlessInvalidIndex)
	{
		B3D_LOG(Error, LogRenderBackend, "The bindless sampler heap is full ({0} slots). Increase gpu.BindlessSamplerCapacity, or create fewer distinct samplers.", heap.Capacity);
		return kGpuBindlessInvalidIndex;
	}

	WriteSamplerSlot(slotIndex, sampler);
	return slotIndex;
}

void GpuBindlessHeaps::UpdateResourceSlot(u32 slotIndex, const GpuBindlessResourceView& view, const void* owner, IGpuResource* previousTarget)
{
	if(view.Texture != nullptr && !view.Texture->GetProperties().ValidateSurface(view.Surface, false))
		return;

	Lock lock(mMutex);

	Heap& heap = mHeaps[(u32)GpuBindlessHeapType::Resource];
	B3D_ASSERT(slotIndex != kGpuBindlessInvalidIndex && slotIndex < heap.NextUnusedSlotIndex);

#if B3D_BUILD_TYPE_DEVELOPMENT
	SlotRecord& record = heap.SlotRecords[slotIndex];
	if(!B3D_ENSURE_LOG(record.Owner != nullptr && !record.IsPendingRelease, "Bindless slot {0} updated after its release, or never allocated.", slotIndex))
		return;

	if(!B3D_ENSURE_LOG(record.Owner == owner, "Bindless slot {0} of '{1}' updated by an object other than its owner.", slotIndex, record.OwnerName))
		return;

	record.ViewType = GetViewObjectType(view);
#else
	(void)owner;
#endif

	const GpuBindlessSlotUpdate update(slotIndex, view);
	const auto [iterator, isInserted] = mPendingSlotUpdates.try_emplace(slotIndex, update, previousTarget);
	if(isInserted)
		return;

	// The replaced update was never applied, so nothing reads its view, but the old descriptor is still read until this one applies
	iterator->second.Update = update;
	if(previousTarget != nullptr)
		mDevice.UnpinResource(*previousTarget);
}

void GpuBindlessHeaps::ReleaseSlot(GpuBindlessHeapType heapType, u32 slotIndex, const void* owner)
{
	Lock lock(mMutex);

	Heap& heap = mHeaps[(u32)heapType];
	B3D_ASSERT(slotIndex != kGpuBindlessInvalidIndex && slotIndex < heap.NextUnusedSlotIndex);

#if B3D_BUILD_TYPE_DEVELOPMENT
	SlotRecord& record = heap.SlotRecords[slotIndex];
	if(!B3D_ENSURE_LOG(record.Owner != nullptr && !record.IsPendingRelease, "Bindless slot {0} released twice, or never allocated.", slotIndex))
		return;

	if(!B3D_ENSURE_LOG(record.Owner == owner, "Bindless slot {0} of '{1}' released by an object other than its owner.", slotIndex, record.OwnerName))
		return;

	record.IsPendingRelease = true;
#else
	(void)owner;

	// The slot's descriptor stays as it is, and refers to a resource destroyed once the release completes
	heap.LiveSlots[slotIndex] = false;
#endif

	if(heapType == GpuBindlessHeapType::Resource)
	{
		const auto iterator = mPendingSlotUpdates.find(slotIndex);
		if(iterator != mPendingSlotUpdates.end())
		{
			if(iterator->second.PreviousTarget != nullptr)
				mDevice.UnpinResource(*iterator->second.PreviousTarget);

			mPendingSlotUpdates.erase(iterator);
		}
	}

	heap.Statistics.AllocatedCount--;
	heap.Statistics.PendingReleaseCount++;

	mPendingSlotReleases.Add(PendingSlotRelease(mCompletionTracker.GetCurrentMarker(), heapType, slotIndex));
}

template<class IsCompleteFunction>
void GpuBindlessHeaps::ProcessReleases(IsCompleteFunction fnIsComplete)
{
	Lock lock(mMutex);

	u32 completeSlotReleaseCount = 0;
	for(const PendingSlotRelease& release : mPendingSlotReleases)
	{
		if(!fnIsComplete(release.Marker))
			break;

		completeSlotReleaseCount++;

		Heap& heap = mHeaps[(u32)release.HeapType];
		heap.FreeSlotIndices.Add(release.SlotIndex);
		heap.Statistics.PendingReleaseCount--;

#if B3D_BUILD_TYPE_DEVELOPMENT
		SlotRecord& record = heap.SlotRecords[release.SlotIndex];
		const GpuParameterObjectType viewType = record.ViewType;
		record = SlotRecord();

		// Nothing reads the slot anymore. A null descriptor makes reads through stale indices well defined, and visible in captures.
		if(release.HeapType == GpuBindlessHeapType::Resource)
			ClearResourceSlot(release.SlotIndex, viewType);
#endif

		// Samplers have no null descriptor, so retired sampler slots get a global sampler instead, unless the heaps are being destroyed. Unlike
		// resource slots, this happens in all builds, so every sampler slot ever written stays valid for backends to copy.
		if(release.HeapType == GpuBindlessHeapType::Sampler && !mGlobalSamplers.Empty())
			WriteSamplerSlot(release.SlotIndex, *mGlobalSamplers[GetGlobalSamplerIndex(GpuGlobalSamplerFilter::Point, GpuGlobalSamplerAddress::Wrap) - 1]);
	}

	mPendingSlotReleases.Erase(mPendingSlotReleases.begin(), mPendingSlotReleases.begin() + completeSlotReleaseCount);
}

void GpuBindlessHeaps::ProcessReleases()
{
	ProcessReleases([this](u64 marker) { return mCompletionTracker.IsMarkerComplete(marker); });
}

void GpuBindlessHeaps::ProcessAllReleases()
{
	ProcessReleases([](u64) { return true; });
}

void GpuBindlessHeaps::ApplyPendingChanges()
{
	Lock lock(mMutex);

	mResourceHeapPool->ProcessRetiredHeaps();

	Heap& heap = mHeaps[(u32)GpuBindlessHeapType::Resource];
	if(!mPendingSlotUpdates.empty())
	{
		TArray<GpuBindlessSlotUpdate> updates;
		updates.Reserve((u32)mPendingSlotUpdates.size());

		for(const auto& entry : mPendingSlotUpdates)
			updates.Add(entry.second.Update);

		const bool isApplied = mUpdateMethod == GpuBindlessUpdateMethod::Patch ? PatchResourceSlots(updates) : SwitchResourceHeap(heap.Capacity, updates, heap.LiveSlots);
		if(isApplied)
		{
			heap.Statistics.UpdateCount += (u32)updates.Size();
			heap.Statistics.UpdateBatchCount++;
		}
		else
			B3D_LOG(Error, LogRenderBackend, "Failed to apply {0} bindless resource slot updates. The slots keep their old descriptors.", updates.Size());

		// Work recorded from now on reads the new descriptors. Work recorded earlier completes with the current frame.
		for(const auto& entry : mPendingSlotUpdates)
		{
			if(entry.second.PreviousTarget != nullptr)
				mDevice.UnpinResource(*entry.second.PreviousTarget);
		}

		mPendingSlotUpdates.clear();
	}

	GrowIfNeeded();
}

void GpuBindlessHeaps::GrowIfNeeded()
{
	Heap& heap = mHeaps[(u32)GpuBindlessHeapType::Resource];

	if(heap.Capacity >= heap.MaximumCapacity)
		return;

	// Slot 0 is never allocated. Slots pending release are not free yet.
	const u32 usedSlotCount = heap.NextUnusedSlotIndex - 1 - (u32)heap.FreeSlotIndices.Size();
	const u32 freeSlotCount = heap.Capacity - 1 - usedSlotCount;
	if((float)freeSlotCount >= (float)heap.Capacity * gGpuBindlessResourceGrowthHeadroom.Get())
		return;

	const u32 newCapacity = (u32)std::min((u64)heap.Capacity * 2, (u64)heap.MaximumCapacity);
	if(!SwitchResourceHeap(newCapacity, {}, heap.LiveSlots))
	{
		B3D_LOG(Warning, LogRenderBackend, "Failed to grow the bindless resource heap from {0} to {1} slots. It stays at {0} slots.", heap.Capacity, newCapacity);

		heap.MaximumCapacity = heap.Capacity;
		heap.Statistics.MaximumCapacity = heap.Capacity;
		return;
	}

	heap.Capacity = newCapacity;
	heap.LiveSlots.Resize(newCapacity);
	heap.Statistics.Capacity = newCapacity;
	heap.Statistics.GrowthCount++;

#if B3D_BUILD_TYPE_DEVELOPMENT
	heap.SlotRecords.Resize(newCapacity);
#endif
}

GpuBindlessHeapStatistics GpuBindlessHeaps::GetStatistics(GpuBindlessHeapType heap) const
{
	Lock lock(mMutex);
	return mHeaps[(u32)heap].Statistics;
}

GpuBindlessHeap* GpuBindlessHeaps::GetAndTrackResourceHeap(render::GpuResourceTracker& resourceTracker)
{
	return mResourceHeapPool->GetAndTrack(resourceTracker);
}

bool GpuBindlessHeaps::SwitchResourceHeap(u32 capacity, TArrayView<const GpuBindlessSlotUpdate> updates, const TBitfield<>& liveSlots)
{
	GpuBindlessHeapPool& pool = *mResourceHeapPool;
	const u32 currentCapacity = pool.GetCurrent()->GetCapacity();
	B3D_ASSERT(capacity >= currentCapacity);

	// A pooled native heap only lacks the slots changed since it was last current. A new one lacks every slot.
	GpuBindlessHeap* heap = capacity == currentCapacity ? pool.TakePooledHeap() : nullptr;
	const TBitfield<>* slotsToCopy = &liveSlots;
	if(heap != nullptr)
		slotsToCopy = &pool.GetDirtySlots(*heap);
	else
	{
		heap = CreateResourceHeap(capacity);
		if(heap == nullptr)
			return false;
	}

	TArray<TIndexRange<u32>> ranges;
	slotsToCopy->ForEachSetRange(liveSlots, currentCapacity, [&ranges](const TIndexRange<u64>& range)
	{
		ranges.Add(TIndexRange<u32>((u32)range.First, (u32)range.Count));
	});

	if(!ranges.Empty())
		CopyResourceSlots(*heap, ranges);

	if(!updates.IsEmpty())
		WriteResourceSlots(*heap, updates);

	pool.MakeCurrent(heap, updates);
	return true;
}

bool GpuBindlessHeaps::PatchResourceSlots(TArrayView<const GpuBindlessSlotUpdate> /*updates*/)
{
	B3D_ASSERT(false && "Backends that patch resource slots must override PatchResourceSlots().");
	return false;
}

GpuBindlessResourceSlots::~GpuBindlessResourceSlots()
{
	B3D_ASSERT(mHeaps == nullptr && "Bindless resource slots must be unregistered before destruction.");
}

void GpuBindlessResourceSlots::Register(GpuDevice& device, IGpuResource& nativeResource, const GpuBindlessResourceView& defaultView)
{
	B3D_ASSERT(mHeaps == nullptr);

	GpuBindlessHeaps* heaps = device.GetBindlessHeaps();
	if(!B3D_ENSURE_LOG(heaps != nullptr, "Cannot create a bindless resource, the GPU device doesn't support bindless resources."))
		return;

	mHeaps = heaps;
	mDefaultView = defaultView;

	device.PinResource(nativeResource);
	mDefaultIndex = mHeaps->AllocateResourceSlot(defaultView, this);
}

void GpuBindlessResourceSlots::Unregister(IGpuResource& nativeResource)
{
	if(mHeaps == nullptr)
		return;

	if(mDefaultIndex != kGpuBindlessInvalidIndex)
		mHeaps->ReleaseSlot(GpuBindlessHeapType::Resource, mDefaultIndex, this);

	for(const ViewSlot& viewSlot : mViewSlots)
		mHeaps->ReleaseSlot(GpuBindlessHeapType::Resource, viewSlot.Index, this);

	mHeaps->GetDevice().UnpinResource(nativeResource);

	mHeaps = nullptr;
	mDefaultIndex = kGpuBindlessInvalidIndex;
	mViewSlots.Clear();
}

void GpuBindlessResourceSlots::Retarget(IGpuResource& previousNativeResource, IGpuResource& nativeResource)
{
	ASSERT_IF_NOT_RENDER_THREAD;

	if(mHeaps == nullptr)
		return;

	mHeaps->GetDevice().PinResource(nativeResource);

	// The first update carries the previous native resource, so its pin is released once the update is applied and its frame completes
	IGpuResource* previousTarget = &previousNativeResource;
	if(mDefaultIndex != kGpuBindlessInvalidIndex)
	{
		mHeaps->UpdateResourceSlot(mDefaultIndex, mDefaultView, this, previousTarget);
		previousTarget = nullptr;
	}

	for(const ViewSlot& viewSlot : mViewSlots)
	{
		mHeaps->UpdateResourceSlot(viewSlot.Index, viewSlot.View, this, previousTarget);
		previousTarget = nullptr;
	}

	if(previousTarget != nullptr)
		mHeaps->GetDevice().UnpinResource(previousNativeResource);
}

u32 GpuBindlessResourceSlots::GetOrCreateIndex(const GpuBindlessResourceView& view)
{
	ASSERT_IF_NOT_RENDER_THREAD;

	if(mHeaps == nullptr)
		return kGpuBindlessInvalidIndex;

	if(view == mDefaultView)
		return mDefaultIndex;

	for(const ViewSlot& viewSlot : mViewSlots)
	{
		if(viewSlot.View == view)
			return viewSlot.Index;
	}

	const u32 index = mHeaps->AllocateResourceSlot(view, this);
	if(index != kGpuBindlessInvalidIndex)
		mViewSlots.Add(ViewSlot(view, index));

	return index;
}

//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GpuBackend/B3DGpuBindlessHeap.h"
#include "GpuBackend/B3DGpuBackend.h"
#include "GpuBackend/B3DGpuBuffer.h"
#include "GpuBackend/B3DGpuCompletionTracker.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DSamplerState.h"
#include "GpuBackend/Allocators/B3DGpuResource.h"
#include "Image/B3DTexture.h"
#include "CoreObject/B3DRenderThread.h"

using namespace b3d;

GpuBindlessResourceView::GpuBindlessResourceView(render::Texture& texture, const TextureSurface& surface)
	: Texture(&texture), Surface(surface)
{ }

GpuBindlessResourceView::GpuBindlessResourceView(render::GpuBuffer& buffer, GpuBufferFormat format)
	: Buffer(&buffer), Format(format)
{ }

bool GpuBindlessResourceView::operator==(const GpuBindlessResourceView& other) const
{
	return Texture == other.Texture && Surface == other.Surface && Buffer == other.Buffer && Format == other.Format;
}

GpuBindlessHeaps::GpuBindlessHeaps(GpuDevice& device, IGpuBindlessHeap& backend, const IGpuCompletionTracker& completionTracker, u32 resourceCapacity, u32 samplerCapacity)
	: mDevice(device), mBackend(backend), mCompletionTracker(completionTracker)
{
	B3D_ASSERT(samplerCapacity > kGpuGlobalSamplerCount + 1);

	const u32 capacities[] = { resourceCapacity, samplerCapacity };
	for(u32 heapIndex = 0; heapIndex < (u32)GpuBindlessHeapType::Count; heapIndex++)
	{
		Heap& heap = mHeaps[heapIndex];
		heap.Capacity = capacities[heapIndex];
		heap.Statistics.Capacity = capacities[heapIndex];

#if B3D_BUILD_TYPE_DEVELOPMENT
		heap.SlotRecords.Resize(capacities[heapIndex]);
#endif
	}
}

GpuBindlessHeaps::~GpuBindlessHeaps()
{
	// Destroying the global samplers releases their slots, which takes the lock
	mGlobalSamplers.Clear();

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

	Lock lock(mMutex);

	Heap& heap = mHeaps[(u32)GpuBindlessHeapType::Resource];
	const u32 slotIndex = TakeSlot(heap, owner, ownerName);
	if(slotIndex == kGpuBindlessInvalidIndex)
	{
		B3D_LOG(Error, LogRenderBackend, "The bindless resource heap is full ({0} slots). Cannot allocate a slot for '{1}'. Increase gpu.BindlessResourceCapacity.", heap.Capacity, ownerName);
		return kGpuBindlessInvalidIndex;
	}

	// The backend writes are serialized, since native descriptor writes into one heap are not thread safe
	mBackend.WriteResourceSlot(slotIndex, view);
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

	mBackend.WriteSamplerSlot(slotIndex, sampler);
	return slotIndex;
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
#endif

	heap.Statistics.AllocatedCount--;
	heap.Statistics.PendingReleaseCount++;

	mPendingSlotReleases.Add(PendingSlotRelease(mCompletionTracker.GetCurrentMarker(), heapType, slotIndex));
}

void GpuBindlessHeaps::PinResource(IGpuResource& resource)
{
	resource.NotifyBound();
}

void GpuBindlessHeaps::UnpinResource(IGpuResource& resource)
{
	Lock lock(mMutex);
	mPendingUnpins.Add(PendingUnpin(mCompletionTracker.GetCurrentMarker(), &resource));
}

template<class IsCompleteFunction>
void GpuBindlessHeaps::ProcessReleases(IsCompleteFunction fnIsComplete)
{
	TInlineArray<IGpuResource*, 16> unpinnedResources;
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
			heap.SlotRecords[release.SlotIndex] = SlotRecord();

			// Nothing reads the slot anymore. A null descriptor makes reads through stale indices well defined, and visible in captures.
			mBackend.ClearSlot(release.HeapType, release.SlotIndex);
#endif
		}

		mPendingSlotReleases.Erase(mPendingSlotReleases.begin(), mPendingSlotReleases.begin() + completeSlotReleaseCount);

		u32 completeUnpinCount = 0;
		for(const PendingUnpin& unpin : mPendingUnpins)
		{
			if(!fnIsComplete(unpin.Marker))
				break;

			completeUnpinCount++;
			unpinnedResources.Add(unpin.Resource);
		}

		mPendingUnpins.Erase(mPendingUnpins.begin(), mPendingUnpins.begin() + completeUnpinCount);
	}

	// Unpinning may destroy the resource, so it runs without the lock held
	for(IGpuResource* resource : unpinnedResources)
		resource->NotifyUnbound();
}

void GpuBindlessHeaps::ProcessReleases()
{
	ProcessReleases([this](u64 marker) { return mCompletionTracker.IsMarkerComplete(marker); });
}

void GpuBindlessHeaps::ProcessAllReleases()
{
	ProcessReleases([](u64) { return true; });
}

GpuBindlessHeapStatistics GpuBindlessHeaps::GetStatistics(GpuBindlessHeapType heap) const
{
	Lock lock(mMutex);
	return mHeaps[(u32)heap].Statistics;
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

	mHeaps->PinResource(nativeResource);
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

	mHeaps->UnpinResource(nativeResource);

	mHeaps = nullptr;
	mDefaultIndex = kGpuBindlessInvalidIndex;
	mViewSlots.Clear();
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

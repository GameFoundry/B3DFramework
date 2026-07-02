//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanResource.h"
#include "B3DVulkanGpuCommandBuffer.h"
#include "CoreObject/B3DRenderThread.h"
#include "GpuBackend/B3DGpuSwapChain.h"

using namespace b3d;
using namespace b3d::render;

template<class TBase>
void TVulkanResource<TBase>::OnNotifyUsed(GpuQueueId queueId, GpuAccessFlags useFlags)
{
	// Called under IGpuResource::mMutex from inside NotifyUsed, after the aggregate use counter has been incremented.
	// IGpuResource has already incremented mUsedCount, so a value > 1 means there were prior in-flight uses.
	const bool wasInUse = this->mUsedCount > 1;
	if(wasInUse && mState == State::Normal) // Used without support for concurrency
	{
		B3D_ASSERT(mOwnedQueueType == queueId.GetType() && "Vulkan resource without concurrency support can only be used by one queue family at once.");
	}

	mOwnedQueueType = queueId.GetType();

	B3D_ASSERT(queueId.Id < kMaximumUniqueQueueCount);

	if(useFlags.IsSet(GpuAccessFlag::Read))
	{
		B3D_ASSERT(mReadUses[queueId.Id] < 255 && "Resource used in too many command buffers at once.");
		mReadUses[queueId.Id]++;
	}

	if(useFlags.IsSet(GpuAccessFlag::Write))
	{
		B3D_ASSERT(mWriteUses[queueId.Id] < 255 && "Resource used in too many command buffers at once.");
		mWriteUses[queueId.Id]++;
	}
}

template<class TBase>
void TVulkanResource<TBase>::OnNotifyDone(GpuQueueId queueId, GpuAccessFlags useFlags)
{
	// Called under IGpuResource::mMutex from inside NotifyDone, after the aggregate counters have been decremented.
	if(useFlags.IsSet(GpuAccessFlag::Read))
	{
		B3D_ASSERT(mReadUses[queueId.Id] > 0);
		mReadUses[queueId.Id]--;
	}

	if(useFlags.IsSet(GpuAccessFlag::Write))
	{
		B3D_ASSERT(mWriteUses[queueId.Id] > 0);
		mWriteUses[queueId.Id]--;
	}
}

template<class TBase>
GpuQueueMask TVulkanResource<TBase>::GetUseInfo(GpuAccessFlags useFlags) const
{
	GpuQueueMask mask = 0;

	Lock lock(this->mMutex);

	if(useFlags.IsSet(GpuAccessFlag::Read))
	{
		for(u32 i = 0; i < kMaximumUniqueQueueCount; i++)
		{
			if(mReadUses[i] > 0)
				mask |= GpuQueueId(i);
		}
	}

	if(useFlags.IsSet(GpuAccessFlag::Write))
	{
		for(u32 i = 0; i < kMaximumUniqueQueueCount; i++)
		{
			if(mWriteUses[i] > 0)
				mask |= GpuQueueId(i);
		}
	}

	return mask;
}

template<class TBase>
VulkanGpuDevice& TVulkanResource<TBase>::GetDevice() const
{
	return mOwner->GetDevice();
}

template class TVulkanResource<IGpuResource>;
template class TVulkanResource<IGpuBufferResource>;
template class TVulkanResource<IGpuImageResource>;
template class TVulkanResource<GpuSwapChain>;

VulkanResourceManager::VulkanResourceManager(VulkanGpuDevice& device)
	: GpuResourceManager(device)
{}

VulkanGpuDevice& VulkanResourceManager::GetDevice() const
{
	return static_cast<VulkanGpuDevice&>(mDevice);
}

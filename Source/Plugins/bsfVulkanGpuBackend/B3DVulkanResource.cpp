//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanResource.h"
#include "B3DVulkanGpuCommandBuffer.h"
#include "CoreObject/B3DRenderThread.h"

using namespace b3d;
using namespace b3d::render;

VulkanResource::VulkanResource(VulkanResourceManager* owner, bool concurrency, const StringView& name)
	: IGpuResource(owner, name)
	, mOwner(owner)
	, mState(concurrency ? State::Shared : State::Normal)
{
	B3DZeroOut(mReadUses);
	B3DZeroOut(mWriteUses);
}

void VulkanResource::OnNotifyUsed(GpuQueueId queueId, GpuAccessFlags useFlags)
{
	// Called under IGpuResource::mMutex from inside NotifyUsed, after the aggregate use counter has been incremented.
	// IGpuResource has already incremented mUsedCount, so a value > 1 means there were prior in-flight uses.
	const bool wasInUse = mUsedCount > 1;
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

void VulkanResource::OnNotifyDone(GpuQueueId queueId, GpuAccessFlags useFlags)
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

GpuQueueMask VulkanResource::GetUseInfo(GpuAccessFlags useFlags) const
{
	GpuQueueMask mask = 0;

	Lock lock(mMutex);

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

VulkanGpuDevice& VulkanResource::GetDevice() const
{
	return mOwner->GetDevice();
}

VulkanResourceManager::VulkanResourceManager(VulkanGpuDevice& device)
	: GpuResourceManager(device)
{}

VulkanGpuDevice& VulkanResourceManager::GetDevice() const
{
	return static_cast<VulkanGpuDevice&>(mDevice);
}

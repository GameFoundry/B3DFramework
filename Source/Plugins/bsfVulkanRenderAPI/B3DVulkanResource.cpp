//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DVulkanResource.h"
#include "B3DVulkanGpuCommandBuffer.h"
#include "CoreObject/B3DRenderThread.h"

using namespace b3d;
using namespace b3d::render;

VulkanResource::VulkanResource(VulkanResourceManager* owner, bool concurrency, const StringView& name)
#if B3D_BUILD_TYPE_DEVELOPMENT
	: mDebugName(name)
#endif
{
	Lock lock(mMutex);

	mOwner = owner;
	mState = concurrency ? State::Shared : State::Normal;
	mUsedCount = 0;
	mBountCount = 0;

	B3DZeroOut(mReadUses);
	B3DZeroOut(mWriteUses);
}

VulkanResource::~VulkanResource()
{
	Lock lock(mMutex);
	B3D_ASSERT(mState == State::Destroyed && "Vulkan resource getting destructed without Destroy() called first.");
}

void VulkanResource::NotifyBound()
{
	Lock lock(mMutex);
	B3D_ASSERT(mState != State::Destroyed);

	mBountCount++;
}

void VulkanResource::NotifyUsed(GpuQueueId queueId, GpuAccessFlags useFlags)
{
	Lock lock(mMutex);
	B3D_ASSERT(useFlags != GpuAccessFlag::None);

	bool isUsed = mUsedCount > 0;
	if(isUsed && mState == State::Normal) // Used without support for concurrency
	{
		B3D_ASSERT(mOwnedQueueType == queueId.GetType() && "Vulkan resource without concurrency support can only be used by one queue family at once.");
	}

	mUsedCount++;
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

void VulkanResource::NotifyDone(GpuQueueId queueId, GpuAccessFlags useFlags)
{
	bool destroy;
	{
		Lock lock(mMutex);
		mUsedCount--;
		mBountCount--;

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

		bool isBound = mBountCount > 0;
		destroy = !isBound && mState == State::Destroyed; // Queued for destruction
	}

	// (Safe to check outside of mutex as we guarantee that once queued for destruction, state cannot be changed)
	if(destroy)
		mOwner->Destroy(this);
}

void VulkanResource::NotifyUnbound()
{
	bool destroy;
	{
		Lock lock(mMutex);
		mBountCount--;

		bool isBound = mBountCount > 0;
		destroy = !isBound && mState == State::Destroyed; // Queued for destruction
	}

	// (Safe to check outside of mutex as we guarantee that once queued for destruction, state cannot be changed)
	if(destroy)
		mOwner->Destroy(this);
}

GpuQueueMask VulkanResource::GetUseInfo(GpuAccessFlags useFlags) const
{
	GpuQueueMask mask = 0;

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

void VulkanResource::Destroy()
{
	bool destroy;
	{
		Lock lock(mMutex);
		B3D_ASSERT(mState != State::Destroyed && "Vulkan resource Destroy() called more than once.");

		mState = State::Destroyed;

		// If not bound anyhwere, destroy right away, otherwise check when it is reported as finished on the device
		bool isBound = mBountCount > 0;
		destroy = !isBound;
	}

	// (Safe to check outside of mutex as we guarantee that once queued for destruction, state cannot be changed)
	if(destroy)
		mOwner->Destroy(this);
}

VulkanGpuDevice& VulkanResource::GetDevice() const
{
	return mOwner->GetDevice();
}

VulkanResourceManager::VulkanResourceManager(VulkanGpuDevice& device)
	: mDevice(device)
{}

VulkanResourceManager::~VulkanResourceManager()
{
#if B3D_DEBUG
	Lock lock(mMutex);
	B3D_ASSERT(mResources.empty() && "Resource manager shutting down but not all resources were released.");
#endif
}

void VulkanResourceManager::Destroy(VulkanResource* resource)
{
#if B3D_DEBUG
	{
		Lock lock(mMutex);
		mResources.erase(resource);
	}
#endif

	B3DDelete(resource);
}

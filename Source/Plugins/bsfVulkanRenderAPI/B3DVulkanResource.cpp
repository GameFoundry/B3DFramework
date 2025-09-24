//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanResource.h"
#include "BsVulkanGpuCommandBuffer.h"
#include "CoreObject/BsRenderThread.h"

using namespace b3d;
using namespace b3d::render;

VulkanResource::VulkanResource(VulkanResourceManager* owner, bool concurrency, const StringView& name)
#if B3D_BUILD_TYPE == B3D_BUILD_TYPE_DEVELOPMENT
	: mDebugName(name)
#endif
{
	Lock lock(mMutex);

	mOwner = owner;
	mState = concurrency ? State::Shared : State::Normal;
	mNumUsedHandles = 0;
	mNumBoundHandles = 0;

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

	mNumBoundHandles++;
}

void VulkanResource::NotifyUsed(u32 globalQueueIdx, VulkanAccessFlags useFlags)
{
	Lock lock(mMutex);
	B3D_ASSERT(useFlags != VulkanAccessFlag::None);

	GpuQueueUsage queueUsage;
	CommandSyncMask::GetQueueIdxAndType(globalQueueIdx, queueUsage);

	bool isUsed = mNumUsedHandles > 0;
	if(isUsed && mState == State::Normal) // Used without support for concurrency
	{
		B3D_ASSERT(mQueueUsage == queueUsage && "Vulkan resource without concurrency support can only be used by one queue family at once.");
	}

	mNumUsedHandles++;
	mQueueUsage = queueUsage;

	B3D_ASSERT(globalQueueIdx < kMaximumUniqueQueueCount);

	if(useFlags.IsSet(VulkanAccessFlag::Read))
	{
		B3D_ASSERT(mReadUses[globalQueueIdx] < 255 && "Resource used in too many command buffers at once.");
		mReadUses[globalQueueIdx]++;
	}

	if(useFlags.IsSet(VulkanAccessFlag::Write))
	{
		B3D_ASSERT(mWriteUses[globalQueueIdx] < 255 && "Resource used in too many command buffers at once.");
		mWriteUses[globalQueueIdx]++;
	}
}

void VulkanResource::NotifyDone(u32 globalQueueIdx, VulkanAccessFlags useFlags)
{
	bool destroy;
	{
		Lock lock(mMutex);
		mNumUsedHandles--;
		mNumBoundHandles--;

		if(useFlags.IsSet(VulkanAccessFlag::Read))
		{
			B3D_ASSERT(mReadUses[globalQueueIdx] > 0);
			mReadUses[globalQueueIdx]--;
		}

		if(useFlags.IsSet(VulkanAccessFlag::Write))
		{
			B3D_ASSERT(mWriteUses[globalQueueIdx] > 0);
			mWriteUses[globalQueueIdx]--;
		}

		bool isBound = mNumBoundHandles > 0;
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
		mNumBoundHandles--;

		bool isBound = mNumBoundHandles > 0;
		destroy = !isBound && mState == State::Destroyed; // Queued for destruction
	}

	// (Safe to check outside of mutex as we guarantee that once queued for destruction, state cannot be changed)
	if(destroy)
		mOwner->Destroy(this);
}

u32 VulkanResource::GetUseInfo(VulkanAccessFlags useFlags) const
{
	u32 mask = 0;

	if(useFlags.IsSet(VulkanAccessFlag::Read))
	{
		for(u32 i = 0; i < kMaximumUniqueQueueCount; i++)
		{
			if(mReadUses[i] > 0)
				mask |= 1 << i;
		}
	}

	if(useFlags.IsSet(VulkanAccessFlag::Write))
	{
		for(u32 i = 0; i < kMaximumUniqueQueueCount; i++)
		{
			if(mWriteUses[i] > 0)
				mask |= 1 << i;
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
		bool isBound = mNumBoundHandles > 0;
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

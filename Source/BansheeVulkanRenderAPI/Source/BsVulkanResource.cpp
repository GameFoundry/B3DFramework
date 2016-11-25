//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanResource.h"
#include "BsVulkanCommandBuffer.h"
#include "BsCoreThread.h"

namespace bs
{
	VulkanResource::VulkanResource(VulkanResourceManager* owner, bool concurrency)
	{
		Lock lock(mMutex);

		mOwner = owner;
		mQueueFamily = -1;
		mState = concurrency ? State::Shared : State::Normal;
		mNumUsedHandles = 0;
		mNumBoundHandles = 0;

		bs_zero_out(mReadUses);
		bs_zero_out(mWriteUses);
	}

	VulkanResource::~VulkanResource()
	{
		THROW_IF_NOT_CORE_THREAD

		Lock lock(mMutex);
		assert(mState == State::Destroyed && "Vulkan resource getting destructed without destroy() called first.");
	}

	void VulkanResource::notifyBound()
	{
		Lock lock(mMutex);
		assert(mState != State::Destroyed);

		mNumBoundHandles++;
	}

	void VulkanResource::notifyUsed(UINT32 globalQueueIdx, UINT32 queueFamily, VulkanUseFlags useFlags)
	{
		Lock lock(mMutex);
		assert(mState != State::Destroyed);
		assert(useFlags != VulkanUseFlag::None);

		if(isUsed() && mState == State::Normal) // Used without support for concurrency
		{
			assert(mQueueFamily == queueFamily &&
				"Vulkan resource without concurrency support can only be used by one queue family at once.");
		}

		mNumUsedHandles++;
		mQueueFamily = queueFamily;

		assert(globalQueueIdx < MAX_UNIQUE_QUEUES);
		
		if (useFlags.isSet(VulkanUseFlag::Read))
		{
			assert(mReadUses[globalQueueIdx] < 255 && "Resource used in too many command buffers at once.");
			mReadUses[globalQueueIdx]++;
		}
		
		if(useFlags.isSet(VulkanUseFlag::Write))
		{
			assert(mWriteUses[globalQueueIdx] < 255 && "Resource used in too many command buffers at once.");
			mWriteUses[globalQueueIdx]++;
		}
	}

	void VulkanResource::notifyDone(UINT32 globalQueueIdx, VulkanUseFlags useFlags)
	{
		Lock lock(mMutex);
		mNumUsedHandles--;
		mNumBoundHandles--;

		if (useFlags.isSet(VulkanUseFlag::Read))
		{
			assert(mReadUses[globalQueueIdx] > 0);
			mReadUses[globalQueueIdx]--;
		}

		if (useFlags.isSet(VulkanUseFlag::Write))
		{
			assert(mWriteUses[globalQueueIdx] > 0);
			mWriteUses[globalQueueIdx]--;
		}

		if (!isBound() && mState == State::Destroyed) // Queued for destruction
			mOwner->destroy(this);
	}

	void VulkanResource::notifyUnbound()
	{
		Lock lock(mMutex);
		mNumBoundHandles--;

		if (!isBound() && mState == State::Destroyed) // Queued for destruction
			mOwner->destroy(this);
	}

	UINT32 VulkanResource::getUseInfo(VulkanUseFlags useFlags) const
	{
		UINT32 mask = 0;

		if(useFlags.isSet(VulkanUseFlag::Read))
		{
			for (UINT32 i = 0; i < MAX_UNIQUE_QUEUES; i++)
			{
				if (mReadUses[i] > 0)
					mask |= 1 << i;
			}
		}

		if (useFlags.isSet(VulkanUseFlag::Write))
		{
			for (UINT32 i = 0; i < MAX_UNIQUE_QUEUES; i++)
			{
				if (mWriteUses[i] > 0)
					mask |= 1 << i;
			}
		}

		return mask;
	}

	void VulkanResource::destroy()
	{
		Lock lock(mMutex);
		assert(mState != State::Destroyed && "Vulkan resource destroy() called more than once.");

		mState = State::Destroyed;

		// If not bound anyhwere, destroy right away, otherwise check when it is reported as finished on the device
		if (!isBound())
			mOwner->destroy(this);
	}

	VulkanResourceManager::VulkanResourceManager(VulkanDevice& device)
		:mDevice(device)
	{ }

	VulkanResourceManager::~VulkanResourceManager()
	{
#if BS_DEBUG_MODE
		Lock lock(mMutex);
		assert(mResources.empty() && "Resource manager shutting down but not all resources were released.");
#endif
	}

	void VulkanResourceManager::destroy(VulkanResource* resource)
	{
#if BS_DEBUG_MODE
		{
			Lock lock(mMutex);
			mResources.erase(resource);
		}
#endif

		bs_delete(resource);
	}
}

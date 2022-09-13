//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanResource.h"
#include "BsVulkanCommandBuffer.h"
#include "CoreThread/BsCoreThread.h"

namespace bs { namespace ct
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

	void VulkanResource::NotifyBound()
	{
		Lock lock(mMutex);
		assert(mState != State::Destroyed);

		mNumBoundHandles++;
	}

	void VulkanResource::NotifyUsed(UINT32 globalQueueIdx, UINT32 queueFamily, VulkanAccessFlags useFlags)
	{
		Lock lock(mMutex);
		assert(useFlags != VulkanAccessFlag::None);

		bool isUsed = mNumUsedHandles > 0;
		if(isUsed && mState == State::Normal) // Used without support for concurrency
		{
			assert(mQueueFamily == queueFamily &&
				"Vulkan resource without concurrency support can only be used by one queue family at once.");
		}

		mNumUsedHandles++;
		mQueueFamily = queueFamily;

		assert(globalQueueIdx < MAX_UNIQUE_QUEUES);
		
		if (useFlags.IsSet(VulkanAccessFlag::Read))
		{
			assert(mReadUses[globalQueueIdx] < 255 && "Resource used in too many command buffers at once.");
			mReadUses[globalQueueIdx]++;
		}
		
		if(useFlags.IsSet(VulkanAccessFlag::Write))
		{
			assert(mWriteUses[globalQueueIdx] < 255 && "Resource used in too many command buffers at once.");
			mWriteUses[globalQueueIdx]++;
		}
	}

	void VulkanResource::NotifyDone(UINT32 globalQueueIdx, VulkanAccessFlags useFlags)
	{
		bool destroy;
		{
			Lock lock(mMutex);
			mNumUsedHandles--;
			mNumBoundHandles--;

			if (useFlags.IsSet(VulkanAccessFlag::Read))
			{
				assert(mReadUses[globalQueueIdx] > 0);
				mReadUses[globalQueueIdx]--;
			}

			if (useFlags.IsSet(VulkanAccessFlag::Write))
			{
				assert(mWriteUses[globalQueueIdx] > 0);
				mWriteUses[globalQueueIdx]--;
			}

			bool isBound = mNumBoundHandles > 0;
			destroy = !isBound && mState == State::Destroyed; // Queued for destruction
		}

		// (Safe to check outside of mutex as we guarantee that once queued for destruction, state cannot be changed)
		if (destroy)
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
		if (destroy)
			mOwner->Destroy(this);
	}

	UINT32 VulkanResource::GetUseInfo(VulkanAccessFlags useFlags) const
	{
		UINT32 mask = 0;

		if(useFlags.IsSet(VulkanAccessFlag::Read))
		{
			for (UINT32 i = 0; i < MAX_UNIQUE_QUEUES; i++)
			{
				if (mReadUses[i] > 0)
					mask |= 1 << i;
			}
		}

		if (useFlags.IsSet(VulkanAccessFlag::Write))
		{
			for (UINT32 i = 0; i < MAX_UNIQUE_QUEUES; i++)
			{
				if (mWriteUses[i] > 0)
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
			assert(mState != State::Destroyed && "Vulkan resource destroy() called more than once.");

			mState = State::Destroyed;

			// If not bound anyhwere, destroy right away, otherwise check when it is reported as finished on the device
			bool isBound = mNumBoundHandles > 0;
			destroy = !isBound;
		}

		// (Safe to check outside of mutex as we guarantee that once queued for destruction, state cannot be changed)
		if (destroy)
			mOwner->Destroy(this);
	}

	VulkanDevice& VulkanResource::GetDevice() const
	{
		return mOwner->GetDevice();
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

	void VulkanResourceManager::Destroy(VulkanResource* resource)
	{
#if BS_DEBUG_MODE
		{
			Lock lock(mMutex);
			mResources.erase(resource);
		}
#endif

		bs_delete(resource);
	}
}}

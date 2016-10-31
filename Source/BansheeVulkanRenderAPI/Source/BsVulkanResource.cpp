//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanResource.h"
#include "BsVulkanCommandBuffer.h"

namespace BansheeEngine
{
	VulkanResource::VulkanResource(VulkanResourceManager* owner, bool concurrency)
		: mOwner(owner), mState(concurrency ? State::Shared : State::Normal), mNumHandles(0)
		, mHandleCapacity(INITIAL_HANDLE_CAPACITY)
	{
		UINT32 bytesCapacity = sizeof(UseHandle) * mHandleCapacity;
		mHandles = (UseHandle*)mAlloc.alloc(sizeof(UseHandle) * INITIAL_HANDLE_CAPACITY);
		memset(mHandles, 0, bytesCapacity);
	}

	VulkanResource::~VulkanResource()
	{
		assert(mState == State::Destroyed && "Vulkan resource getting destructed without destroy() called first.");

		mAlloc.free(mHandles);
		mAlloc.clear();
	}

	void VulkanResource::notifyUsed(VulkanCmdBuffer* buffer, VulkanUseFlags flags)
	{
		assert(mState != State::Destroyed);

		if(isUsed() && mState == State::Normal) // Used without support for concurrency
		{
			assert(mQueueFamily == buffer->getQueueFamily() && 
				"Vulkan resource without concurrency support can only be used by one queue family at once.");
		}

		mNumHandles++;

		if(mNumHandles > mHandleCapacity)
		{
			UINT32 bytesCapacity = sizeof(UseHandle) * mHandleCapacity;
			void* tempData = bs_stack_alloc(bytesCapacity);
			memcpy(tempData, mHandles, bytesCapacity);

			mAlloc.free(mHandles);
			mHandles = (UseHandle*)mAlloc.alloc(bytesCapacity * 2);
			memcpy(mHandles, tempData, bytesCapacity);

			bs_stack_free(tempData);

			memset(mHandles + mHandleCapacity, 0, bytesCapacity);
			mHandleCapacity *= 2;
		}

		for(UINT32 i = 0; i < mHandleCapacity; i++)
		{
			if (mHandles[i].buffer == nullptr)
				continue;

			mHandles[i].buffer = buffer;
			mHandles[i].flags |= flags;

			break;
		}

		mQueueFamily = buffer->getQueueFamily();
	}

	void VulkanResource::notifyDone(VulkanCmdBuffer* buffer)
	{
		bool foundBuffer = false;
		for(UINT32 i = 0; i < mNumHandles; i++)
		{
			if(mHandles[i].buffer == buffer)
			{
				mHandles[i].buffer = nullptr;
				mHandles[i].flags = VulkanUseFlag::None;

				foundBuffer = true;
				break;
			}
		}

		assert(foundBuffer);

		mNumHandles--;

		if (!isUsed() && mState == State::Destroyed) // Queued for destruction
			mOwner->destroy(this);
	}

	void VulkanResource::destroy()
	{
		assert(mState != State::Destroyed && "Vulkan resource destroy() called more than once.");

		mState = State::Destroyed;

		// If not used, destroy right away, otherwise check when it is reported as finished on the device
		if (!isUsed())
			mOwner->destroy(this);
	}

	VulkanResourceManager::~VulkanResourceManager()
	{
#if BS_DEBUG_MODE
		assert(mResources.empty() && "Resource manager shutting down but not all resources were released.");
#endif
	}

	void VulkanResourceManager::destroy(VulkanResource* resource)
	{
#if BS_DEBUG_MODE
		mResources.erase(resource);
#endif

		bs_delete(resource);
	}
}
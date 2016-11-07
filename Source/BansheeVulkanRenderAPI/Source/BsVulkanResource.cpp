//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanResource.h"
#include "BsVulkanCommandBuffer.h"
#include "BsCoreThread.h"

namespace BansheeEngine
{
	VulkanResource::VulkanResource(VulkanResourceManager* owner, bool concurrency)
	{
		Lock lock(mMutex);

		mOwner = owner;
		mQueueFamily = -1;
		mState = concurrency ? State::Shared : State::Normal;
		mNumUsedHandles = 0;
		mNumBoundHandles = 0;
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

	void VulkanResource::notifyUsed(VulkanCmdBuffer* buffer, VulkanUseFlags useFlags)
	{
		Lock lock(mMutex);
		assert(mState != State::Destroyed);

		if(isUsed() && mState == State::Normal) // Used without support for concurrency
		{
			assert(mQueueFamily == buffer->getQueueFamily() && 
				"Vulkan resource without concurrency support can only be used by one queue family at once.");
		}

		mNumUsedHandles++;
		mQueueFamily = buffer->getQueueFamily();
		mUseFlags |= useFlags;
	}

	void VulkanResource::notifyDone()
	{
		Lock lock(mMutex);
		mNumUsedHandles--;
		mNumBoundHandles--;

		// Note: If resource is used on different command buffers with different use flags, we should clear individual flags
		// depending on which command buffer finished. But this requires extra per-command buffer state tracking, so we
		// instead just clear all flags at once when all command buffers finish.
		if (!isUsed())
			mUseFlags = VulkanUseFlag::None;

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
		Lock lock(mMutex);
		mResources.erase(resource);
#endif

		bs_delete(resource);
	}
}

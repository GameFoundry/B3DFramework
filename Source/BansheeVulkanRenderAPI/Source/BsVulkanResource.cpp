//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanResource.h"
#include "BsVulkanCommandBuffer.h"

namespace BansheeEngine
{
	VulkanResource::VulkanResource(VulkanResourceManager* owner)
		:mOwner(owner)
	{
		
	}

	VulkanResource::~VulkanResource()
	{
		assert(mIsDestroyed && "Vulkan resource getting destructed without destroy() called first.");
	}

	void VulkanResource::notifyUsed(VulkanCmdBuffer& buffer, VulkanUseFlags flags)
	{
		assert(!isUsed() && !mIsDestroyed);

		// Note: I could allow resource usage on multiple command buffers at once by keeping a list of separate usage
		// flags and ID's, per buffer.

		mCmdBufferId = buffer.getId();
		mFlags |= flags;
	}

	void VulkanResource::notifyDone()
	{
		mCmdBufferId = -1;
		mFlags = VulkanUseFlag::None;

		if (mIsDestroyed) // Queued for destruction
			mOwner->destroy(this);
	}

	void VulkanResource::destroy()
	{
		assert(!mIsDestroyed && "Vulkan resource destroy() called more than once.");

		mIsDestroyed = true;

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
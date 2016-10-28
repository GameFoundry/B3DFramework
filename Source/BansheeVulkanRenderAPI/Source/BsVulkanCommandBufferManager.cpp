//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanCommandBufferManager.h"
#include "BsVulkanCommandBuffer.h"
#include "BsVulkanRenderAPI.h"

namespace BansheeEngine
{
	VulkanCommandBufferManager::VulkanCommandBufferManager(const VulkanRenderAPI& rapi)
		:mRapi(rapi)
	{ }

	VulkanCommandBufferManager::~VulkanCommandBufferManager()
	{
		
	}

	SPtr<CommandBuffer> VulkanCommandBufferManager::createInternal(UINT32 id, CommandBufferType type, UINT32 deviceIdx, 
		UINT32 queueIdx, bool secondary)
	{
		UINT32 numDevices = mRapi._getNumDevices();
		if(deviceIdx >= numDevices)
		{
			LOGERR("Cannot create command buffer, invalid device index: " + toString(deviceIdx) + 
				". Valid range: [0, " + toString(numDevices) + ").");

			return nullptr;
		}

		SPtr<VulkanDevice> device = mRapi._getDevice(deviceIdx);

		CommandBuffer* buffer = 
			new (bs_alloc<VulkanCommandBuffer>()) VulkanCommandBuffer(*device, id, type, queueIdx, secondary);

		return bs_shared_ptr(buffer);
	}
}
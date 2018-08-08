//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanVertexBuffer.h"
#include "BsVulkanHardwareBuffer.h"
#include "Profiling/BsRenderStats.h"

namespace bs { namespace ct
{
	static void deleteBuffer(HardwareBuffer* buffer)
	{
		bs_pool_delete(static_cast<VulkanHardwareBuffer*>(buffer));
	}

	VulkanVertexBuffer::VulkanVertexBuffer(const VERTEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask)
		:VertexBuffer(desc, deviceMask), mDeviceMask(deviceMask)
	{ }

	void VulkanVertexBuffer::initialize()
	{
		mBuffer = bs_pool_new<VulkanHardwareBuffer>(VulkanHardwareBuffer::BT_VERTEX, BF_UNKNOWN, mUsage, mSize, mDeviceMask);
		mBufferDeleter = &deleteBuffer;

		VertexBuffer::initialize();
	}

	VulkanBuffer* VulkanVertexBuffer::getResource(UINT32 deviceIdx) const
	{
		return static_cast<VulkanHardwareBuffer*>(mBuffer)->getResource(deviceIdx);
	}
}}
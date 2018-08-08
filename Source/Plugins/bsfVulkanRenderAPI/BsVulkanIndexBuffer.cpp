//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanIndexBuffer.h"
#include "BsVulkanHardwareBuffer.h"
#include "Profiling/BsRenderStats.h"

namespace bs { namespace ct
{
	static void deleteBuffer(HardwareBuffer* buffer)
	{
		bs_pool_delete(static_cast<VulkanHardwareBuffer*>(buffer));
	}

	VulkanIndexBuffer::VulkanIndexBuffer(const INDEX_BUFFER_DESC& desc, GpuDeviceFlags deviceMask)
		:IndexBuffer(desc, deviceMask),  mDeviceMask(deviceMask)
	{ }

	void VulkanIndexBuffer::initialize()
	{
		mBuffer = bs_pool_new<VulkanHardwareBuffer>(VulkanHardwareBuffer::BT_INDEX, BF_UNKNOWN, mUsage, mSize, mDeviceMask);
		mBufferDeleter = &deleteBuffer;

		IndexBuffer::initialize();
	}

	VulkanBuffer* VulkanIndexBuffer::getResource(UINT32 deviceIdx) const
	{
		return static_cast<VulkanHardwareBuffer*>(mBuffer)->getResource(deviceIdx);
	}
}}
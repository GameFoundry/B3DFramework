//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanGpuParamBlockBuffer.h"
#include "BsVulkanHardwareBuffer.h"
#include "Profiling/BsRenderStats.h"

namespace bs { namespace ct
{
	VulkanGpuParamBlockBuffer::VulkanGpuParamBlockBuffer(UINT32 size, GpuBufferUsage usage, GpuDeviceFlags deviceMask)
		:GpuParamBlockBuffer(size, usage, deviceMask), mDeviceMask(deviceMask)
	{ }

	VulkanGpuParamBlockBuffer::~VulkanGpuParamBlockBuffer()
	{
		if(mBuffer != nullptr)
			bs_pool_delete(static_cast<VulkanHardwareBuffer*>(mBuffer));
	}

	void VulkanGpuParamBlockBuffer::initialize()
	{
		mBuffer = bs_pool_new<VulkanHardwareBuffer>(VulkanHardwareBuffer::BT_UNIFORM, BF_UNKNOWN, mUsage, mSize, mDeviceMask);

		GpuParamBlockBuffer::initialize();
	}

	VulkanBuffer* VulkanGpuParamBlockBuffer::getResource(UINT32 deviceIdx) const
	{
		return static_cast<VulkanHardwareBuffer*>(mBuffer)->getResource(deviceIdx);
	}
}}
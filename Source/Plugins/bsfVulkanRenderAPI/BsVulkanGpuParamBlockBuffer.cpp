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
			bs_pool_delete(mBuffer);

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_GpuParamBuffer);
	}

	void VulkanGpuParamBlockBuffer::initialize()
	{
		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_GpuParamBuffer);

		mBuffer = bs_pool_new<VulkanHardwareBuffer>(VulkanHardwareBuffer::BT_UNIFORM, BF_UNKNOWN, mUsage, mSize, mDeviceMask);

		GpuParamBlockBuffer::initialize();
	}

	void VulkanGpuParamBlockBuffer::writeToGPU(const UINT8* data, UINT32 queueIdx)
	{
		mBuffer->writeData(0, mSize, data, BWT_DISCARD, queueIdx);

		BS_INC_RENDER_STAT_CAT(ResWrite, RenderStatObject_GpuParamBuffer);
	}

	VulkanBuffer* VulkanGpuParamBlockBuffer::getResource(UINT32 deviceIdx) const
	{
		return mBuffer->getResource(deviceIdx);
	}
}}
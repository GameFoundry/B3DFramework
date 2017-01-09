//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanGpuParamBlockBuffer.h"
#include "BsVulkanHardwareBuffer.h"
#include "BsRenderStats.h"

namespace bs { namespace ct
{
	VulkanGpuParamBlockBuffer::VulkanGpuParamBlockBuffer(UINT32 size, GpuParamBlockUsage usage,
		GpuDeviceFlags deviceMask)
		:GpuParamBlockBufferCore(size, usage, deviceMask), mBuffer(nullptr), mDeviceMask(deviceMask)
	{ }

	VulkanGpuParamBlockBuffer::~VulkanGpuParamBlockBuffer()
	{
		if(mBuffer != nullptr)
			bs_delete(mBuffer);

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_GpuParamBuffer);
	}

	void VulkanGpuParamBlockBuffer::initialize()
	{
		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_GpuParamBuffer);

		GpuBufferUsage usage = mUsage == GPBU_STATIC ? GBU_STATIC : GBU_DYNAMIC;

		mBuffer = bs_new<VulkanHardwareBuffer>(VulkanHardwareBuffer::BT_UNIFORM, BF_UNKNOWN, usage, mSize, mDeviceMask);

		GpuParamBlockBufferCore::initialize();
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
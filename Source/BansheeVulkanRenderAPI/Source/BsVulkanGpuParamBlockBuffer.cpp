//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanGpuParamBlockBuffer.h"
#include "BsRenderStats.h"

namespace BansheeEngine
{
	VulkanGpuParamBlockBufferCore::VulkanGpuParamBlockBufferCore(UINT32 size, GpuParamBlockUsage usage,
		GpuDeviceFlags deviceMask)
		:GpuParamBlockBufferCore(size, usage, deviceMask)
	{ }

	VulkanGpuParamBlockBufferCore::~VulkanGpuParamBlockBufferCore()
	{
		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_GpuParamBuffer);
	}

	void VulkanGpuParamBlockBufferCore::initialize()
	{
		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_GpuParamBuffer);

		GpuParamBlockBufferCore::initialize();
	}

	void VulkanGpuParamBlockBufferCore::writeToGPU(const UINT8* data)
	{
		BS_INC_RENDER_STAT_CAT(ResWrite, RenderStatObject_GpuParamBuffer);
	}

	void VulkanGpuParamBlockBufferCore::readFromGPU(UINT8* data) const
	{
		BS_INC_RENDER_STAT_CAT(ResRead, RenderStatObject_GpuParamBuffer);
	}
}
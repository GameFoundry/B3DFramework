//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanGpuPipelineState.h"
#include "BsRenderStats.h"

namespace BansheeEngine
{
	VulkanGpuPipelineStateCore::VulkanGpuPipelineStateCore(const PIPELINE_STATE_CORE_DESC& desc, GpuDeviceFlags deviceMask)
		:GpuPipelineStateCore(desc, deviceMask)
	{
		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_PipelineState);
	}

	VulkanGpuPipelineStateCore::~VulkanGpuPipelineStateCore()
	{
		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_PipelineState);
	}
}
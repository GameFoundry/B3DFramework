//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanRenderStateManager.h"
#include "BsVulkanGpuPipelineState.h"

namespace BansheeEngine
{
	SPtr<GpuPipelineStateCore> VulkanRenderStateCoreManager::_createPipelineState(const PIPELINE_STATE_CORE_DESC& desc,
		GpuDeviceFlags deviceMask) const
	{
		SPtr<VulkanGpuPipelineStateCore> pipelineState =
			bs_shared_ptr<VulkanGpuPipelineStateCore>(new (bs_alloc<VulkanGpuPipelineStateCore>()) VulkanGpuPipelineStateCore(desc, deviceMask));
		pipelineState->_setThisPtr(pipelineState);

		return pipelineState;
	}
}
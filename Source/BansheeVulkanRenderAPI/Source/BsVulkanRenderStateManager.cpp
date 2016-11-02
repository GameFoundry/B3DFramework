//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanRenderStateManager.h"
#include "BsVulkanGpuPipelineState.h"
#include "BsVulkanSamplerState.h"

namespace BansheeEngine
{
	SPtr<SamplerStateCore> VulkanRenderStateCoreManager::createSamplerStateInternal(const SAMPLER_STATE_DESC& desc,
		GpuDeviceFlags deviceMask) const
	{
		SPtr<VulkanSamplerStateCore> samplerState =
			bs_shared_ptr<VulkanSamplerStateCore>(new (bs_alloc<VulkanSamplerStateCore>()) VulkanSamplerStateCore(desc, deviceMask));
		samplerState->_setThisPtr(samplerState);

		return samplerState;
	}

	SPtr<GpuPipelineStateCore> VulkanRenderStateCoreManager::_createPipelineState(const PIPELINE_STATE_CORE_DESC& desc,
		GpuDeviceFlags deviceMask) const
	{
		SPtr<VulkanGpuPipelineStateCore> pipelineState =
			bs_shared_ptr<VulkanGpuPipelineStateCore>(new (bs_alloc<VulkanGpuPipelineStateCore>()) VulkanGpuPipelineStateCore(desc, deviceMask));
		pipelineState->_setThisPtr(pipelineState);

		return pipelineState;
	}
}
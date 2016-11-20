//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanRenderStateManager.h"
#include "BsVulkanGpuPipelineState.h"
#include "BsVulkanGpuPipelineParamInfo.h"
#include "BsVulkanSamplerState.h"

namespace bs
{
	SPtr<SamplerStateCore> VulkanRenderStateCoreManager::createSamplerStateInternal(const SAMPLER_STATE_DESC& desc,
		GpuDeviceFlags deviceMask) const
	{
		SPtr<VulkanSamplerStateCore> samplerState =
			bs_shared_ptr<VulkanSamplerStateCore>(new (bs_alloc<VulkanSamplerStateCore>()) VulkanSamplerStateCore(desc, deviceMask));
		samplerState->_setThisPtr(samplerState);

		return samplerState;
	}

	SPtr<GraphicsPipelineStateCore> VulkanRenderStateCoreManager::_createGraphicsPipelineState(
		const PIPELINE_STATE_CORE_DESC& desc, GpuDeviceFlags deviceMask) const
	{
		SPtr<VulkanGraphicsPipelineStateCore> pipelineState =
			bs_shared_ptr<VulkanGraphicsPipelineStateCore>(new (bs_alloc<VulkanGraphicsPipelineStateCore>()) 
			VulkanGraphicsPipelineStateCore(desc, deviceMask));
		pipelineState->_setThisPtr(pipelineState);

		return pipelineState;
	}

	SPtr<ComputePipelineStateCore> VulkanRenderStateCoreManager::_createComputePipelineState(
		const SPtr<GpuProgramCore>& program, GpuDeviceFlags deviceMask) const
	{
		SPtr<VulkanComputePipelineStateCore> pipelineState =
			bs_shared_ptr<VulkanComputePipelineStateCore>(new (bs_alloc<VulkanComputePipelineStateCore>())
														  VulkanComputePipelineStateCore(program, deviceMask));
		pipelineState->_setThisPtr(pipelineState);

		return pipelineState;
	}

	SPtr<GpuPipelineParamInfoCore> VulkanRenderStateCoreManager::_createPipelineParamInfo(
		const GPU_PIPELINE_PARAMS_DESC& desc, GpuDeviceFlags deviceMask) const
	{
		SPtr<VulkanGpuPipelineParamInfo> paramInfo =
			bs_shared_ptr<VulkanGpuPipelineParamInfo>(new (bs_alloc<VulkanGpuPipelineParamInfo>())
													  VulkanGpuPipelineParamInfo(desc, deviceMask));
		paramInfo->_setThisPtr(paramInfo);

		return paramInfo;
	}
}
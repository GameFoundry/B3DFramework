//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanRenderStateManager.h"
#include "BsVulkanGpuPipelineState.h"
#include "BsVulkanGpuPipelineParamInfo.h"
#include "BsVulkanSamplerState.h"

namespace bs { namespace ct
{
	SPtr<SamplerStateCore> VulkanRenderStateManager::createSamplerStateInternal(const SAMPLER_STATE_DESC& desc,
		GpuDeviceFlags deviceMask) const
	{
		SPtr<VulkanSamplerState> samplerState =
			bs_shared_ptr<VulkanSamplerState>(new (bs_alloc<VulkanSamplerState>()) VulkanSamplerState(desc, deviceMask));
		samplerState->_setThisPtr(samplerState);

		return samplerState;
	}

	SPtr<GraphicsPipelineStateCore> VulkanRenderStateManager::_createGraphicsPipelineState(
		const PIPELINE_STATE_DESC& desc, GpuDeviceFlags deviceMask) const
	{
		SPtr<VulkanGraphicsPipelineState> pipelineState =
			bs_shared_ptr<VulkanGraphicsPipelineState>(new (bs_alloc<VulkanGraphicsPipelineState>()) 
			VulkanGraphicsPipelineState(desc, deviceMask));
		pipelineState->_setThisPtr(pipelineState);

		return pipelineState;
	}

	SPtr<ComputePipelineStateCore> VulkanRenderStateManager::_createComputePipelineState(
		const SPtr<GpuProgramCore>& program, GpuDeviceFlags deviceMask) const
	{
		SPtr<VulkanComputePipelineState> pipelineState =
			bs_shared_ptr<VulkanComputePipelineState>(new (bs_alloc<VulkanComputePipelineState>())
														  VulkanComputePipelineState(program, deviceMask));
		pipelineState->_setThisPtr(pipelineState);

		return pipelineState;
	}

	SPtr<GpuPipelineParamInfoCore> VulkanRenderStateManager::_createPipelineParamInfo(
		const GPU_PIPELINE_PARAMS_DESC& desc, GpuDeviceFlags deviceMask) const
	{
		SPtr<VulkanGpuPipelineParamInfo> paramInfo =
			bs_shared_ptr<VulkanGpuPipelineParamInfo>(new (bs_alloc<VulkanGpuPipelineParamInfo>())
													  VulkanGpuPipelineParamInfo(desc, deviceMask));
		paramInfo->_setThisPtr(paramInfo);

		return paramInfo;
	}
}}
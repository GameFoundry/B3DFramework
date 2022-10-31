//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/BsVulkanRenderStateManager.h"
#include "BsVulkanGpuPipelineState.h"
#include "BsVulkanGpuPipelineParamInfo.h"
#include "BsVulkanSamplerState.h"

using namespace bs;
using namespace bs::ct;

SPtr<ct::SamplerState> VulkanRenderStateManager::CreateSamplerStateInternalInternal(const SAMPLER_STATE_DESC& desc, GpuDeviceFlags deviceMask) const
{
	SPtr<VulkanSamplerState> samplerState =
		B3DMakeSharedFromExisting<VulkanSamplerState>(new(B3DAllocate<VulkanSamplerState>()) VulkanSamplerState(desc, deviceMask));
	samplerState->SetThisPtrInternal(samplerState);

	return samplerState;
}

SPtr<ct::GraphicsPipelineState> VulkanRenderStateManager::CreateGraphicsPipelineStateInternal(
	const PIPELINE_STATE_DESC& desc, GpuDeviceFlags deviceMask) const
{
	SPtr<VulkanGraphicsPipelineState> pipelineState =
		B3DMakeSharedFromExisting<VulkanGraphicsPipelineState>(new(B3DAllocate<VulkanGraphicsPipelineState>())
													   VulkanGraphicsPipelineState(desc, deviceMask));
	pipelineState->SetThisPtrInternal(pipelineState);

	return pipelineState;
}

SPtr<ct::ComputePipelineState> VulkanRenderStateManager::CreateComputePipelineStateInternal(
	const SPtr<GpuProgram>& program, GpuDeviceFlags deviceMask) const
{
	SPtr<VulkanComputePipelineState> pipelineState =
		B3DMakeSharedFromExisting<VulkanComputePipelineState>(new(B3DAllocate<VulkanComputePipelineState>())
													  VulkanComputePipelineState(program, deviceMask));
	pipelineState->SetThisPtrInternal(pipelineState);

	return pipelineState;
}

SPtr<ct::GpuPipelineParamInfo> VulkanRenderStateManager::CreatePipelineParamInfoInternal(
	const GPU_PIPELINE_PARAMS_DESC& desc, GpuDeviceFlags deviceMask) const
{
	SPtr<VulkanGpuPipelineParamInfo> paramInfo =
		B3DMakeSharedFromExisting<VulkanGpuPipelineParamInfo>(new(B3DAllocate<VulkanGpuPipelineParamInfo>())
													  VulkanGpuPipelineParamInfo(desc, deviceMask));
	paramInfo->SetThisPtrInternal(paramInfo);

	return paramInfo;
}

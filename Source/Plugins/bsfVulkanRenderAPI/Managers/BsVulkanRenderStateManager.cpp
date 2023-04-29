//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/BsVulkanRenderStateManager.h"

#include "BsCoreApplication.h"
#include "BsVulkanGpuPipelineState.h"
#include "BsVulkanGpuPipelineParameterLayout.h"
#include "BsVulkanSamplerState.h"
#include "BsVulkanGpuDevice.h"

using namespace bs;
using namespace bs::ct;

SPtr<ct::SamplerState> VulkanRenderStateManager::CreateSamplerStateInternalInternal(const SamplerStateCreateInformation& desc) const
{
	const SPtr<GpuDevice> gpuDevice = GetCoreApplication().GetPrimaryGpuDevice();
	if (!gpuDevice)
		return nullptr;

	SPtr<VulkanSamplerState> samplerState =
		B3DMakeSharedFromExisting<VulkanSamplerState>(new(B3DAllocate<VulkanSamplerState>()) VulkanSamplerState(static_cast<VulkanGpuDevice&>(*gpuDevice), desc));
	samplerState->SetShared(samplerState);

	return samplerState;
}

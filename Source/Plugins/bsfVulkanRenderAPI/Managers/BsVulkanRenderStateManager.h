//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "Managers/BsRenderStateManager.h"

namespace bs { namespace ct
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/**	Handles creation of Vulkan pipeline states. */
	class VulkanRenderStateManager : public RenderStateManager
	{
	protected:
		/** @copydoc RenderStateManager::createSamplerStateInternal */
		SPtr<SamplerState> CreateSamplerStateInternalInternal(const SAMPLER_STATE_DESC& desc,
			GpuDeviceFlags deviceMask) const ;

		/** @copydoc RenderStateManager::CreateGraphicsPipelineStateInternal */
		SPtr<GraphicsPipelineState> CreateGraphicsPipelineStateInternal(const PIPELINE_STATE_DESC& desc,
			GpuDeviceFlags deviceMask = GDF_DEFAULT) const override;

		/** @copydoc RenderStateManager::CreateComputePipelineStateInternal */
		SPtr<ComputePipelineState> CreateComputePipelineStateInternal(const SPtr<GpuProgram>& program,
			GpuDeviceFlags deviceMask = GDF_DEFAULT) const override;

		/** @copydoc RenderStateManager::CreatePipelineParamInfoInternal */
		SPtr<GpuPipelineParamInfo> CreatePipelineParamInfoInternal(const GPU_PIPELINE_PARAMS_DESC& desc,
			 GpuDeviceFlags deviceMask = GDF_DEFAULT) const override;
	};

	/** @} */
}}

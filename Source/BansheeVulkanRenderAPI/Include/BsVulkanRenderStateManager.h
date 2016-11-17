//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsRenderStateManager.h"

namespace BansheeEngine
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/**	Handles creation of Vulkan pipeline states. */
	class VulkanRenderStateCoreManager : public RenderStateCoreManager
	{
	protected:
		/** @copydoc RenderStateCoreManager::createSamplerStateStateInternal */
		SPtr<SamplerStateCore> createSamplerStateInternal(const SAMPLER_STATE_DESC& desc,
			GpuDeviceFlags deviceMask) const override;

		/** @copydoc RenderStateCoreManager::_createPipelineState */
		SPtr<GraphicsPipelineStateCore> _createPipelineState(const PIPELINE_STATE_CORE_DESC& desc,
			GpuDeviceFlags deviceMask = GDF_DEFAULT) const override;
	};

	/** @} */
}
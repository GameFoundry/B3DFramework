//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsGpuPipelineState.h"

namespace BansheeEngine
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/**	Vulkan implementation of a GPU pipeline state. */
	class VulkanGpuPipelineStateCore : public GpuPipelineStateCore
	{
	public:
		~VulkanGpuPipelineStateCore();

	protected:
		friend class VulkanRenderStateCoreManager;

		VulkanGpuPipelineStateCore(const PIPELINE_STATE_CORE_DESC& desc, GpuDeviceFlags deviceMask);
	};

	/** @} */
}
//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsGpuProgram.h"

namespace bs
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/**	Abstraction of a Vulkan shader object. */
	class 
	VulkanGpuProgramCore : public GpuProgramCore
	{
	public:
		virtual ~VulkanGpuProgramCore();

		/** 
		 * Returns a handle to the Vulkan shader module, on the specified device. If program device mask doesn't 
		 * include the provided device, null is returned.  
		 */
		VkShaderModule getHandle(UINT32 deviceIdx) const;

	protected:
		friend class VulkanGLSLProgramFactory;

		VulkanGpuProgramCore(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask);

		/** @copydoc GpuProgramCore::initialize */
		void initialize() override;

	private:
	};

	/** @} */
}
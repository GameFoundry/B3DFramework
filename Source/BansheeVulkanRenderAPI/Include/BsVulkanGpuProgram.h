//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsVulkanResource.h"
#include "BsGpuProgram.h"

namespace bs { namespace ct
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/** Wrapper around a Vulkan shader module (GPU program) that manages its usage and lifetime. */
	class VulkanShaderModule : public VulkanResource
	{
	public:
		VulkanShaderModule(VulkanResourceManager* owner, VkShaderModule module);
		~VulkanShaderModule();

		/** Returns the internal handle to the Vulkan object. */
		VkShaderModule getHandle() const { return mModule; }

	private:
		VkShaderModule mModule;
	};

	/**	Abstraction of a Vulkan shader object. */
	class VulkanGpuProgram : public GpuProgramCore
	{
	public:
		virtual ~VulkanGpuProgram();

		/** 
		 * Returns the shader module for the specified device. If program device mask doesn't include the provided device, 
		 * null is returned.  
		 */
		VulkanShaderModule* getShaderModule(UINT32 deviceIdx) const { return mModules[deviceIdx]; }

	protected:
		friend class VulkanGLSLProgramFactory;

		VulkanGpuProgram(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask);

		/** @copydoc GpuProgramCore::initialize */
		void initialize() override;

	private:
		GpuDeviceFlags mDeviceMask;
		VulkanShaderModule* mModules[BS_MAX_DEVICES];
	};

	/** @} */
}}
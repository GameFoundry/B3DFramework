//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsVulkanResource.h"
#include "RenderAPI/BsGpuProgram.h"

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
	class VulkanGpuProgram : public GpuProgram
	{
	public:
		virtual ~VulkanGpuProgram();

		/** 
		 * Returns the shader module for the specified device. If program device mask doesn't include the provided device, 
		 * null is returned.  
		 */
		VulkanShaderModule* getShaderModule(UINT32 deviceIdx) const { return mModules[deviceIdx]; }

		/** Returns the name of the program entry point function. */
		const String& getEntryPoint() const { return mEntryPoint; }

	protected:
		friend class VulkanGLSLProgramFactory;

		VulkanGpuProgram(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask);

		/** @copydoc GpuProgram::initialize */
		void initialize() override;

	private:
		GpuDeviceFlags mDeviceMask;
		VulkanShaderModule* mModules[BS_MAX_DEVICES];
	};

	/** Identifier of the compiler used for compiling Vulkan GPU programs. */
	static constexpr const char* VULKAN_COMPILER_ID = "Vulkan";

	/** @} */
}}
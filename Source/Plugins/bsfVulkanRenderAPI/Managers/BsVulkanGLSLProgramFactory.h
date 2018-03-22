//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "Managers/BsGpuProgramManager.h"

namespace bs { namespace ct
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/**	Handles creation of GLSL GPU programs. */
	class VulkanGLSLProgramFactory : public GpuProgramFactory
	{
	public:
		VulkanGLSLProgramFactory();
		~VulkanGLSLProgramFactory();

		/** @copydoc GpuProgramFactory::create(const GPU_PROGRAM_DESC&, GpuDeviceFlags) */
		SPtr<GpuProgram> create(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT) override;

		/** @copydoc GpuProgramFactory::create(GpuProgramType, GpuDeviceFlags) */
		SPtr<GpuProgram> create(GpuProgramType type, GpuDeviceFlags deviceMask = GDF_DEFAULT) override;

		/** @copydoc GpuProgramFactory::compile(const GPU_PROGRAM_DESC&) */
		SPtr<GpuProgramBytecode> compileBytecode(const GPU_PROGRAM_DESC& desc) override;
	protected:
		static const String LANGUAGE_NAME;
	};

	/** @} */
}}
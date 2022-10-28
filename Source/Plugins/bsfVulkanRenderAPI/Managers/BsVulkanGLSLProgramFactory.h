//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "Managers/BsGpuProgramManager.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup Vulkan
		 *  @{
		 */

		/**	Handles creation of GLSL GPU programs. */
		class VulkanGLSLProgramFactory final : public GpuProgramFactory
		{
		public:
			VulkanGLSLProgramFactory();
			~VulkanGLSLProgramFactory();

			SPtr<GpuProgram> Create(const GPU_PROGRAM_DESC& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT) override;
			SPtr<GpuProgram> Create(GpuProgramType type, GpuDeviceFlags deviceMask = GDF_DEFAULT) override;
			SPtr<GpuProgramBytecode> CompileBytecode(const GPU_PROGRAM_DESC& desc) override;

		protected:
			static const String LANGUAGE_NAME;
		};

		/** @} */
	} // namespace ct
} // namespace bs

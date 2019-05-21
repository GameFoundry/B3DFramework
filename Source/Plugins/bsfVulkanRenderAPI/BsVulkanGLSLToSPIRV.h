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

	/**
	 * Compiles GLSL source code into SPIR-V. Also performs reflection on the GLSL and outputs information about
	 * vertex inputs and GPU program globals.
	 */
	class GLSLToSPIRV : public Module<GLSLToSPIRV>
	{
	public:
		GLSLToSPIRV();
		~GLSLToSPIRV();

		/** Performs the GLSL -> SPIR-V conversion. */
		SPtr<GpuProgramBytecode> convert(const GPU_PROGRAM_DESC& desc);
	};

	/** @} */
}}
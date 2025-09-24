//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DVulkanPrerequisites.h"
#include "Utility/B3DModule.h"

namespace b3d
{
	struct GpuProgramCreateInformation;
	struct GpuProgramBytecode;
}

namespace b3d
{
	namespace render
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
			SPtr<GpuProgramBytecode> Convert(const GpuProgramCreateInformation& desc);
		};

		/** @} */
	} // namespace render
} // namespace b3d

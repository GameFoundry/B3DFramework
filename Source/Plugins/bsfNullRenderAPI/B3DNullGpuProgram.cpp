//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DNullGpuProgram.h"
#include "B3DNullGpuDevice.h"

namespace b3d
{
	namespace render
	{
		NullGpuProgram::NullGpuProgram(NullGpuDevice& gpuDevice, const GpuProgramCreateInformation& createInformation)
			: GpuProgram(createInformation)
		{
			(void)gpuDevice; // Unused parameter

			// Mark as compiled immediately since we don't actually compile anything
			mIsCompiled = true;
		}
	} // namespace render
} // namespace b3d

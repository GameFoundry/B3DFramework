//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DNullSamplerState.h"
#include "B3DNullGpuDevice.h"

namespace b3d
{
	namespace render
	{
		NullSamplerState::NullSamplerState(NullGpuDevice& gpuDevice, const SamplerStateCreateInformation& createInformation)
			: SamplerState(createInformation)
		{
			(void)gpuDevice; // Unused parameter
		}
	} // namespace render
} // namespace b3d

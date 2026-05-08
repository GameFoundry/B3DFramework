//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DNullGpuParameterSet.h"
#include "B3DNullGpuDevice.h"

namespace b3d
{
	namespace render
	{
		NullGpuParameters::NullGpuParameters(NullGpuDevice& gpuDevice, const TShared<GpuPipelineParameterSetLayout>& parameterSetLayout)
			: GpuParameterSet(parameterSetLayout, 0)
		{
			(void)gpuDevice; // Unused parameter
		}
	} // namespace render
} // namespace b3d

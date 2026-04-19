//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DNullGpuPipelineParameterLayout.h"
#include "B3DNullGpuDevice.h"

namespace b3d
{
	namespace render
	{
		NullGpuPipelineParameterLayout::NullGpuPipelineParameterLayout(NullGpuDevice& gpuDevice, const GpuPipelineParameterLayoutCreateInformation& createInformation)
			: GpuPipelineParameterLayout(gpuDevice, createInformation)
		{}
	} // namespace render
} // namespace b3d

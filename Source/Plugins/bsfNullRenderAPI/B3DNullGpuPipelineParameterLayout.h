//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DNullPrerequisites.h"
#include "RenderAPI/B3DGpuPipelineParameterLayout.h"

namespace b3d
{
	namespace render
	{
		class NullGpuDevice;

		/** @addtogroup Null
		 *  @{
		 */

		/** Holds meta-data about a set of GPU parameters used by a single pipeline state. */
		class NullGpuPipelineParameterLayout : public GpuPipelineParameterLayout
		{
		public:
			NullGpuPipelineParameterLayout(NullGpuDevice& gpuDevice, const GpuPipelineParameterLayoutCreateInformation& createInformation);
			~NullGpuPipelineParameterLayout() = default;

			void Initialize() override {}
		};

		/** @} */
	} // namespace render
} // namespace b3d

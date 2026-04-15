//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DNullPrerequisites.h"
#include "RenderAPI/B3DGpuPipelineState.h"

namespace b3d
{
	namespace render
	{
		class NullGpuDevice;

		/** @addtogroup NullGpuBackend 
		 *  @{
		 */

		/**	Null implementation of a graphics pipeline state. */
		class NullGpuGraphicsPipelineState : public GpuGraphicsPipelineState
		{
		public:
			NullGpuGraphicsPipelineState(NullGpuDevice& gpuDevice, const GpuGraphicsPipelineStateCreateInformation& createInformation);
			~NullGpuGraphicsPipelineState() = default;

			void Initialize() override {}
		};

		/**	Null implementation of a compute pipeline state. */
		class NullGpuComputePipelineState : public GpuComputePipelineState
		{
		public:
			NullGpuComputePipelineState(NullGpuDevice& gpuDevice, const GpuComputePipelineStateCreateInformation& createInformation);
			~NullGpuComputePipelineState() = default;

			void Initialize() override {}
		};

		/** @} */
	} // namespace render
} // namespace b3d

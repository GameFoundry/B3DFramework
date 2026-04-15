//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DNullPrerequisites.h"
#include "RenderAPI/B3DSamplerState.h"

namespace b3d
{
	namespace render
	{
		class NullGpuDevice;

		/** @addtogroup NullGpuBackend 
		 *  @{
		 */

		/**	Null implementation of a sampler state. */
		class NullSamplerState : public SamplerState
		{
		public:
			NullSamplerState(NullGpuDevice& gpuDevice, const SamplerStateCreateInformation& createInformation);
			~NullSamplerState() = default;

			void Initialize() override {}
		};

		/** @} */
	} // namespace render
} // namespace b3d

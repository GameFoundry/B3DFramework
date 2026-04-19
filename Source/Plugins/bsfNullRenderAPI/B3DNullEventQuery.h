//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DNullPrerequisites.h"
#include "RenderAPI/B3DEventQuery.h"

namespace b3d
{
	namespace render
	{
		class NullGpuDevice;

		/** @addtogroup NullGpuBackend
		 *  @{
		 */

		/** @copydoc EventQuery */
		class NullEventQuery final : public EventQuery
		{
		public:
			NullEventQuery(NullGpuDevice& gpuDevice);
			~NullEventQuery() = default;

			void Begin(GpuCommandBuffer& commandBuffer) override {}
			bool IsReady() const override { return true; }
		};

		/** @} */
	} // namespace render
} // namespace b3d

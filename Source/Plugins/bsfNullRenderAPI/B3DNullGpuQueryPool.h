//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DNullPrerequisites.h"
#include "RenderAPI/B3DGpuQueries.h"

namespace b3d
{
	namespace render
	{
		class NullGpuDevice;

		/** @addtogroup NullGpuBackend 
		 *  @{
		 */

		/** Null implementation of a query pool. */
		class NullGpuQueryPool final : public GpuQueryPool
		{
		public:
			NullGpuQueryPool(NullGpuDevice& gpuDevice, const GpuQueryPoolCreateInformation& createInformation);
			~NullGpuQueryPool() = default;

			GpuQueryId AllocateQuery() override;
			bool TryResolve(bool wait = false) override;
			u64 GetQueryResult(GpuQueryId queryId, u32 elementIndex = 0) override;

		private:
			u32 mNextQueryId = 0;
		};

		/** @} */
	} // namespace render
} // namespace b3d

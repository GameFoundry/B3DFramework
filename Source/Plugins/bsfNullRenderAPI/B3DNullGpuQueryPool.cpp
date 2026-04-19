//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DNullGpuQueryPool.h"
#include "B3DNullGpuDevice.h"

namespace b3d
{
	namespace render
	{
		NullGpuQueryPool::NullGpuQueryPool(NullGpuDevice& gpuDevice, const GpuQueryPoolCreateInformation& createInformation)
			: GpuQueryPool(createInformation)
		{
			(void)gpuDevice; // Unused parameter
		}

		GpuQueryId NullGpuQueryPool::AllocateQuery()
		{
			if (mNextQueryId >= mPoolSize)
				return GpuQueryId();

			return GpuQueryId(mNextQueryId++);
		}

		bool NullGpuQueryPool::TryResolve(bool wait)
		{
			return true;
		}

		u64 NullGpuQueryPool::GetQueryResult(GpuQueryId queryId, u32 elementIndex)
		{
			return 0;
		}
	} // namespace render
} // namespace b3d

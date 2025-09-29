//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DVulkanPrerequisites.h"
#include "B3DVulkanGpuCommandBuffer.h"
#include "B3DVulkanGpuDevice.h"
#include "B3DVulkanResource.h"
#include "RenderAPI/B3DGpuQueries.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup Vulkan
		 *  @{
		 */

		/** Vulkan implementation of a GPU query pool. */
		class VulkanGpuQueryPool : public GpuQueryPool, public VulkanResource
		{
		public:
			VulkanGpuQueryPool(VulkanResourceManager& vulkanResourceManager, const GpuQueryPoolCreateInformation& createInformation);
			~VulkanGpuQueryPool() override;

			GpuQueryId AllocateQuery() override;
			void Reset() override;
			bool TryResolve(bool wait = false) override;
			u64 GetQueryResult(GpuQueryId queryId, u32 elementIndex = 0) override;

			/** Returns the internal Vulkan handle to the pool. */
			VkQueryPool GetVulkanHandle() const { return mPool; }

			/** Returns number of queries allocated since the last reset. */
			u32 GetUsedQueryCount() const { return mNextFreeQueryId; }

			/** Called by the command buffer when the pool has been queued for a reset operation. */
			void NotifyPoolReset() { mNextFreeQueryId = 0; }
		private:
			VkQueryPool mPool;
			TArray<u64> mResultBuffer;
			u32 mNextFreeQueryId = 0;
		};

		/** @} */
	} // namespace render
} // namespace b3d

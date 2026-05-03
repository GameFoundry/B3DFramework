//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DVulkanPrerequisites.h"
#include "B3DVulkanGpuCommandBuffer.h"
#include "B3DVulkanGpuDevice.h"
#include "B3DVulkanResource.h"
#include "GpuBackend/B3DGpuQueries.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup Vulkan
		 *  @{
		 */

		/** Vulkan implementation of a GPU query pool. */
		class VulkanGpuQueryPool : public VulkanResource, public GpuQueryPool // Note: VulkanResource must be first base class because we call delete on 'this' from VulkanResource
		{
		public:
			VulkanGpuQueryPool(VulkanResourceManager& vulkanResourceManager, const GpuQueryPoolCreateInformation& createInformation);
			~VulkanGpuQueryPool() override;

			GpuQueryId AllocateQuery() override;
			bool TryResolve(bool wait = false) override;
			u64 GetQueryResult(GpuQueryId queryId, u32 elementIndex = 0) override;

			/** Returns the internal Vulkan handle to the pool. */
			VkQueryPool GetVulkanHandle() const { return mPool; }

			/** Returns number of queries allocated since the last reset. */
			u32 GetUsedQueryCount() const { return mNextFreeQueryId; }

			/** Called by the command buffer when the pool has been queued for a reset operation. */
			void NotifyPoolReset() { mNextFreeQueryId = 0; }

			/** Called when the last reference in the shared pointer owning this object goes out of scope. */
			template <class T, class AllocatorTag>
			static void SharedDeleter(VulkanGpuQueryPool* object)
			{
				if(!object->IsDestroyRequested())
					object->Destroy();
			}
		private:
			VkQueryPool mPool;
			TArray<u64> mResultBuffer;
			u32 mNextFreeQueryId = 0;
		};

		/** @} */
	} // namespace render
} // namespace b3d

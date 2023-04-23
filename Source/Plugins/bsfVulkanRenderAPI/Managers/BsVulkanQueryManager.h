//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsVulkanResource.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup Vulkan
		 *  @{
		 */

		/**
		 * Pool that allocates and distributes Vulkan queries.
		 *
		 * @note	Thread safe.
		 */
		class VulkanQueryPool
		{
		public:
			VulkanQueryPool(VulkanGpuDevice& device);
			~VulkanQueryPool();

			/**
			 * Begins a timer query on the provided command buffer.
			 *
			 * @param	commandBuffer	Command buffer to begin the query on.
			 * @return					Relevant query object that was queued. It must be released via ReleaseQuery() once the
			 *							caller is done accessing it.
			 */
			VulkanQuery* BeginTimerQuery(VulkanGpuCommandBuffer& commandBuffer);

			/**
			 * Begins an occlusion query on the provided command buffer. Must be followed with a call to EndOcclusionQuery
			 * on the same command buffer, before the command buffer gets submitted.
			 *
			 * @param	commandBuffer	Command buffer to begin the query on.
			 * @param	precise			When true the query will be able to return the precise number of processed samples,
			 *							otherwise it just returns a boolean value if anything was drawn.
			 * @return					Relevant query object that was queued. It must be released via ReleaseQuery() once the
			 *							caller is done accessing it.
			 */
			VulkanQuery* BeginOcclusionQuery(VulkanGpuCommandBuffer& commandBuffer, bool precise);

			/**
			 * End am occlusion query query on the provided command buffer.
			 *
			 * @param	commandBuffer	Command buffer to begin the query on.
			 * @param	query			Query previously begun with BeginOcclusionQuery().
			 */
			void EndOcclusionQuery(VulkanGpuCommandBuffer& commandBuffer, VulkanQuery& query);

			/** Releases a previously retrieved query, ensuring it can be re-used. */
			void ReleaseQuery(VulkanQuery& query);

		private:
			/** Query buffer pool and related information. */
			struct PoolInfo
			{
				VkQueryPool Pool = VK_NULL_HANDLE;
				u32 StartIdx;
			};

			/** Attempts to find a free query of the specified type, or allocates a new one. Creates new pools as needed. */
			VulkanQuery* GetQuery(VkQueryType type);

			/** Creates a new Vulkan query pool object. */
			PoolInfo& AllocatePool(VkQueryType type);

			static constexpr u32 kQueriesPerPoolCount = 16;

			VulkanGpuDevice& mDevice;

			Vector<VulkanQuery*> mTimerQueries;
			Vector<VulkanQuery*> mOcclusionQueries;

			Vector<PoolInfo> mTimerPools;
			Vector<PoolInfo> mOcclusionPools;

			Mutex mMutex;
		};

		/** Wrapper around a single query in a Vulkan query pool object. */
		class VulkanQuery : public VulkanResource
		{
		public:
			VulkanQuery(VulkanResourceManager* owner, VkQueryPool pool, u32 queryIdx);

			/**
			 * Attempts to retrieve the result from the query. The result is only valid if the query stopped executing on the
			 * GPU (otherwise previous query results could be accessed, if the reset command hasn't executed yet).
			 *
			 * @param[out]	result	Value of the query, if the method return true. Undefined otherwise.
			 * @return				True if the result is ready, false otherwise.
			 */
			bool GetResult(u64& result) const;

			/** Queues a command for the query reset, on the provided command buffer. */
			void Reset(VkCommandBuffer cmdBuf);

		private:
			friend class VulkanQueryPool;

			VkQueryPool mPool;
			u32 mQueryIdx;
			bool mFree = true;
		};

		/** @} */
	} // namespace ct
} // namespace bs

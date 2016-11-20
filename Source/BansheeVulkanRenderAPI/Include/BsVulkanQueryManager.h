//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsVulkanResource.h"
#include "BsQueryManager.h"

namespace bs
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
		VulkanQueryPool(VulkanDevice& device);
		~VulkanQueryPool();

		/** 
		 * Begins a timer query on the provided command buffer. 
		 * 
		 * @param[in]	cb			Command buffer to begin the query on.
		 * @return					Relevant query object that was queued. It must be released via releaseQuery() once the
		 *							caller is done accessing it.
		 */
		VulkanQuery* beginTimerQuery(VulkanCmdBuffer* cb);

		/** 
		 * Begins an occlusion query on the provided command buffer. Must be followed with a call to endOcclusionQuery
		 * on the same command buffer, before the command buffer gets submitted.
		 * 
		 * @param[in]	cb			Command buffer to begin the query on.
		 * @param[in]	precise		When true the query will be able to return the precise number of processed samples, 
		 *							otherwise it just returns a boolean value if anything was drawn.
		 * @return					Relevant query object that was queued. It must be released via releaseQuery() once the
		 *							caller is done accessing it.
		 */
		VulkanQuery* beginOcclusionQuery(VulkanCmdBuffer* cb, bool precise);

		/** 
		 * End am occlusion query query on the provided command buffer. 
		 * 
		 * @param[in]	query		Query previously begun with beginOcclusionQuery().
		 * @param[in]	cb			Command buffer to end the query on.
		 */
		void endOcclusionQuery(VulkanQuery* query, VulkanCmdBuffer* cb);

		/** Releases a previously retrieved query, ensuring it can be re-used. */
		void releaseQuery(VulkanQuery* query);

	private:
		/** Query buffer pool and related information. */
		struct PoolInfo
		{
			VkQueryPool pool = VK_NULL_HANDLE;
			UINT32 startIdx;
		};

		/** Attempts to find a free query of the specified type, or allocates a new one. Creates new pools as needed. */
		VulkanQuery* getQuery(VkQueryType type);

		/** Creates a new Vulkan query pool object. */
		PoolInfo& allocatePool(VkQueryType type);

		static const UINT32 NUM_QUERIES_PER_POOL = 16;

		VulkanDevice& mDevice;

		Vector<VulkanQuery*> mTimerQueries;
		Vector<VulkanQuery*> mOcclusionQueries;

		Vector<PoolInfo> mTimerPools;
		Vector<PoolInfo> mOcclusionPools;

		Mutex mMutex;
	};

	/**	Handles creation of Vulkan queries. */
	class VulkanQueryManager : public QueryManager
	{
	public:
		VulkanQueryManager(VulkanRenderAPI& rapi);
		~VulkanQueryManager();

		/** @copydoc QueryManager::createEventQuery */
		SPtr<EventQuery> createEventQuery(UINT32 deviceIdx = 0) const override;

		/** @copydoc QueryManager::createTimerQuery */
		SPtr<TimerQuery> createTimerQuery(UINT32 deviceIdx = 0) const override;

		/** @copydoc QueryManager::createOcclusionQuery */
		SPtr<OcclusionQuery> createOcclusionQuery(bool binary, UINT32 deviceIdx = 0) const override;

	private:
		VulkanRenderAPI& mRenderAPI;
	};

	/** Wrapper around a single query in a Vulkan query pool object. */
	class VulkanQuery : public VulkanResource
	{
	public:
		VulkanQuery(VulkanResourceManager* owner, VkQueryPool pool, UINT32 queryIdx);

		/** 
		 * Attempts to retrieve the result from the query. The result is only valid if the query stopped executing on the
		 * GPU (otherwise previous query results could be accessed, if the reset command hasn't executed yet).
		 * 
		 * @param[out]	result	Value of the query, if the method return true. Undefined otherwise.
		 * @return				True if the result is ready, false otherwise.
		 */
		bool getResult(UINT64& result) const;

	private:
		friend class VulkanQueryPool;

		VkQueryPool mPool;
		UINT32 mQueryIdx;
		bool mFree;		
	};

	/** @} */
}
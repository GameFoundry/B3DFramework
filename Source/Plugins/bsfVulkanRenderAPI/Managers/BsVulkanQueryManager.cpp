//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/BsVulkanQueryManager.h"
#include "BsVulkanEventQuery.h"
#include "BsVulkanTimerQuery.h"
#include "BsVulkanOcclusionQuery.h"
#include "BsVulkanRenderAPI.h"
#include "BsVulkanDevice.h"
#include "BsVulkanCommandBuffer.h"

namespace bs { namespace ct
{
	VulkanQueryPool::VulkanQueryPool(VulkanDevice& device)
		:mDevice(device)
	{
		Lock lock(mMutex);

		allocatePool(VK_QUERY_TYPE_TIMESTAMP);
		allocatePool(VK_QUERY_TYPE_OCCLUSION);
	}

	VulkanQueryPool::~VulkanQueryPool()
	{
		Lock lock(mMutex);

		for (auto& entry : mTimerQueries)
		{
			if(entry != nullptr)
				entry->destroy();
		}

		for (auto& entry : mOcclusionQueries)
		{
			if(entry != nullptr)
				entry->destroy();
		}

		for (auto& entry : mTimerPools)
			vkDestroyQueryPool(mDevice.getLogical(), entry.pool, gVulkanAllocator);

		for (auto& entry : mOcclusionPools)
			vkDestroyQueryPool(mDevice.getLogical(), entry.pool, gVulkanAllocator);
	}

	VulkanQueryPool::PoolInfo& VulkanQueryPool::allocatePool(VkQueryType type)
	{
		VkQueryPoolCreateInfo queryPoolCI;
		queryPoolCI.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
		queryPoolCI.pNext = nullptr;
		queryPoolCI.flags = 0;
		queryPoolCI.pipelineStatistics = 0;
		queryPoolCI.queryCount = NUM_QUERIES_PER_POOL;
		queryPoolCI.queryType = type;

		PoolInfo poolInfo;
		VkResult result = vkCreateQueryPool(mDevice.getLogical(), &queryPoolCI, gVulkanAllocator, &poolInfo.pool);
		assert(result == VK_SUCCESS);

		Vector<PoolInfo>& poolInfos = type == VK_QUERY_TYPE_TIMESTAMP ? mTimerPools : mOcclusionPools;
		poolInfo.startIdx = (UINT32)poolInfos.size() * NUM_QUERIES_PER_POOL;

		poolInfos.push_back(poolInfo);

		Vector<VulkanQuery*>& queries = type == VK_QUERY_TYPE_TIMESTAMP ? mTimerQueries : mOcclusionQueries;
		for (UINT32 i = 0; i < NUM_QUERIES_PER_POOL; i++)
			queries.push_back(nullptr);

		return poolInfos.back();
	}

	VulkanQuery* VulkanQueryPool::getQuery(VkQueryType type)
	{
		Vector<VulkanQuery*>& queries = type == VK_QUERY_TYPE_TIMESTAMP ? mTimerQueries : mOcclusionQueries;
		Vector<PoolInfo>& poolInfos = type == VK_QUERY_TYPE_TIMESTAMP ? mTimerPools : mOcclusionPools;

		for (UINT32 i = 0; i < (UINT32)queries.size(); i++)
		{
			VulkanQuery* curQuery = queries[i];
			if (curQuery == nullptr)
			{
				div_t divResult = std::div(i, (INT32)NUM_QUERIES_PER_POOL);
				UINT32 poolIdx = (UINT32)divResult.quot;
				UINT32 queryIdx = (UINT32)divResult.rem;

				curQuery = mDevice.getResourceManager().create<VulkanQuery>(poolInfos[poolIdx].pool, queryIdx);
				queries[i] = curQuery;

				return curQuery;
			}
			else if (!curQuery->isBound() && curQuery->mFree)
				return curQuery;
		}

		PoolInfo& poolInfo = allocatePool(type);
		UINT32 queryIdx = poolInfo.startIdx % NUM_QUERIES_PER_POOL;

		VulkanQuery* query = mDevice.getResourceManager().create<VulkanQuery>(poolInfo.pool, queryIdx);
		queries[poolInfo.startIdx] = query;

		return query;
	}

	VulkanQuery* VulkanQueryPool::beginTimerQuery(VulkanCmdBuffer* cb)
	{
		Lock lock(mMutex);

		VulkanQuery* query = getQuery(VK_QUERY_TYPE_TIMESTAMP);
		query->mFree = false;

		VkCommandBuffer vkCmdBuf = cb->getHandle();
		cb->resetQuery(query);
		vkCmdWriteTimestamp(vkCmdBuf, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, query->mPool, query->mQueryIdx);

		// Note: Must happen only here because we need to check VulkanResource::isBound under the same mutex
		cb->registerResource(query, VulkanAccessFlag::Write);

		return query;
	}

	VulkanQuery* VulkanQueryPool::beginOcclusionQuery(VulkanCmdBuffer* cb, bool precise)
	{
		Lock lock(mMutex);

		VulkanQuery* query = getQuery(VK_QUERY_TYPE_TIMESTAMP);
		query->mFree = false;

		VkCommandBuffer vkCmdBuf = cb->getHandle();
		cb->resetQuery(query);
		vkCmdBeginQuery(vkCmdBuf, query->mPool, query->mQueryIdx, precise ? VK_QUERY_CONTROL_PRECISE_BIT : 0);

		// Note: Must happen only here because we need to check VulkanResource::isBound under the same mutex
		cb->registerResource(query, VulkanAccessFlag::Write);

		return query;
	}

	void VulkanQueryPool::endOcclusionQuery(VulkanQuery* query, VulkanCmdBuffer* cb)
	{
		Lock lock(mMutex);

		VkCommandBuffer vkCmdBuf = cb->getHandle();
		vkCmdEndQuery(vkCmdBuf, query->mPool, query->mQueryIdx);
	}

	void VulkanQueryPool::releaseQuery(VulkanQuery* query)
	{
		Lock lock(mMutex);

		query->mFree = true;
	}

	VulkanQueryManager::VulkanQueryManager(VulkanRenderAPI& rapi)
		:mRenderAPI(rapi)
	{ }

	SPtr<EventQuery> VulkanQueryManager::createEventQuery(UINT32 deviceIdx) const
	{
		SPtr<VulkanDevice> device = mRenderAPI._getDevice(deviceIdx);

		SPtr<EventQuery> query = SPtr<VulkanEventQuery>(bs_new<VulkanEventQuery>(*device),
			&QueryManager::deleteEventQuery, StdAlloc<VulkanEventQuery>());
		mEventQueries.push_back(query.get());

		return query;
	}

	SPtr<TimerQuery> VulkanQueryManager::createTimerQuery(UINT32 deviceIdx) const
	{
		SPtr<VulkanDevice> device = mRenderAPI._getDevice(deviceIdx);

		SPtr<TimerQuery> query = SPtr<VulkanTimerQuery>(bs_new<VulkanTimerQuery>(*device),
			&QueryManager::deleteTimerQuery, StdAlloc<VulkanTimerQuery>());
		mTimerQueries.push_back(query.get());

		return query;
	}

	SPtr<OcclusionQuery> VulkanQueryManager::createOcclusionQuery(bool binary, UINT32 deviceIdx) const
	{
		SPtr<VulkanDevice> device = mRenderAPI._getDevice(deviceIdx);

		SPtr<OcclusionQuery> query = SPtr<VulkanOcclusionQuery>(bs_new<VulkanOcclusionQuery>(*device, binary),
			&QueryManager::deleteOcclusionQuery, StdAlloc<VulkanOcclusionQuery>());
		mOcclusionQueries.push_back(query.get());

		return query;
	}

	VulkanQuery::VulkanQuery(VulkanResourceManager* owner, VkQueryPool pool, UINT32 queryIdx)
		:VulkanResource(owner, false), mPool(pool), mQueryIdx(queryIdx)
	{
	}

	bool VulkanQuery::getResult(UINT64& result) const
	{
		// Note: A potentially better approach to get results is to make the query pool a VulkanResource, which we attach
		// to a command buffer upon use. Then when CB finishes executing we perform vkGetQueryPoolResults on all queries
		// in the pool at once.

		VkDevice vkDevice = mOwner->getDevice().getLogical();
		VkResult vkResult = vkGetQueryPoolResults(vkDevice, mPool, mQueryIdx, 1, sizeof(result), &result, sizeof(result),
												  VK_QUERY_RESULT_64_BIT);
		assert(vkResult == VK_SUCCESS || vkResult == VK_NOT_READY);

		return vkResult == VK_SUCCESS;
	}

	void VulkanQuery::reset(VkCommandBuffer cmdBuf)
	{
		vkCmdResetQueryPool(cmdBuf, mPool, mQueryIdx, 1);
	}
}}
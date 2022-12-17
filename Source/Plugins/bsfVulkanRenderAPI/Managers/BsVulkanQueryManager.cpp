//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/BsVulkanQueryManager.h"
#include "BsVulkanEventQuery.h"
#include "BsVulkanTimerQuery.h"
#include "BsVulkanOcclusionQuery.h"
#include "BsVulkanRenderAPI.h"
#include "BsVulkanDevice.h"
#include "BsVulkanCommandBuffer.h"

using namespace bs;
using namespace bs::ct;

VulkanQueryPool::VulkanQueryPool(VulkanDevice& device)
	: mDevice(device)
{
	Lock lock(mMutex);

	AllocatePool(VK_QUERY_TYPE_TIMESTAMP);
	AllocatePool(VK_QUERY_TYPE_OCCLUSION);
}

VulkanQueryPool::~VulkanQueryPool()
{
	Lock lock(mMutex);

	for(auto& entry : mTimerQueries)
	{
		if(entry != nullptr)
			entry->Destroy();
	}

	for(auto& entry : mOcclusionQueries)
	{
		if(entry != nullptr)
			entry->Destroy();
	}

	for(auto& entry : mTimerPools)
		vkDestroyQueryPool(mDevice.GetLogical(), entry.Pool, gVulkanAllocator);

	for(auto& entry : mOcclusionPools)
		vkDestroyQueryPool(mDevice.GetLogical(), entry.Pool, gVulkanAllocator);
}

VulkanQueryPool::PoolInfo& VulkanQueryPool::AllocatePool(VkQueryType type)
{
	VkQueryPoolCreateInfo queryPoolCI;
	queryPoolCI.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
	queryPoolCI.pNext = nullptr;
	queryPoolCI.flags = 0;
	queryPoolCI.pipelineStatistics = 0;
	queryPoolCI.queryCount = NUM_QUERIES_PER_POOL;
	queryPoolCI.queryType = type;

	PoolInfo poolInfo;
	VkResult result = vkCreateQueryPool(mDevice.GetLogical(), &queryPoolCI, gVulkanAllocator, &poolInfo.Pool);
	B3D_ASSERT(result == VK_SUCCESS);

	Vector<PoolInfo>& poolInfos = type == VK_QUERY_TYPE_TIMESTAMP ? mTimerPools : mOcclusionPools;
	poolInfo.StartIdx = (u32)poolInfos.size() * NUM_QUERIES_PER_POOL;

	poolInfos.push_back(poolInfo);

	Vector<VulkanQuery*>& queries = type == VK_QUERY_TYPE_TIMESTAMP ? mTimerQueries : mOcclusionQueries;
	for(u32 i = 0; i < NUM_QUERIES_PER_POOL; i++)
		queries.push_back(nullptr);

	return poolInfos.back();
}

VulkanQuery* VulkanQueryPool::GetQuery(VkQueryType type)
{
	Vector<VulkanQuery*>& queries = type == VK_QUERY_TYPE_TIMESTAMP ? mTimerQueries : mOcclusionQueries;
	Vector<PoolInfo>& poolInfos = type == VK_QUERY_TYPE_TIMESTAMP ? mTimerPools : mOcclusionPools;

	for(u32 i = 0; i < (u32)queries.size(); i++)
	{
		VulkanQuery* curQuery = queries[i];
		if(curQuery == nullptr)
		{
			div_t divResult = std::div(i, (i32)NUM_QUERIES_PER_POOL);
			u32 poolIdx = (u32)divResult.quot;
			u32 queryIdx = (u32)divResult.rem;

			curQuery = mDevice.GetResourceManager().Create<VulkanQuery>(poolInfos[poolIdx].Pool, queryIdx);
			queries[i] = curQuery;

			return curQuery;
		}
		else if(!curQuery->IsBound() && curQuery->mFree)
			return curQuery;
	}

	PoolInfo& poolInfo = AllocatePool(type);
	u32 queryIdx = poolInfo.StartIdx % NUM_QUERIES_PER_POOL;

	VulkanQuery* query = mDevice.GetResourceManager().Create<VulkanQuery>(poolInfo.Pool, queryIdx);
	queries[poolInfo.StartIdx] = query;

	return query;
}

VulkanQuery* VulkanQueryPool::BeginTimerQuery(VulkanInternalCommandBuffer* cb)
{
	Lock lock(mMutex);

	VulkanQuery* query = GetQuery(VK_QUERY_TYPE_TIMESTAMP);
	query->mFree = false;

	VkCommandBuffer vkCmdBuf = cb->GetHandle();
	cb->ResetQuery(query);
	vkCmdWriteTimestamp(vkCmdBuf, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, query->mPool, query->mQueryIdx);

	// Note: Must happen only here because we need to check VulkanResource::isBound under the same mutex
	cb->RegisterResource(query, VulkanAccessFlag::Write);

	return query;
}

VulkanQuery* VulkanQueryPool::BeginOcclusionQuery(VulkanInternalCommandBuffer* cb, bool precise)
{
	Lock lock(mMutex);

	VulkanQuery* query = GetQuery(VK_QUERY_TYPE_OCCLUSION);
	query->mFree = false;

	VkCommandBuffer vkCmdBuf = cb->GetHandle();
	cb->ResetQuery(query);
	vkCmdBeginQuery(vkCmdBuf, query->mPool, query->mQueryIdx, precise ? VK_QUERY_CONTROL_PRECISE_BIT : 0);

	// Note: Must happen only here because we need to check VulkanResource::isBound under the same mutex
	cb->RegisterResource(query, VulkanAccessFlag::Write);

	return query;
}

void VulkanQueryPool::EndOcclusionQuery(VulkanQuery* query, VulkanInternalCommandBuffer* cb)
{
	Lock lock(mMutex);

	VkCommandBuffer vkCmdBuf = cb->GetHandle();
	vkCmdEndQuery(vkCmdBuf, query->mPool, query->mQueryIdx);
}

void VulkanQueryPool::ReleaseQuery(VulkanQuery* query)
{
	Lock lock(mMutex);

	query->mFree = true;
}

VulkanQueryManager::VulkanQueryManager(VulkanRenderAPI& rapi)
	: mRenderAPI(rapi)
{}

SPtr<EventQuery> VulkanQueryManager::CreateEventQuery(u32 deviceIdx) const
{
	SPtr<VulkanDevice> device = mRenderAPI.GetDevice(deviceIdx);

	SPtr<EventQuery> query = SPtr<VulkanEventQuery>(B3DNew<VulkanEventQuery>(*device), &QueryManager::DeleteEventQuery, StdAlloc<VulkanEventQuery>());
	mEventQueries.push_back(query.get());

	return query;
}

SPtr<TimerQuery> VulkanQueryManager::CreateTimerQuery(u32 deviceIdx) const
{
	SPtr<VulkanDevice> device = mRenderAPI.GetDevice(deviceIdx);

	SPtr<TimerQuery> query = SPtr<VulkanTimerQuery>(B3DNew<VulkanTimerQuery>(*device), &QueryManager::DeleteTimerQuery, StdAlloc<VulkanTimerQuery>());
	mTimerQueries.push_back(query.get());

	return query;
}

SPtr<OcclusionQuery> VulkanQueryManager::CreateOcclusionQuery(bool binary, u32 deviceIdx) const
{
	SPtr<VulkanDevice> device = mRenderAPI.GetDevice(deviceIdx);

	SPtr<OcclusionQuery> query = SPtr<VulkanOcclusionQuery>(B3DNew<VulkanOcclusionQuery>(*device, binary), &QueryManager::DeleteOcclusionQuery, StdAlloc<VulkanOcclusionQuery>());
	mOcclusionQueries.push_back(query.get());

	return query;
}

VulkanQuery::VulkanQuery(VulkanResourceManager* owner, VkQueryPool pool, u32 queryIdx)
	: VulkanResource(owner, false), mPool(pool), mQueryIdx(queryIdx)
{
}

bool VulkanQuery::GetResult(u64& result) const
{
	// Note: A potentially better approach to get results is to make the query pool a VulkanResource, which we attach
	// to a command buffer upon use. Then when CB finishes executing we perform vkGetQueryPoolResults on all queries
	// in the pool at once.

	VkDevice vkDevice = mOwner->GetDevice().GetLogical();
	VkResult vkResult = vkGetQueryPoolResults(vkDevice, mPool, mQueryIdx, 1, sizeof(result), &result, sizeof(result), VK_QUERY_RESULT_64_BIT);
	B3D_ASSERT(vkResult == VK_SUCCESS || vkResult == VK_NOT_READY);

	return vkResult == VK_SUCCESS;
}

void VulkanQuery::Reset(VkCommandBuffer cmdBuf)
{
	vkCmdResetQueryPool(cmdBuf, mPool, mQueryIdx, 1);
}

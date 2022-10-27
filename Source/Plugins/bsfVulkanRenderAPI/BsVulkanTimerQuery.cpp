//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanTimerQuery.h"
#include "BsVulkanDevice.h"
#include "Managers/BsVulkanQueryManager.h"
#include "BsVulkanRenderAPI.h"
#include "BsVulkanCommandBuffer.h"
#include "Profiling/BsRenderStats.h"

using namespace bs;
using namespace bs::ct;

VulkanTimerQuery::VulkanTimerQuery(VulkanDevice& device)
	: mDevice(device), mQueryEndCalled(false), mQueryFinalized(false)
{
	BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_Query);
}

VulkanTimerQuery::~VulkanTimerQuery()
{
	for(auto& query : mQueries)
	{
		if(query.first != nullptr)
			mDevice.GetQueryPool().ReleaseQuery(query.first);

		if(query.second != nullptr)
			mDevice.GetQueryPool().ReleaseQuery(query.second);
	}

	mQueries.clear();

	BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_Query);
}

void VulkanTimerQuery::Begin(const SPtr<CommandBuffer>& cb)
{
	VulkanQueryPool& queryPool = mDevice.GetQueryPool();

	// Clear any existing queries
	for(auto& query : mQueries)
	{
		if(query.first != nullptr)
			queryPool.ReleaseQuery(query.first);

		if(query.second != nullptr)
			queryPool.ReleaseQuery(query.second);
	}

	mQueries.clear();

	mQueryEndCalled = false;
	mTimeDelta = 0.0f;

	// Retrieve and queue new query
	VulkanCommandBuffer* vulkanCB;
	if(cb != nullptr)
		vulkanCB = static_cast<VulkanCommandBuffer*>(cb.get());
	else
		vulkanCB = static_cast<VulkanCommandBuffer*>(gVulkanRenderAPI().GetMainCommandBufferInternal());

	VulkanCmdBuffer* internalCB = vulkanCB->GetInternal();
	VulkanQuery* beginQuery = queryPool.BeginTimerQuery(internalCB);
	internalCB->RegisterQuery(this);

	mQueries.push_back(std::make_pair(beginQuery, nullptr));

	SetActive(true);
}

void VulkanTimerQuery::End(const SPtr<CommandBuffer>& cb)
{
	if(mQueries.empty())
	{
		BS_LOG(Error, RenderBackend, "end() called but query was never started.");
		return;
	}

	mQueryEndCalled = true;
	mQueryFinalized = false;

	VulkanCommandBuffer* vulkanCB;
	if(cb != nullptr)
		vulkanCB = static_cast<VulkanCommandBuffer*>(cb.get());
	else
		vulkanCB = static_cast<VulkanCommandBuffer*>(gVulkanRenderAPI().GetMainCommandBufferInternal());

	VulkanQueryPool& queryPool = mDevice.GetQueryPool();
	VulkanCmdBuffer* internalCB = vulkanCB->GetInternal();
	VulkanQuery* endQuery = queryPool.BeginTimerQuery(internalCB);
	internalCB->RegisterQuery(this);

	mQueries.back().second = endQuery;
}

bool VulkanTimerQuery::IsInProgressInternal() const
{
	return !mQueries.empty() && !mQueryEndCalled;
}

void VulkanTimerQuery::InterruptInternal(VulkanCmdBuffer& cb)
{
	assert(!mQueries.empty() && !mQueryEndCalled);

	mQueryEndCalled = true;
	mQueryFinalized = false;

	VulkanQueryPool& queryPool = mDevice.GetQueryPool();
	VulkanQuery* endQuery = queryPool.BeginTimerQuery(&cb);
	cb.RegisterQuery(this);

	mQueries.back().second = endQuery;
}

bool VulkanTimerQuery::IsReady() const
{
	if(!mQueryEndCalled)
		return false;

	if(mQueryFinalized)
		return true;

	u64 timeBegin, timeEnd;
	bool ready = true;
	for(auto& entry : mQueries)
	{
		ready &= !entry.first->IsBound() && entry.first->GetResult(timeBegin);
		ready &= !entry.second->IsBound() && entry.second->GetResult(timeEnd);
	}

	return ready;
}

float VulkanTimerQuery::GetTimeMs()
{
	if(!mQueryFinalized)
	{
		u64 totalTimeDiff = 0;
		bool ready = true;
		for(auto& entry : mQueries)
		{
			u64 timeBegin = 0;
			u64 timeEnd = 0;
			ready &= !entry.first->IsBound() && entry.first->GetResult(timeBegin);
			ready &= !entry.second->IsBound() && entry.second->GetResult(timeEnd);

			totalTimeDiff += (timeEnd - timeBegin);
		}

		if(ready)
		{
			mQueryFinalized = true;

			double timestampToMs = (double)mDevice.GetDeviceProperties().limits.timestampPeriod / 1e6; // Nano to milli
			mTimeDelta = (float)((double)totalTimeDiff * timestampToMs);

			VulkanQueryPool& queryPool = mDevice.GetQueryPool();
			for(auto& query : mQueries)
			{
				if(query.first != nullptr)
					queryPool.ReleaseQuery(query.first);

				if(query.second != nullptr)
					queryPool.ReleaseQuery(query.second);
			}

			mQueries.clear();
		}
	}

	return mTimeDelta;
}

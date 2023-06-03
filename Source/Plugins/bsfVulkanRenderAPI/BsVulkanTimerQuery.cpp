//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanTimerQuery.h"
#include "BsVulkanGpuDevice.h"
#include "Managers/BsVulkanQueryManager.h"
#include "BsVulkanGpuCommandBuffer.h"
#include "Profiling/BsRenderStats.h"

using namespace bs;
using namespace bs::ct;

VulkanTimerQuery::VulkanTimerQuery(VulkanGpuDevice& device)
	: mDevice(device), mQueryEndCalled(false), mQueryFinalized(false)
{
	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResCreated, RenderStatObject_Query);
}

VulkanTimerQuery::~VulkanTimerQuery()
{
	for(auto& query : mQueries)
	{
		if(query.first != nullptr)
			mDevice.GetQueryPool().ReleaseQuery(*query.first);

		if(query.second != nullptr)
			mDevice.GetQueryPool().ReleaseQuery(*query.second);
	}

	mQueries.clear();

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResDestroyed, RenderStatObject_Query);
}

void VulkanTimerQuery::Begin(GpuCommandBuffer& commandBuffer)
{
	VulkanQueryPool& queryPool = mDevice.GetQueryPool();

	// Clear any existing queries
	for(auto& query : mQueries)
	{
		if(query.first != nullptr)
			queryPool.ReleaseQuery(*query.first);

		if(query.second != nullptr)
			queryPool.ReleaseQuery(*query.second);
	}

	mQueries.clear();

	mQueryEndCalled = false;
	mTimeDelta = 0.0f;

	// Retrieve and queue new query
	VulkanGpuCommandBuffer& vulkanCommandBuffer = static_cast<VulkanGpuCommandBuffer&>(commandBuffer);
	VulkanQuery* beginQuery = queryPool.BeginTimerQuery(vulkanCommandBuffer);
	vulkanCommandBuffer.RegisterQuery(this);

	mQueries.push_back(std::make_pair(beginQuery, nullptr));
}

void VulkanTimerQuery::End(GpuCommandBuffer& commandBuffer)
{
	if(mQueries.empty())
	{
		B3D_LOG(Error, RenderBackend, "end() called but query was never started.");
		return;
	}

	mQueryEndCalled = true;
	mQueryFinalized = false;

	VulkanGpuCommandBuffer& vulkanCommandBuffer = static_cast<VulkanGpuCommandBuffer&>(commandBuffer);

	VulkanQueryPool& queryPool = mDevice.GetQueryPool();
	VulkanQuery* endQuery = queryPool.BeginTimerQuery(vulkanCommandBuffer);
	vulkanCommandBuffer.RegisterQuery(this);

	mQueries.back().second = endQuery;
}

bool VulkanTimerQuery::IsInProgress() const
{
	return !mQueries.empty() && !mQueryEndCalled;
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
					queryPool.ReleaseQuery(*query.first);

				if(query.second != nullptr)
					queryPool.ReleaseQuery(*query.second);
			}

			mQueries.clear();
		}
	}

	return mTimeDelta;
}

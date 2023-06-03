//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanOcclusionQuery.h"
#include "BsVulkanGpuDevice.h"
#include "Managers/BsVulkanQueryManager.h"
#include "BsVulkanGpuCommandBuffer.h"
#include "Profiling/BsRenderStats.h"

using namespace bs;
using namespace bs::ct;

VulkanOcclusionQuery::VulkanOcclusionQuery(VulkanGpuDevice& device, bool binary)
	: OcclusionQuery(binary), mDevice(device), mQueryEndCalled(false), mQueryFinalized(false)
{
	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResCreated, RenderStatObject_Query);
}

VulkanOcclusionQuery::~VulkanOcclusionQuery()
{
	for(auto& query : mQueries)
		mDevice.GetQueryPool().ReleaseQuery(*query);

	mQueries.clear();

	B3D_INCREMENT_RENDER_STATISTIC_CATEGORY(ResDestroyed, RenderStatObject_Query);
}

void VulkanOcclusionQuery::Begin(GpuCommandBuffer& commandBuffer)
{
	VulkanQueryPool& queryPool = mDevice.GetQueryPool();

	// Clear any existing queries
	for(auto& query : mQueries)
		mDevice.GetQueryPool().ReleaseQuery(*query);

	mQueries.clear();

	mQueryEndCalled = false;
	mNumSamples = 0;

	// Retrieve and queue new query
	VulkanGpuCommandBuffer& vulkanCommandBuffer = static_cast<VulkanGpuCommandBuffer&>(commandBuffer);
	mQueries.push_back(queryPool.BeginOcclusionQuery(vulkanCommandBuffer, !mBinary));
	vulkanCommandBuffer.RegisterQuery(this);
}

void VulkanOcclusionQuery::End(GpuCommandBuffer& commandBuffer)
{
	if(mQueries.empty())
	{
		B3D_LOG(Error, RenderBackend, "end() called but query was never started.");
		return;
	}

	// Could have been interrupted
	if(mQueryEndCalled)
		return;

	mQueryEndCalled = true;
	mQueryFinalized = false;

	VulkanGpuCommandBuffer& vulkanCommandBuffer = static_cast<VulkanGpuCommandBuffer&>(commandBuffer);

	VulkanQueryPool& queryPool = mDevice.GetQueryPool();
	queryPool.EndOcclusionQuery(vulkanCommandBuffer, *mQueries.back());
}

bool VulkanOcclusionQuery::IsInProgress() const
{
	return !mQueries.empty() && !mQueryEndCalled;
}

void VulkanOcclusionQuery::Interrupt(VulkanGpuCommandBuffer& commandBuffer)
{
	B3D_ASSERT(!mQueries.empty() && !mQueryEndCalled);

	mQueryEndCalled = true;
	mQueryFinalized = false;

	VulkanQueryPool& queryPool = mDevice.GetQueryPool();
	queryPool.EndOcclusionQuery(commandBuffer, *mQueries.back());
}

bool VulkanOcclusionQuery::IsReady() const
{
	if(!mQueryEndCalled)
		return false;

	if(mQueryFinalized)
		return true;

	u64 numSamples;
	bool ready = true;
	for(auto& query : mQueries)
		ready &= !query->IsBound() && query->GetResult(numSamples);

	return ready;
}

u32 VulkanOcclusionQuery::GetSampleCount()
{
	if(!mQueryFinalized)
	{
		u64 totalNumSamples = 0;
		bool ready = true;
		for(auto& query : mQueries)
		{
			u64 numSamples = 0;
			ready &= !query->IsBound() && query->GetResult(numSamples);

			totalNumSamples += numSamples;
		}

		if(ready)
		{
			mQueryFinalized = true;
			mNumSamples = totalNumSamples;

			VulkanQueryPool& queryPool = mDevice.GetQueryPool();
			for(auto& query : mQueries)
				queryPool.ReleaseQuery(*query);

			mQueries.clear();
		}
	}

	if(mBinary)
		return mNumSamples == 0 ? 0 : 1;

	return (u32)mNumSamples;
}

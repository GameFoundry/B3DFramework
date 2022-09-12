//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanOcclusionQuery.h"
#include "BsVulkanDevice.h"
#include "Managers/BsVulkanQueryManager.h"
#include "BsVulkanRenderAPI.h"
#include "BsVulkanCommandBuffer.h"
#include "Profiling/BsRenderStats.h"

namespace bs { namespace ct
{
	VulkanOcclusionQuery::VulkanOcclusionQuery(VulkanDevice& device, bool binary)
		: OcclusionQuery(binary), mDevice(device), mQueryEndCalled(false), mQueryFinalized(false)
	{
		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_Query);
	}

	VulkanOcclusionQuery::~VulkanOcclusionQuery()
	{
		for(auto& query : mQueries)
			mDevice.GetQueryPool().releaseQuery(query);

		mQueries.Clear();

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_Query);
	}

	void VulkanOcclusionQuery::Begin(const SPtr<CommandBuffer>& cb)
	{
		VulkanQueryPool& queryPool = mDevice.GetQueryPool();

		// Clear any existing queries
		for (auto& query : mQueries)
			mDevice.GetQueryPool().releaseQuery(query);

		mQueries.Clear();

		mQueryEndCalled = false;
		mNumSamples = 0;

		// Retrieve and queue new query
		VulkanCommandBuffer* vulkanCB;
		if (cb != nullptr)
			vulkanCB = static_cast<VulkanCommandBuffer*>(cb.Get());
		else
			vulkanCB = static_cast<VulkanCommandBuffer*>(gVulkanRenderAPI()._getMainCommandBuffer());

		VulkanCmdBuffer* internalCB = vulkanCB->GetInternal();
		mQueries.push_back(queryPool.BeginOcclusionQuery(internalCB, !mBinary));
		internalCB->RegisterQuery(this);

		setActive(true);
	}

	void VulkanOcclusionQuery::End(const SPtr<CommandBuffer>& cb)
	{
		if(mQueries.empty())
		{
			BS_LOG(Error, RenderBackend, "end() called but query was never started.");
			return;
		}

		// Could have been interrupted
		if (mQueryEndCalled)
			return;

		mQueryEndCalled = true;
		mQueryFinalized = false;

		VulkanCommandBuffer* vulkanCB;
		if (cb != nullptr)
			vulkanCB = static_cast<VulkanCommandBuffer*>(cb.Get());
		else
			vulkanCB = static_cast<VulkanCommandBuffer*>(gVulkanRenderAPI()._getMainCommandBuffer());

		VulkanQueryPool& queryPool = mDevice.GetQueryPool();
		VulkanCmdBuffer* internalCB = vulkanCB->GetInternal();
		queryPool.EndOcclusionQuery(mQueries.back(), internalCB);
	}

	bool VulkanOcclusionQuery::_isInProgress() const
	{
		return !mQueries.empty() && !mQueryEndCalled;
	}

	void VulkanOcclusionQuery::_interrupt(VulkanCmdBuffer& cb)
	{
		assert(!mQueries.empty() && !mQueryEndCalled);

		mQueryEndCalled = true;
		mQueryFinalized = false;

		VulkanQueryPool& queryPool = mDevice.GetQueryPool();
		queryPool.EndOcclusionQuery(mQueries.back(), &cb);
	}

	bool VulkanOcclusionQuery::IsReady() const
	{
		if (!mQueryEndCalled)
			return false;

		if (mQueryFinalized)
			return true;

		UINT64 numSamples;
		bool ready = true;
		for (auto& query : mQueries)
			ready &= !query->IsBound() && query->getResult(numSamples);

		return ready;
	}

	UINT32 VulkanOcclusionQuery::GetNumSamples()
	{
		if(!mQueryFinalized)
		{
			UINT64 totalNumSamples = 0;
			bool ready = true;
			for (auto& query : mQueries)
			{
				UINT64 numSamples = 0;
				ready &= !query->IsBound() && query->getResult(numSamples);

				totalNumSamples += numSamples;
			}

			if(ready)
			{
				mQueryFinalized = true;
				mNumSamples = totalNumSamples;

				VulkanQueryPool& queryPool = mDevice.GetQueryPool();
				for (auto& query : mQueries)
					queryPool.ReleaseQuery(query);

				mQueries.Clear();
			}
		}

		if (mBinary)
			return mNumSamples == 0 ? 0 : 1;

		return (UINT32)mNumSamples;
	}
}}

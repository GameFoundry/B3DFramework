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
		, mQueryInterrupted(false)
	{
		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_Query);
	}

	VulkanOcclusionQuery::~VulkanOcclusionQuery()
	{
		for(auto& query : mQueries)
			mDevice.getQueryPool().releaseQuery(query);

		mQueries.clear();

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_Query);
	}

	void VulkanOcclusionQuery::begin(const SPtr<CommandBuffer>& cb)
	{
		VulkanQueryPool& queryPool = mDevice.getQueryPool();

		// Clear any existing queries
		for (auto& query : mQueries)
			mDevice.getQueryPool().releaseQuery(query);

		mQueries.clear();

		mQueryEndCalled = false;
		mNumSamples = 0;

		// Retrieve and queue new query
		VulkanCommandBuffer* vulkanCB;
		if (cb != nullptr)
			vulkanCB = static_cast<VulkanCommandBuffer*>(cb.get());
		else
			vulkanCB = static_cast<VulkanCommandBuffer*>(gVulkanRenderAPI()._getMainCommandBuffer());

		VulkanCmdBuffer* internalCB = vulkanCB->getInternal();
		mQueries.push_back(queryPool.beginOcclusionQuery(internalCB, !mBinary));
		internalCB->registerQuery(this);

		setActive(true);
	}

	void VulkanOcclusionQuery::end(const SPtr<CommandBuffer>& cb)
	{
		assert(!mQueryInterrupted);

		if(mQueries.size() == 0)
		{
			LOGERR("end() called but query was never started.");
			return;
		}

		mQueryEndCalled = true;
		mQueryFinalized = false;

		VulkanCommandBuffer* vulkanCB;
		if (cb != nullptr)
			vulkanCB = static_cast<VulkanCommandBuffer*>(cb.get());
		else
			vulkanCB = static_cast<VulkanCommandBuffer*>(gVulkanRenderAPI()._getMainCommandBuffer());

		VulkanQueryPool& queryPool = mDevice.getQueryPool();
		VulkanCmdBuffer* internalCB = vulkanCB->getInternal();
		queryPool.endOcclusionQuery(mQueries.back(), internalCB);
	}

	bool VulkanOcclusionQuery::_isInProgress() const
	{
		return !mQueries.empty() && !mQueryEndCalled;
	}

	void VulkanOcclusionQuery::_interrupt(VulkanCmdBuffer& cb)
	{
		assert(mQueries.size() != 0 && !mQueryEndCalled);

		VulkanQueryPool& queryPool = mDevice.getQueryPool();
		queryPool.endOcclusionQuery(mQueries.back(), &cb);

		mQueryInterrupted = true;
	}

	void VulkanOcclusionQuery::_resume(VulkanCmdBuffer& cb)
	{
		assert(mQueryInterrupted);

		VulkanQueryPool& queryPool = mDevice.getQueryPool();
		mQueries.push_back(queryPool.beginOcclusionQuery(&cb, !mBinary));
		cb.registerQuery(this);

		mQueryInterrupted = false;
	}

	bool VulkanOcclusionQuery::isReady() const
	{
		if (!mQueryEndCalled)
			return false;

		if (mQueryFinalized)
			return true;

		UINT64 numSamples;
		bool ready = true;
		for (auto& query : mQueries)
			ready &= !query->isBound() && query->getResult(numSamples);

		return ready;
	}

	UINT32 VulkanOcclusionQuery::getNumSamples()
	{
		if(!mQueryFinalized)
		{
			UINT64 totalNumSamples = 0;
			bool ready = true;
			for (auto& query : mQueries)
			{
				UINT64 numSamples = 0;
				ready &= !query->isBound() && query->getResult(numSamples);

				totalNumSamples += numSamples;
			}

			if(ready)
			{
				mQueryFinalized = true;
				mNumSamples = totalNumSamples;

				VulkanQueryPool& queryPool = mDevice.getQueryPool();
				for (auto& query : mQueries)
					queryPool.releaseQuery(query);

				mQueries.clear();
			}
		}

		if (mBinary)
			return mNumSamples == 0 ? 0 : 1;

		return (UINT32)mNumSamples;
	}
}}
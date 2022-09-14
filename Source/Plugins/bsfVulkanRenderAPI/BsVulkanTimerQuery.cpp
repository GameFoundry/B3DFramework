//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsVulkanTimerQuery.h"
#include "BsVulkanDevice.h"
#include "Managers/BsVulkanQueryManager.h"
#include "BsVulkanRenderAPI.h"
#include "BsVulkanCommandBuffer.h"
#include "Profiling/BsRenderStats.h"

namespace bs { namespace ct
{
	VulkanTimerQuery::VulkanTimerQuery(VulkanDevice& device)
		: mDevice(device), mQueryEndCalled(false), mQueryFinalized(false)
	{
		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_Query);
	}

	VulkanTimerQuery::~VulkanTimerQuery()
	{
		for (auto& query : mQueries)
		{
			if(query.first != nullptr)
				mDevice.getQueryPool().releaseQuery(query.first);

			if (query.second != nullptr)
				mDevice.getQueryPool().releaseQuery(query.second);
		}

		mQueries.clear();

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_Query);
	}

	void VulkanTimerQuery::begin(const SPtr<CommandBuffer>& cb)
	{
		VulkanQueryPool& queryPool = mDevice.getQueryPool();

		// Clear any existing queries
		for (auto& query : mQueries)
		{
			if (query.first != nullptr)
				queryPool.releaseQuery(query.first);

			if (query.second != nullptr)
				queryPool.releaseQuery(query.second);
		}

		mQueries.clear();

		mQueryEndCalled = false;
		mTimeDelta = 0.0f;

		// Retrieve and queue new query
		VulkanCommandBuffer* vulkanCB;
		if (cb != nullptr)
			vulkanCB = static_cast<VulkanCommandBuffer*>(cb.get());
		else
			vulkanCB = static_cast<VulkanCommandBuffer*>(gVulkanRenderAPI().GetMainCommandBufferInternal());

		VulkanCmdBuffer* internalCB = vulkanCB->getInternal();
		VulkanQuery* beginQuery = queryPool.beginTimerQuery(internalCB);
		internalCB->registerQuery(this);

		mQueries.push_back(std::make_pair(beginQuery, nullptr));

		setActive(true);
	}

	void VulkanTimerQuery::end(const SPtr<CommandBuffer>& cb)
	{
		if (mQueries.empty())
		{
			BS_LOG(Error, RenderBackend, "end() called but query was never started.");
			return;
		}

		mQueryEndCalled = true;
		mQueryFinalized = false;

		VulkanCommandBuffer* vulkanCB;
		if (cb != nullptr)
			vulkanCB = static_cast<VulkanCommandBuffer*>(cb.get());
		else
			vulkanCB = static_cast<VulkanCommandBuffer*>(gVulkanRenderAPI().GetMainCommandBufferInternal());

		VulkanQueryPool& queryPool = mDevice.getQueryPool();
		VulkanCmdBuffer* internalCB = vulkanCB->getInternal();
		VulkanQuery* endQuery = queryPool.beginTimerQuery(internalCB);
		internalCB->registerQuery(this);

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

		VulkanQueryPool& queryPool = mDevice.getQueryPool();
		VulkanQuery* endQuery = queryPool.beginTimerQuery(&cb);
		cb.registerQuery(this);

		mQueries.back().second = endQuery;
	}

	bool VulkanTimerQuery::isReady() const
	{
		if (!mQueryEndCalled)
			return false;

		if (mQueryFinalized)
			return true;

		UINT64 timeBegin, timeEnd;
		bool ready = true;
		for (auto& entry : mQueries)
		{
			ready &= !entry.first->isBound() && entry.first->getResult(timeBegin);
			ready &= !entry.second->isBound() && entry.second->getResult(timeEnd);
		}

		return ready;
	}

	float VulkanTimerQuery::getTimeMs()
	{
		if (!mQueryFinalized)
		{
			UINT64 totalTimeDiff = 0;
			bool ready = true;
			for (auto& entry : mQueries)
			{
				UINT64 timeBegin = 0;
				UINT64 timeEnd = 0;
				ready &= !entry.first->isBound() && entry.first->getResult(timeBegin);
				ready &= !entry.second->isBound() && entry.second->getResult(timeEnd);

				totalTimeDiff += (timeEnd - timeBegin);
			}

			if (ready)
			{
				mQueryFinalized = true;

				double timestampToMs = (double)mDevice.getDeviceProperties().limits.timestampPeriod / 1e6; // Nano to milli
				mTimeDelta = (float)((double)totalTimeDiff * timestampToMs);

				VulkanQueryPool& queryPool = mDevice.getQueryPool();
				for (auto& query : mQueries)
				{
					if (query.first != nullptr)
						queryPool.releaseQuery(query.first);

					if (query.second != nullptr)
						queryPool.releaseQuery(query.second);
				}

				mQueries.clear();
			}
		}

		return mTimeDelta;
	}
}}

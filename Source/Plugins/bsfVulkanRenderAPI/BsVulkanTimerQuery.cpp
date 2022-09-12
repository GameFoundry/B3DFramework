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
		: MDevice(device), mQueryEndCalled(false), mQueryFinalized(false)
	{
		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_Query);
	}

	VulkanTimerQuery::~VulkanTimerQuery()
	{
		for (auto& query : mQueries)
		{
			if(query.first != nullptr)
				mDevice.GetQueryPool().releaseQuery(query.first);

			if (query.second != nullptr)
				mDevice.GetQueryPool().releaseQuery(query.second);
		}

		mQueries.Clear();

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_Query);
	}

	void VulkanTimerQuery::Begin(const SPtr<CommandBuffer>& cb)
	{
		VulkanQueryPool& queryPool = mDevice.GetQueryPool();

		// Clear any existing queries
		for (auto& query : mQueries)
		{
			if (query.first != nullptr)
				queryPool.ReleaseQuery(query.first);

			if (query.second != nullptr)
				queryPool.ReleaseQuery(query.second);
		}

		mQueries.Clear();

		mQueryEndCalled = false;
		mTimeDelta = 0.0f;

		// Retrieve and queue new query
		VulkanCommandBuffer* vulkanCB;
		if (cb != nullptr)
			vulkanCB = static_cast<VulkanCommandBuffer*>(cb.Get());
		else
			vulkanCB = static_cast<VulkanCommandBuffer*>(gVulkanRenderAPI()._getMainCommandBuffer());

		VulkanCmdBuffer* internalCB = vulkanCB->GetInternal();
		VulkanQuery* beginQuery = queryPool.BeginTimerQuery(internalCB);
		internalCB->RegisterQuery(this);

		mQueries.push_back(std::make_pair(beginQuery, nullptr));

		setActive(true);
	}

	void VulkanTimerQuery::End(const SPtr<CommandBuffer>& cb)
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
			vulkanCB = static_cast<VulkanCommandBuffer*>(cb.Get());
		else
			vulkanCB = static_cast<VulkanCommandBuffer*>(gVulkanRenderAPI()._getMainCommandBuffer());

		VulkanQueryPool& queryPool = mDevice.GetQueryPool();
		VulkanCmdBuffer* internalCB = vulkanCB->GetInternal();
		VulkanQuery* endQuery = queryPool.BeginTimerQuery(internalCB);
		internalCB->RegisterQuery(this);

		mQueries.back().second = endQuery;
	}

	bool VulkanTimerQuery::_isInProgress() const
	{
		return !mQueries.empty() && !mQueryEndCalled;
	}

	void VulkanTimerQuery::_interrupt(VulkanCmdBuffer& cb)
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
		if (!mQueryEndCalled)
			return false;

		if (mQueryFinalized)
			return true;

		UINT64 timeBegin, timeEnd;
		bool ready = true;
		for (auto& entry : mQueries)
		{
			ready &= !entry.first->IsBound() && entry.first->getResult(timeBegin);
			ready &= !entry.second->IsBound() && entry.second->getResult(timeEnd);
		}

		return ready;
	}

	float VulkanTimerQuery::GetTimeMs()
	{
		if (!mQueryFinalized)
		{
			UINT64 totalTimeDiff = 0;
			bool ready = true;
			for (auto& entry : mQueries)
			{
				UINT64 timeBegin = 0;
				UINT64 timeEnd = 0;
				ready &= !entry.first->IsBound() && entry.first->getResult(timeBegin);
				ready &= !entry.second->IsBound() && entry.second->getResult(timeEnd);

				totalTimeDiff += (timeEnd - timeBegin);
			}

			if (ready)
			{
				mQueryFinalized = true;

				double timestampToMs = (double)mDevice.GetDeviceProperties().limits.timestampPeriod / 1e6; // Nano to milli
				mTimeDelta = (float)((double)totalTimeDiff * timestampToMs);

				VulkanQueryPool& queryPool = mDevice.GetQueryPool();
				for (auto& query : mQueries)
				{
					if (query.first != nullptr)
						queryPool.ReleaseQuery(query.first);

					if (query.second != nullptr)
						queryPool.ReleaseQuery(query.second);
				}

				mQueries.Clear();
			}
		}

		return mTimeDelta;
	}
}}

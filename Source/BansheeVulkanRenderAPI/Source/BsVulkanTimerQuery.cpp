//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanTimerQuery.h"
#include "BsVulkanDevice.h"
#include "BsVulkanQueryManager.h"
#include "BsVulkanRenderAPI.h"
#include "BsVulkanCommandBuffer.h"
#include "BsRenderStats.h"

namespace bs
{
	VulkanTimerQuery::VulkanTimerQuery(VulkanDevice& device)
		: mDevice(device), mBeginQuery(nullptr), mEndQuery(nullptr), mTimeDelta(0.0f), mQueryEndCalled(false)
		, mQueryFinalized(false)
	{
		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_Query);
	}

	VulkanTimerQuery::~VulkanTimerQuery()
	{
		if (mBeginQuery != nullptr)
			mDevice.getQueryPool().releaseQuery(mBeginQuery);

		if (mEndQuery != nullptr)
			mDevice.getQueryPool().releaseQuery(mEndQuery);

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_Query);
	}

	void VulkanTimerQuery::begin(const SPtr<CommandBuffer>& cb)
	{
		VulkanQueryPool& queryPool = mDevice.getQueryPool();

		// Clear any existing queries
		if (mBeginQuery != nullptr)
		{
			queryPool.releaseQuery(mBeginQuery);
			mBeginQuery = nullptr;
		}

		if (mEndQuery != nullptr)
		{
			queryPool.releaseQuery(mEndQuery);
			mEndQuery = nullptr;
		}

		mQueryEndCalled = false;
		mTimeDelta = 0.0f;

		// Retrieve and queue new query
		VulkanCommandBuffer* vulkanCB;
		if (cb != nullptr)
			vulkanCB = static_cast<VulkanCommandBuffer*>(cb.get());
		else
			vulkanCB = static_cast<VulkanCommandBuffer*>(gVulkanRenderAPI()._getMainCommandBuffer());

		VulkanCmdBuffer* internalCB = vulkanCB->getInternal();
		mBeginQuery = queryPool.beginTimerQuery(internalCB);

		setActive(true);
	}

	void VulkanTimerQuery::end(const SPtr<CommandBuffer>& cb)
	{
		if (mBeginQuery == nullptr)
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
		mEndQuery = queryPool.beginTimerQuery(internalCB);
	}

	bool VulkanTimerQuery::isReady() const
	{
		if (!mQueryEndCalled)
			return false;

		if (mQueryFinalized)
			return true;

		UINT64 timeBegin;
		bool beginReady = !mBeginQuery->isBound() && mBeginQuery->getResult(timeBegin);

		UINT64 timeEnd;
		bool endReady = !mEndQuery->isBound() && mEndQuery->getResult(timeEnd);

		return beginReady && endReady;
	}

	float VulkanTimerQuery::getTimeMs()
	{
		if (!mQueryFinalized)
		{
			UINT64 timeBegin;
			bool beginReady = !mBeginQuery->isBound() && mBeginQuery->getResult(timeBegin);

			UINT64 timeEnd;
			bool endReady = !mEndQuery->isBound() && mEndQuery->getResult(timeEnd);

			if (beginReady && endReady)
			{
				mQueryFinalized = true;

				UINT64 timeDiff = timeEnd - timeBegin;
				double timestampToMs = (double)mDevice.getDeviceProperties().limits.timestampPeriod / 1e6; // Nano to milli

				mTimeDelta = (float)((double)timeDiff * timestampToMs);

				VulkanQueryPool& queryPool = mDevice.getQueryPool();
				queryPool.releaseQuery(mBeginQuery);
				mBeginQuery = nullptr;

				queryPool.releaseQuery(mEndQuery);
				mEndQuery = nullptr;
			}
		}

		return mTimeDelta;
	}
}
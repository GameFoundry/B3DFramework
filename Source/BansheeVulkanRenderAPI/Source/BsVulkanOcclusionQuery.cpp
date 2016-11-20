//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanOcclusionQuery.h"
#include "BsVulkanDevice.h"
#include "BsVulkanQueryManager.h"
#include "BsVulkanRenderAPI.h"
#include "BsVulkanCommandBuffer.h"
#include "BsRenderStats.h"

namespace bs
{
	VulkanOcclusionQuery::VulkanOcclusionQuery(VulkanDevice& device, bool binary)
		: OcclusionQuery(binary), mDevice(device), mQuery(nullptr), mNumSamples(0), mQueryEndCalled(false)
		, mQueryFinalized(false)
	{
		BS_INC_RENDER_STAT_CAT(ResCreated, RenderStatObject_Query);
	}

	VulkanOcclusionQuery::~VulkanOcclusionQuery()
	{
		if (mQuery != nullptr)
			mDevice.getQueryPool().releaseQuery(mQuery);

		BS_INC_RENDER_STAT_CAT(ResDestroyed, RenderStatObject_Query);
	}

	void VulkanOcclusionQuery::begin(const SPtr<CommandBuffer>& cb)
	{
		VulkanQueryPool& queryPool = mDevice.getQueryPool();

		// Clear any existing query
		if (mQuery != nullptr)
		{
			queryPool.releaseQuery(mQuery);
			mQuery = nullptr;
		}

		mQueryEndCalled = false;
		mNumSamples = 0;

		// Retrieve and queue new query
		VulkanCommandBuffer* vulkanCB;
		if (cb != nullptr)
			vulkanCB = static_cast<VulkanCommandBuffer*>(cb.get());
		else
			vulkanCB = static_cast<VulkanCommandBuffer*>(gVulkanRenderAPI()._getMainCommandBuffer());

		VulkanCmdBuffer* internalCB = vulkanCB->getInternal();
		mQuery = queryPool.beginOcclusionQuery(internalCB, !mBinary);

		setActive(true);
	}

	void VulkanOcclusionQuery::end(const SPtr<CommandBuffer>& cb)
	{
		if(mQuery == nullptr)
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
		queryPool.endOcclusionQuery(mQuery, internalCB);
	}

	bool VulkanOcclusionQuery::isReady() const
	{
		if (!mQueryEndCalled)
			return false;

		if (mQueryFinalized)
			return true;

		UINT64 numSamples;
		return !mQuery->isBound() && mQuery->getResult(numSamples);
	}

	UINT32 VulkanOcclusionQuery::getNumSamples()
	{
		if(!mQueryFinalized)
		{
			UINT64 numSamples;
			if(!mQuery->isBound() && mQuery->getResult(numSamples))
			{
				mQueryFinalized = true;
				mNumSamples = numSamples;

				VulkanQueryPool& queryPool = mDevice.getQueryPool();
				queryPool.releaseQuery(mQuery);
				mQuery = nullptr;
			}
		}

		return (UINT32)mNumSamples;
	}
}
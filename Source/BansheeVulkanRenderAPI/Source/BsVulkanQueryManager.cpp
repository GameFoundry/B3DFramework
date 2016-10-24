//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanQueryManager.h"
#include "BsVulkanEventQuery.h"
#include "BsVulkanTimerQuery.h"
#include "BsVulkanOcclusionQuery.h"

namespace BansheeEngine
{
	SPtr<EventQuery> VulkanQueryManager::createEventQuery(UINT32 deviceIdx) const
	{
		SPtr<EventQuery> query = SPtr<VulkanEventQuery>(bs_new<VulkanEventQuery>(deviceIdx), 
			&QueryManager::deleteEventQuery, StdAlloc<VulkanEventQuery>());
		mEventQueries.push_back(query.get());

		return query;
	}

	SPtr<TimerQuery> VulkanQueryManager::createTimerQuery(UINT32 deviceIdx) const
	{
		SPtr<TimerQuery> query = SPtr<VulkanTimerQuery>(bs_new<VulkanTimerQuery>(deviceIdx), 
			&QueryManager::deleteTimerQuery, StdAlloc<VulkanTimerQuery>());
		mTimerQueries.push_back(query.get());

		return query;
	}

	SPtr<OcclusionQuery> VulkanQueryManager::createOcclusionQuery(bool binary, UINT32 deviceIdx) const
	{
		SPtr<OcclusionQuery> query = SPtr<VulkanOcclusionQuery>(bs_new<VulkanOcclusionQuery>(binary, deviceIdx),
			&QueryManager::deleteOcclusionQuery, StdAlloc<VulkanOcclusionQuery>());
		mOcclusionQueries.push_back(query.get());

		return query;
	}
}
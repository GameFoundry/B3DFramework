//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsD3D11QueryManager.h"
#include "BsD3D11EventQuery.h"
#include "BsD3D11TimerQuery.h"
#include "BsD3D11OcclusionQuery.h"

namespace BansheeEngine
{
	SPtr<EventQuery> D3D11QueryManager::createEventQuery(UINT32 deviceIdx) const
	{
		SPtr<EventQuery> query = SPtr<D3D11EventQuery>(bs_new<D3D11EventQuery>(deviceIdx), &QueryManager::deleteEventQuery, StdAlloc<D3D11EventQuery>());
		mEventQueries.push_back(query.get());

		return query;
	}

	SPtr<TimerQuery> D3D11QueryManager::createTimerQuery(UINT32 deviceIdx) const
	{
		SPtr<TimerQuery> query = SPtr<D3D11TimerQuery>(bs_new<D3D11TimerQuery>(deviceIdx), &QueryManager::deleteTimerQuery, StdAlloc<D3D11TimerQuery>());
		mTimerQueries.push_back(query.get());

		return query;
	}

	SPtr<OcclusionQuery> D3D11QueryManager::createOcclusionQuery(bool binary, UINT32 deviceIdx) const
	{
		SPtr<OcclusionQuery> query = SPtr<D3D11OcclusionQuery>(bs_new<D3D11OcclusionQuery>(binary, deviceIdx), 
			&QueryManager::deleteOcclusionQuery, StdAlloc<D3D11OcclusionQuery>());
		mOcclusionQueries.push_back(query.get());

		return query;
	}
}
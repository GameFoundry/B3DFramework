//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsD3D11QueryManager.h"
#include "BsD3D11EventQuery.h"
#include "BsD3D11TimerQuery.h"
#include "BsD3D11OcclusionQuery.h"

namespace bs
{
	namespace ct
	{
		SPtr<EventQuery> D3D11QueryManager::CreateEventQuery(u32 deviceIdx) const
		{
			SPtr<EventQuery> query = SPtr<D3D11EventQuery>(bs_new<D3D11EventQuery>(deviceIdx), &QueryManager::DeleteEventQuery, StdAlloc<D3D11EventQuery>());
			mEventQueries.push_back(query.get());

			return query;
		}

		SPtr<TimerQuery> D3D11QueryManager::CreateTimerQuery(u32 deviceIdx) const
		{
			SPtr<TimerQuery> query = SPtr<D3D11TimerQuery>(bs_new<D3D11TimerQuery>(deviceIdx), &QueryManager::DeleteTimerQuery, StdAlloc<D3D11TimerQuery>());
			mTimerQueries.push_back(query.get());

			return query;
		}

		SPtr<OcclusionQuery> D3D11QueryManager::CreateOcclusionQuery(bool binary, u32 deviceIdx) const
		{
			SPtr<OcclusionQuery> query = SPtr<D3D11OcclusionQuery>(bs_new<D3D11OcclusionQuery>(binary, deviceIdx), &QueryManager::DeleteOcclusionQuery, StdAlloc<D3D11OcclusionQuery>());
			mOcclusionQueries.push_back(query.get());

			return query;
		}
	} // namespace ct
} // namespace bs

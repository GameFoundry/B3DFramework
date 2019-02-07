//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsNullQueries.h"

namespace bs { namespace ct
{
	SPtr<EventQuery> NullQueryManager::createEventQuery(UINT32 deviceIdx) const
	{
		SPtr<EventQuery> query = SPtr<NullEventQuery>(bs_new<NullEventQuery>(), &QueryManager::deleteEventQuery, 
			StdAlloc<NullEventQuery>());
		mEventQueries.push_back(query.get());

		return query;
	}

	SPtr<TimerQuery> NullQueryManager::createTimerQuery(UINT32 deviceIdx) const
	{
		SPtr<TimerQuery> query = SPtr<NullTimerQuery>(bs_new<NullTimerQuery>(), &QueryManager::deleteTimerQuery, 
			StdAlloc<NullTimerQuery>());
		mTimerQueries.push_back(query.get());

		return query;
	}

	SPtr<OcclusionQuery> NullQueryManager::createOcclusionQuery(bool binary, UINT32 deviceIdx) const
	{
		SPtr<OcclusionQuery> query = SPtr<NullOcclusionQuery>(bs_new<NullOcclusionQuery>(binary), 
			&QueryManager::deleteOcclusionQuery, StdAlloc<NullOcclusionQuery>());
		mOcclusionQueries.push_back(query.get());

		return query;
	}
}}

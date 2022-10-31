//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsNullQueries.h"

using namespace bs;
using namespace bs::ct;

SPtr<EventQuery> NullQueryManager::CreateEventQuery(u32 deviceIdx) const
{
	SPtr<EventQuery> query = SPtr<NullEventQuery>(B3DNew<NullEventQuery>(), &QueryManager::DeleteEventQuery, StdAlloc<NullEventQuery>());
	mEventQueries.push_back(query.get());

	return query;
}

SPtr<TimerQuery> NullQueryManager::CreateTimerQuery(u32 deviceIdx) const
{
	SPtr<TimerQuery> query = SPtr<NullTimerQuery>(B3DNew<NullTimerQuery>(), &QueryManager::DeleteTimerQuery, StdAlloc<NullTimerQuery>());
	mTimerQueries.push_back(query.get());

	return query;
}

SPtr<OcclusionQuery> NullQueryManager::CreateOcclusionQuery(bool binary, u32 deviceIdx) const
{
	SPtr<OcclusionQuery> query = SPtr<NullOcclusionQuery>(B3DNew<NullOcclusionQuery>(binary), &QueryManager::DeleteOcclusionQuery, StdAlloc<NullOcclusionQuery>());
	mOcclusionQueries.push_back(query.get());

	return query;
}

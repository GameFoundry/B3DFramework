//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsGLQueryManager.h"
#include "BsGLEventQuery.h"
#include "BsGLTimerQuery.h"
#include "BsGLOcclusionQuery.h"

using namespace bs;
using namespace bs::ct;

SPtr<EventQuery> GLQueryManager::CreateEventQuery(u32 deviceIdx) const
{
	SPtr<EventQuery> query = SPtr<GLEventQuery>(B3DNew<GLEventQuery>(deviceIdx), &QueryManager::DeleteEventQuery, StdAlloc<GLEventQuery>());
	mEventQueries.push_back(query.get());

	return query;
}

SPtr<TimerQuery> GLQueryManager::CreateTimerQuery(u32 deviceIdx) const
{
	SPtr<TimerQuery> query = SPtr<GLTimerQuery>(B3DNew<GLTimerQuery>(deviceIdx), &QueryManager::DeleteTimerQuery, StdAlloc<GLTimerQuery>());
	mTimerQueries.push_back(query.get());

	return query;
}

SPtr<OcclusionQuery> GLQueryManager::CreateOcclusionQuery(bool binary, u32 deviceIdx) const
{
	SPtr<OcclusionQuery> query = SPtr<GLOcclusionQuery>(B3DNew<GLOcclusionQuery>(binary, deviceIdx), &QueryManager::DeleteOcclusionQuery, StdAlloc<GLOcclusionQuery>());
	mOcclusionQueries.push_back(query.get());

	return query;
}

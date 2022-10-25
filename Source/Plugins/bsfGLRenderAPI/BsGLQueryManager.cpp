//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsGLQueryManager.h"
#include "BsGLEventQuery.h"
#include "BsGLTimerQuery.h"
#include "BsGLOcclusionQuery.h"

namespace bs
{
	namespace ct
	{
		SPtr<EventQuery> GLQueryManager::CreateEventQuery(u32 deviceIdx) const
		{
			SPtr<EventQuery> query = SPtr<GLEventQuery>(bs_new<GLEventQuery>(deviceIdx), &QueryManager::DeleteEventQuery, StdAlloc<GLEventQuery>());
			mEventQueries.push_back(query.get());

			return query;
		}

		SPtr<TimerQuery> GLQueryManager::CreateTimerQuery(u32 deviceIdx) const
		{
			SPtr<TimerQuery> query = SPtr<GLTimerQuery>(bs_new<GLTimerQuery>(deviceIdx), &QueryManager::DeleteTimerQuery, StdAlloc<GLTimerQuery>());
			mTimerQueries.push_back(query.get());

			return query;
		}

		SPtr<OcclusionQuery> GLQueryManager::CreateOcclusionQuery(bool binary, u32 deviceIdx) const
		{
			SPtr<OcclusionQuery> query = SPtr<GLOcclusionQuery>(bs_new<GLOcclusionQuery>(binary, deviceIdx), &QueryManager::DeleteOcclusionQuery, StdAlloc<GLOcclusionQuery>());
			mOcclusionQueries.push_back(query.get());

			return query;
		}
	} // namespace ct
} // namespace bs

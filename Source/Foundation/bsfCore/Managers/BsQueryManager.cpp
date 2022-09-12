//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/BsQueryManager.h"
#include "RenderAPI/BsEventQuery.h"
#include "RenderAPI/BsTimerQuery.h"
#include "RenderAPI/BsOcclusionQuery.h"
#include "Profiling/BsProfilerCPU.h"

namespace bs { namespace ct
{
	QueryManager::~QueryManager()
	{
		// Trigger all remaining queries, whether they completed or not

		for (auto& query : mEventQueries)
		{
			if (query->IsActive())
				query->OnTriggered();
		}

		for (auto& query : mTimerQueries)
		{
			if (query->IsActive())
				query->OnTriggered(query->getTimeMs());
		}

		for (auto& query : mOcclusionQueries)
		{
			if (query->IsActive())
				query->OnComplete(query->getNumSamples());
		}

		processDeletedQueue();
	}

	void QueryManager::_update()
	{
		for(auto& query : mEventQueries)
		{
			if(query->IsActive() && query->isReady())
			{
				query->OnTriggered();
				query->SetActive(false);
			}
		}

		for(auto& query : mTimerQueries)
		{
			if(query->IsActive() && query->isReady())
			{
				query->OnTriggered(query->getTimeMs());
				query->SetActive(false);
			}
		}

		for (auto& query : mOcclusionQueries)
		{
			if (query->IsActive() && query->isReady())
			{
				query->OnComplete(query->getNumSamples());
				query->SetActive(false);
			}
		}

		processDeletedQueue();
	}

	void QueryManager::DeleteEventQuery(EventQuery* query)
	{
		instance().mDeletedEventQueries.push_back(query);
	}

	void QueryManager::DeleteTimerQuery(TimerQuery* query)
	{
		instance().mDeletedTimerQueries.push_back(query);
	}

	void QueryManager::DeleteOcclusionQuery(OcclusionQuery* query)
	{
		instance().mDeletedOcclusionQueries.push_back(query);
	}

	void QueryManager::ProcessDeletedQueue()
	{
		for (auto& query : mDeletedEventQueries)
		{
			auto iterFind = std::find(mEventQueries.begin(), mEventQueries.end(), query);

			if (iterFind != mEventQueries.end())
				mEventQueries.erase(iterFind);

			bs_delete(query);
		}

		mDeletedEventQueries.Clear();

		for (auto& query : mDeletedTimerQueries)
		{
			auto iterFind = std::find(mTimerQueries.begin(), mTimerQueries.end(), query);

			if (iterFind != mTimerQueries.end())
				mTimerQueries.erase(iterFind);

			bs_delete(query);
		}

		mDeletedTimerQueries.Clear();

		for (auto& query : mDeletedOcclusionQueries)
		{
			auto iterFind = std::find(mOcclusionQueries.begin(), mOcclusionQueries.end(), query);

			if (iterFind != mOcclusionQueries.end())
				mOcclusionQueries.erase(iterFind);

			bs_delete(query);
		}

		mDeletedOcclusionQueries.Clear();
	}
}}

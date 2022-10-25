//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Managers/BsQueryManager.h"
#include "RenderAPI/BsEventQuery.h"
#include "RenderAPI/BsTimerQuery.h"
#include "RenderAPI/BsOcclusionQuery.h"
#include "Profiling/BsProfilerCPU.h"

using namespace bs;

namespace bs { namespace ct
{
QueryManager::~QueryManager()
{
	// Trigger all remaining queries, whether they completed or not

	for(auto& query : mEventQueries)
	{
		if(query->IsActive())
			query->OnTriggered();
	}

	for(auto& query : mTimerQueries)
	{
		if(query->IsActive())
			query->OnTriggered(query->GetTimeMs());
	}

	for(auto& query : mOcclusionQueries)
	{
		if(query->IsActive())
			query->OnComplete(query->GetNumSamples());
	}

	ProcessDeletedQueue();
}

void QueryManager::UpdateInternal()
{
	for(auto& query : mEventQueries)
	{
		if(query->IsActive() && query->IsReady())
		{
			query->OnTriggered();
			query->SetActive(false);
		}
	}

	for(auto& query : mTimerQueries)
	{
		if(query->IsActive() && query->IsReady())
		{
			query->OnTriggered(query->GetTimeMs());
			query->SetActive(false);
		}
	}

	for(auto& query : mOcclusionQueries)
	{
		if(query->IsActive() && query->IsReady())
		{
			query->OnComplete(query->GetNumSamples());
			query->SetActive(false);
		}
	}

	ProcessDeletedQueue();
}

void QueryManager::DeleteEventQuery(EventQuery* query)
{
	Instance().mDeletedEventQueries.push_back(query);
}

void QueryManager::DeleteTimerQuery(TimerQuery* query)
{
	Instance().mDeletedTimerQueries.push_back(query);
}

void QueryManager::DeleteOcclusionQuery(OcclusionQuery* query)
{
	Instance().mDeletedOcclusionQueries.push_back(query);
}

void QueryManager::ProcessDeletedQueue()
{
	for(auto& query : mDeletedEventQueries)
	{
		auto iterFind = std::find(mEventQueries.begin(), mEventQueries.end(), query);

		if(iterFind != mEventQueries.end())
			mEventQueries.erase(iterFind);

		bs_delete(query);
	}

	mDeletedEventQueries.clear();

	for(auto& query : mDeletedTimerQueries)
	{
		auto iterFind = std::find(mTimerQueries.begin(), mTimerQueries.end(), query);

		if(iterFind != mTimerQueries.end())
			mTimerQueries.erase(iterFind);

		bs_delete(query);
	}

	mDeletedTimerQueries.clear();

	for(auto& query : mDeletedOcclusionQueries)
	{
		auto iterFind = std::find(mOcclusionQueries.begin(), mOcclusionQueries.end(), query);

		if(iterFind != mOcclusionQueries.end())
			mOcclusionQueries.erase(iterFind);

		bs_delete(query);
	}

	mDeletedOcclusionQueries.clear();
}
}}

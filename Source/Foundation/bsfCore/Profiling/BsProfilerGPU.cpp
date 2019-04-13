//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Profiling/BsProfilerGPU.h"
#include "Profiling/BsRenderStats.h"
#include "RenderAPI/BsTimerQuery.h"
#include "RenderAPI/BsOcclusionQuery.h"
#include "Error/BsException.h"

namespace bs
{
	const UINT32 ProfilerGPU::MAX_QUEUE_ELEMENTS = 5;

	ProfilerGPU::ProfilerGPU()
	{
		mReadyReports = bs_newN<GPUProfilerReport>(MAX_QUEUE_ELEMENTS);
	}

	ProfilerGPU::~ProfilerGPU()
	{
		while (!mUnresolvedFrames.empty())
		{
			ProfiledSample& frameSample = mUnresolvedFrames.front();

			freeSample(frameSample);
			mUnresolvedFrames.pop();
		}

		bs_deleteN(mReadyReports, MAX_QUEUE_ELEMENTS);
	}

	void ProfilerGPU::beginFrame()
	{
		if (mIsFrameActive)
		{
			LOGERR("Cannot begin a frame because another frame is active.");
			return;
		}

		mFrameSample = ProfiledSample();
		mFrameSample.name = "Frame";
		beginSampleInternal(mFrameSample, true);

		mIsFrameActive = true;
	}

	void ProfilerGPU::endFrame()
	{
		if (!mActiveSamples.empty())
		{
			LOGERR("Attempting to end a frame while a sample is active.");
			return;
		}

		if (!mIsFrameActive)
			return;

		endSampleInternal(mFrameSample);

		mUnresolvedFrames.push(mFrameSample);
		mIsFrameActive = false;
	}

	void ProfilerGPU::beginSample(ProfilerString name)
	{
		if (!mIsFrameActive)
		{
			LOGERR("Cannot begin a sample because no frame is active.");
			return;
		}

		auto sample = mSamplePool.construct<ProfiledSample>();
		sample->name = std::move(name);
		beginSampleInternal(*sample, false);

		if(mActiveSamples.empty())
			mFrameSample.children.push_back(sample);
		else
		{
			ProfiledSample* parent = mActiveSamples.top();
			parent->children.push_back(sample);
		}
		
		mActiveSamples.push(sample);
	}

	void ProfilerGPU::endSample(const ProfilerString& name)
	{
		if (mActiveSamples.empty())
			return;

		ProfiledSample* lastSample = mActiveSamples.top();
		if (lastSample->name != name)
		{
			String errorStr = "Attempting to end a sample that doesn't match. Got: " +
				String(name.c_str()) + ". Expected: " + String(lastSample->name.c_str());

			LOGERR(errorStr);
			return;
		}

		endSampleInternal(*lastSample);
		mActiveSamples.pop();
	}

	UINT32 ProfilerGPU::getNumAvailableReports()
	{
		Lock lock(mMutex);

		return mReportCount;
	}

	GPUProfilerReport ProfilerGPU::getNextReport()
	{
		Lock lock(mMutex);

		if (mReportCount == 0)
		{
			LOGERR("No reports are available.")
			return GPUProfilerReport();
		}

		GPUProfilerReport report = mReadyReports[mReportHeadPos];

		mReportHeadPos = (mReportHeadPos + 1) % MAX_QUEUE_ELEMENTS;
		mReportCount--;

		return report;
	}

	void ProfilerGPU::_update()
	{
		while (!mUnresolvedFrames.empty())
		{
			ProfiledSample& frameSample = mUnresolvedFrames.front();

			// Frame sample timer query is the last query we issued
			// so if it is complete, we may assume all queries are complete.
			if (frameSample.activeTimeQuery->isReady())
			{
				GPUProfilerReport report;
				resolveSample(frameSample, report.frameSample);

				freeSample(frameSample);
				mUnresolvedFrames.pop();

				{
					Lock lock(mMutex);
					mReadyReports[(mReportHeadPos + mReportCount) % MAX_QUEUE_ELEMENTS] = report;
					if (mReportCount == MAX_QUEUE_ELEMENTS)
						mReportHeadPos = (mReportHeadPos + 1) % MAX_QUEUE_ELEMENTS;
					else
						mReportCount++;
				}
			}
			else
				break;
		}
	}

	void ProfilerGPU::freeSample(ProfiledSample& sample)
	{
		for(auto& entry : sample.children)
		{
			freeSample(*entry);
			mSamplePool.destruct(entry);
		}

		sample.children.clear();

		mFreeTimerQueries.push(sample.activeTimeQuery);

		if(sample.activeOcclusionQuery)
			mFreeOcclusionQueries.push(sample.activeOcclusionQuery);
	}

	void ProfilerGPU::resolveSample(const ProfiledSample& sample, GPUProfileSample& reportSample)
	{
		reportSample.name.assign(sample.name.data(), sample.name.size());
		reportSample.timeMs = sample.activeTimeQuery->getTimeMs();

		if(sample.activeOcclusionQuery)
			reportSample.numDrawnSamples = sample.activeOcclusionQuery->getNumSamples();
		else
			reportSample.numDrawnSamples = 0;

		reportSample.numDrawCalls = (UINT32)(sample.endStats.numDrawCalls - sample.startStats.numDrawCalls);
		reportSample.numRenderTargetChanges = (UINT32)(sample.endStats.numRenderTargetChanges - sample.startStats.numRenderTargetChanges);
		reportSample.numPresents = (UINT32)(sample.endStats.numPresents - sample.startStats.numPresents);
		reportSample.numClears = (UINT32)(sample.endStats.numClears - sample.startStats.numClears);

		reportSample.numVertices = (UINT32)(sample.endStats.numVertices - sample.startStats.numVertices);
		reportSample.numPrimitives = (UINT32)(sample.endStats.numPrimitives - sample.startStats.numPrimitives);

		reportSample.numPipelineStateChanges = (UINT32)(sample.endStats.numPipelineStateChanges - sample.startStats.numPipelineStateChanges);

		reportSample.numGpuParamBinds = (UINT32)(sample.endStats.numGpuParamBinds - sample.startStats.numGpuParamBinds);
		reportSample.numVertexBufferBinds = (UINT32)(sample.endStats.numVertexBufferBinds - sample.startStats.numVertexBufferBinds);
		reportSample.numIndexBufferBinds = (UINT32)(sample.endStats.numIndexBufferBinds - sample.startStats.numIndexBufferBinds);

		reportSample.numResourceWrites = (UINT32)(sample.endStats.numResourceWrites - sample.startStats.numResourceWrites);
		reportSample.numResourceReads = (UINT32)(sample.endStats.numResourceReads - sample.startStats.numResourceReads);

		reportSample.numObjectsCreated = (UINT32)(sample.endStats.numObjectsCreated - sample.startStats.numObjectsCreated);
		reportSample.numObjectsDestroyed = (UINT32)(sample.endStats.numObjectsDestroyed - sample.startStats.numObjectsDestroyed);

		for(auto& entry : sample.children)
		{
			reportSample.children.push_back(GPUProfileSample());
			resolveSample(*entry, reportSample.children.back());
		}
	}

	void ProfilerGPU::beginSampleInternal(ProfiledSample& sample, bool issueOcclusion)
	{
		sample.startStats = RenderStats::instance().getData();
		sample.activeTimeQuery = getTimerQuery();
		sample.activeTimeQuery->begin();

		if(issueOcclusion)
		{
			sample.activeOcclusionQuery = getOcclusionQuery();
			sample.activeOcclusionQuery->begin();
		}
	}

	void ProfilerGPU::endSampleInternal(ProfiledSample& sample)
	{
		sample.endStats = RenderStats::instance().getData();

		if(sample.activeOcclusionQuery)
			sample.activeOcclusionQuery->end();

		sample.activeTimeQuery->end();
	}

	SPtr<ct::TimerQuery> ProfilerGPU::getTimerQuery() const
	{
		if (!mFreeTimerQueries.empty())
		{
			SPtr<ct::TimerQuery> timerQuery = mFreeTimerQueries.top();
			mFreeTimerQueries.pop();

			return timerQuery;
		}

		return ct::TimerQuery::create();
	}

	SPtr<ct::OcclusionQuery> ProfilerGPU::getOcclusionQuery() const
	{
		if (!mFreeOcclusionQueries.empty())
		{
			SPtr<ct::OcclusionQuery> occlusionQuery = mFreeOcclusionQueries.top();
			mFreeOcclusionQueries.pop();

			return occlusionQuery;
		}

		return ct::OcclusionQuery::create(false);
	}

	ProfilerGPU& gProfilerGPU()
	{
		return ProfilerGPU::instance();
	}
}
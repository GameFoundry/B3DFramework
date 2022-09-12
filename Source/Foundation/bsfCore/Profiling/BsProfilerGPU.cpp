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
			ProfiledFrame& frame = mUnresolvedFrames.front();

			freeFrame(frame);
			mUnresolvedFrames.pop();
		}

		bs_deleteN(mReadyReports, MAX_QUEUE_ELEMENTS);
	}

	void ProfilerGPU::BeginFrame()
	{
		if (mIsFrameActive)
		{
			BS_LOG(Error, Profiler, "Cannot begin a frame because another frame is active.");
			return;
		}

		mIsFrameActive = true;
		mActiveFrame.uncategorizedSamples.Clear();
		mActiveFrame.viewSamples.Clear();
	}

	void ProfilerGPU::EndFrame(bool discard)
	{
		if (!mActiveSamples.empty())
		{
			BS_LOG(Error, Profiler, "Attempting to end a frame while a sample is active.");
			return;
		}

		if (mIsViewActive)
		{
			BS_LOG(Error, Profiler, "Attempting to end a frame while a view is active.");
			return;
		}

		if (!mIsFrameActive)
			return;

		if(!discard)
			mUnresolvedFrames.Push(mActiveFrame);
		else
			freeFrame(mActiveFrame);

		mIsFrameActive = false;
	}

	void ProfilerGPU::BeginView(UINT64 id, ProfilerString title)
	{
		if (!mIsFrameActive)
		{
			BS_LOG(Error, Profiler, "Cannot begin a view because no frame is active.");
			return;
		}

		if (mIsViewActive)
		{
			BS_LOG(Error, Profiler, "Cannot begin a view because another view is active.");
			return;
		}

		auto sample = mViewSamplePool.Construct<ProfiledViewSample>();
		sample->viewId = id;

		mActiveFrame.viewSamples.push_back(sample);

		beginSampleInternal(*sample, true);
		mIsViewActive = true;
	}

	void ProfilerGPU::EndView()
	{
		if (!mActiveSamples.empty())
		{
			BS_LOG(Error, Profiler, "Attempting to end a view while a sample is active.");
			return;
		}

		if (!mIsViewActive)
			return;

		endSampleInternal(*mActiveFrame.viewSamples.back());
		mIsViewActive = false;
	}

	void ProfilerGPU::BeginSample(ProfilerString name)
	{
		if (!mIsFrameActive)
		{
			BS_LOG(Error, Profiler, "Cannot begin a sample because no frame is active.");
			return;
		}

		auto sample = mSamplePool.Construct<ProfiledSample>();
		sample->name = std::move(name);
		beginSampleInternal(*sample, false);

		if (mActiveSamples.empty())
		{
			if (mIsViewActive)
				mActiveFrame.viewSamples.back()->children.push_back(sample);
			else
				mActiveFrame.uncategorizedSamples.push_back(sample);
		}
		else
		{
			ProfiledSample* parent = mActiveSamples.Top();
			parent->children.push_back(sample);
		}
		
		mActiveSamples.Push(sample);
	}

	void ProfilerGPU::EndSample(const ProfilerString& name)
	{
		if (mActiveSamples.empty())
			return;

		ProfiledSample* lastSample = mActiveSamples.Top();
		if (lastSample->name != name)
		{
			BS_LOG(Error, Profiler, "Attempting to end a sample that doesn't match. Got: {0}. Expected: {1}",
				name.c_str(), lastSample->name.c_str());
			return;
		}

		endSampleInternal(*lastSample);
		mActiveSamples.pop();
	}

	UINT32 ProfilerGPU::GetNumAvailableReports()
	{
		Lock Lock(mMutex);

		return mReportCount;
	}

	GPUProfilerReport ProfilerGPU::GetNextReport()
	{
		Lock Lock(mMutex);

		if (mReportCount == 0)
		{
			BS_LOG(Error, Profiler, "No reports are available.");
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
			ProfiledFrame& frame = mUnresolvedFrames.front();

			// Make sure all the top-level queries have finished. If they have that implies
			// all their children have finished as well
			bool isReady = true;
			for(auto& entry : frame.viewSamples)
			{
				if (!entry->activeTimeQuery->IsReady())
				{
					isReady = false;
					break;
				}
			}

			for(auto& entry : frame.uncategorizedSamples)
			{
				if (!entry->activeTimeQuery->IsReady())
				{
					isReady = false;
					break;
				}
			}

			if (!isReady)
				break;
			
			GPUProfilerReport report;
			report.viewSamples.Resize(frame.viewSamples.size());
			report.uncategorizedSamples.Resize(frame.uncategorizedSamples.size());
			
			for (size_t i = 0; i < frame.viewSamples.size(); i++)
				resolveSample(*frame.viewSamples[i], report.viewSamples[i]);
				
			for (size_t i = 0; i < frame.uncategorizedSamples.size(); i++)
				resolveSample(*frame.uncategorizedSamples[i], report.uncategorizedSamples[i]);
				
			freeFrame(frame);
			mUnresolvedFrames.pop();

			{
				Lock Lock(mMutex);
				mReadyReports[(mReportHeadPos + mReportCount) % MAX_QUEUE_ELEMENTS] = report;
				if (mReportCount == MAX_QUEUE_ELEMENTS)
					mReportHeadPos = (mReportHeadPos + 1) % MAX_QUEUE_ELEMENTS;
				else
					mReportCount++;
			}
		}
	}

	void ProfilerGPU::FreeSample(ProfiledSample& sample)
	{
		for(auto& entry : sample.children)
		{
			freeSample(*entry);
			mSamplePool.Destruct(entry);
		}

		sample.children.Clear();

		mFreeTimerQueries.Push(sample.activeTimeQuery);

		if(sample.activeOcclusionQuery)
			mFreeOcclusionQueries.Push(sample.activeOcclusionQuery);
	}

	void ProfilerGPU::FreeFrame(ProfiledFrame& frame)
	{
		for (size_t i = 0; i < frame.viewSamples.size(); i++)
		{
			freeSample(*frame.viewSamples[i]);
			mViewSamplePool.Destruct(frame.viewSamples[i]);
		}
			
		for (size_t i = 0; i < frame.uncategorizedSamples.size(); i++)
		{
			freeSample(*frame.uncategorizedSamples[i]);
			mSamplePool.Destruct(frame.uncategorizedSamples[i]);
		}

		frame.viewSamples.Clear();
		frame.uncategorizedSamples.Clear();
	}

	void ProfilerGPU::ResolveSample(const ProfiledSample& sample, GPUProfileSample& reportSample)
	{
		reportSample.name.Assign(sample.name.data(), sample.name.size());
		reportSample.timeMs = sample.activeTimeQuery->GetTimeMs();

		if(sample.activeOcclusionQuery)
			reportSample.numDrawnSamples = sample.activeOcclusionQuery->GetNumSamples();
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

	void ProfilerGPU::BeginSampleInternal(ProfiledSample& sample, bool issueOcclusion)
	{
		sample.startStats = RenderStats::instance().GetData();
		sample.activeTimeQuery = getTimerQuery();
		sample.activeTimeQuery->Begin();

		if(issueOcclusion)
		{
			sample.activeOcclusionQuery = getOcclusionQuery();
			sample.activeOcclusionQuery->Begin();
		}
	}

	void ProfilerGPU::EndSampleInternal(ProfiledSample& sample)
	{
		sample.endStats = RenderStats::instance().GetData();

		if(sample.activeOcclusionQuery)
			sample.activeOcclusionQuery->End();

		sample.activeTimeQuery->End();
	}

	SPtr<ct::TimerQuery> ProfilerGPU::GetTimerQuery() const
	{
		if (!mFreeTimerQueries.empty())
		{
			SPtr<ct::TimerQuery> timerQuery = mFreeTimerQueries.Top();
			mFreeTimerQueries.pop();

			return timerQuery;
		}

		return ct::TimerQuery::create();
	}

	SPtr<ct::OcclusionQuery> ProfilerGPU::GetOcclusionQuery() const
	{
		if (!mFreeOcclusionQueries.empty())
		{
			SPtr<ct::OcclusionQuery> occlusionQuery = mFreeOcclusionQueries.Top();
			mFreeOcclusionQueries.pop();

			return occlusionQuery;
		}

		return ct::OcclusionQuery::create(false);
	}

	ProfilerGPU& GProfilerGPU()
	{
		return ProfilerGPU::Instance();
	}
}

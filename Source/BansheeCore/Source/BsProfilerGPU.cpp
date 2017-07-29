//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsProfilerGPU.h"
#include "BsRenderStats.h"
#include "BsTimerQuery.h"
#include "BsOcclusionQuery.h"
#include "BsException.h"

namespace bs
{
	const UINT32 ProfilerGPU::MAX_QUEUE_ELEMENTS = 5;

	ProfilerGPU::ProfilerGPU()
		:mIsFrameActive(false), mReadyReports(nullptr), mReportHeadPos(0), mReportCount(0)
	{
		mReadyReports = bs_newN<GPUProfilerReport>(MAX_QUEUE_ELEMENTS);
	}

	ProfilerGPU::~ProfilerGPU()
	{
		bs_deleteN(mReadyReports, MAX_QUEUE_ELEMENTS);
	}

	void ProfilerGPU::beginFrame()
	{
		if (mIsFrameActive)
			BS_EXCEPT(InvalidStateException, "Cannot begin a frame because another frame is active.");

		mActiveFrame = ActiveFrame();

		mActiveFrame.frameSample.sampleName = "Frame";
		beginSampleInternal(mActiveFrame.frameSample);

		mIsFrameActive = true;
	}

	void ProfilerGPU::endFrame()
	{
		if (mActiveSampleIndexes.size() > 0)
			BS_EXCEPT(InvalidStateException, "Attempting to end a frame while a sample is active.");

		if (!mIsFrameActive)
			return;

		endSampleInternal(mActiveFrame.frameSample);

		mUnresolvedFrames.push(mActiveFrame);
		mIsFrameActive = false;
	}

	void ProfilerGPU::beginSample(const ProfilerString& name)
	{
		if (!mIsFrameActive)
			BS_EXCEPT(InvalidStateException, "Cannot begin a sample because no frame is active.");

		mActiveFrame.samples.push_back(ActiveSample());
		ActiveSample& sample = mActiveFrame.samples.back();

		sample.sampleName = name;
		beginSampleInternal(sample);

		mActiveSampleIndexes.push((UINT32)mActiveFrame.samples.size() - 1);
	}

	void ProfilerGPU::endSample(const ProfilerString& name)
	{
		if (mActiveSampleIndexes.size() == 0)
			return;

		UINT32 lastSampleIdx = mActiveSampleIndexes.top();
		ActiveSample& sample = mActiveFrame.samples[lastSampleIdx];
		if (sample.sampleName != name)
		{
			String errorStr = "Attempting to end a sample that doesn't match. Got: " +
				String(name.c_str()) + ". Expected: " + String(sample.sampleName.c_str());

			BS_EXCEPT(InvalidStateException, errorStr);
		}

		endSampleInternal(sample);
		mActiveSampleIndexes.pop();
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
			BS_EXCEPT(InvalidStateException, "No reports are available.")

		GPUProfilerReport report = mReadyReports[mReportHeadPos];

		mReportHeadPos = (mReportHeadPos + 1) % MAX_QUEUE_ELEMENTS;
		mReportCount--;

		return report;
	}

	void ProfilerGPU::_update()
	{
		while (!mUnresolvedFrames.empty())
		{
			ActiveFrame& frame = mUnresolvedFrames.front();

			// Frame sample timer query is the last query we issued
			// so if it is complete, we may assume all queries are complete.
			if (frame.frameSample.activeTimeQuery->isReady())
			{
				GPUProfilerReport report = resolveFrame(frame);
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

	GPUProfilerReport ProfilerGPU::resolveFrame(ActiveFrame& frame)
	{
		GPUProfilerReport report;

		resolveSample(frame.frameSample, report.frameSample);

		for (auto& sample : frame.samples)
		{
			report.samples.push_back(GPUProfileSample());
			GPUProfileSample& newSample = report.samples.back();

			resolveSample(sample, newSample);
		}

		return report;
	}

	void ProfilerGPU::resolveSample(const ActiveSample& sample, GPUProfileSample& reportSample)
	{
		reportSample.name = String(sample.sampleName.c_str());
		reportSample.timeMs = sample.activeTimeQuery->getTimeMs();
		reportSample.numDrawnSamples = sample.activeOcclusionQuery->getNumSamples();

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

		mFreeTimerQueries.push(sample.activeTimeQuery);
		mFreeOcclusionQueries.push(sample.activeOcclusionQuery);
	}

	void ProfilerGPU::beginSampleInternal(ActiveSample& sample)
	{
		sample.startStats = RenderStats::instance().getData();
		sample.activeTimeQuery = getTimerQuery();
		sample.activeTimeQuery->begin();

		sample.activeOcclusionQuery = getOcclusionQuery();
		sample.activeOcclusionQuery->begin();
	}

	void ProfilerGPU::endSampleInternal(ActiveSample& sample)
	{
		sample.endStats = RenderStats::instance().getData();
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
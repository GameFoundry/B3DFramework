//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Profiling/BsProfilerGPU.h"
#include "Profiling/BsRenderStats.h"
#include "RenderAPI/BsTimerQuery.h"
#include "RenderAPI/BsOcclusionQuery.h"
#include "Error/BsException.h"

using namespace bs;

const u32 ProfilerGPU::MAX_QUEUE_ELEMENTS = 5;

ProfilerGPU::ProfilerGPU()
{
	mReadyReports = bs_newN<GPUProfilerReport>(MAX_QUEUE_ELEMENTS);
}

ProfilerGPU::~ProfilerGPU()
{
	while(!mUnresolvedFrames.empty())
	{
		ProfiledFrame& frame = mUnresolvedFrames.front();

		FreeFrame(frame);
		mUnresolvedFrames.pop();
	}

	bs_deleteN(mReadyReports, MAX_QUEUE_ELEMENTS);
}

void ProfilerGPU::BeginFrame()
{
	if(mIsFrameActive)
	{
		BS_LOG(Error, Profiler, "Cannot begin a frame because another frame is active.");
		return;
	}

	mIsFrameActive = true;
	mActiveFrame.UncategorizedSamples.clear();
	mActiveFrame.ViewSamples.clear();
}

void ProfilerGPU::EndFrame(bool discard)
{
	if(!mActiveSamples.empty())
	{
		BS_LOG(Error, Profiler, "Attempting to end a frame while a sample is active.");
		return;
	}

	if(mIsViewActive)
	{
		BS_LOG(Error, Profiler, "Attempting to end a frame while a view is active.");
		return;
	}

	if(!mIsFrameActive)
		return;

	if(!discard)
		mUnresolvedFrames.push(mActiveFrame);
	else
		FreeFrame(mActiveFrame);

	mIsFrameActive = false;
}

void ProfilerGPU::BeginView(u64 id, ProfilerString title)
{
	if(!mIsFrameActive)
	{
		BS_LOG(Error, Profiler, "Cannot begin a view because no frame is active.");
		return;
	}

	if(mIsViewActive)
	{
		BS_LOG(Error, Profiler, "Cannot begin a view because another view is active.");
		return;
	}

	auto sample = mViewSamplePool.Construct<ProfiledViewSample>();
	sample->ViewId = id;

	mActiveFrame.ViewSamples.push_back(sample);

	BeginSampleInternal(*sample, true);
	mIsViewActive = true;
}

void ProfilerGPU::EndView()
{
	if(!mActiveSamples.empty())
	{
		BS_LOG(Error, Profiler, "Attempting to end a view while a sample is active.");
		return;
	}

	if(!mIsViewActive)
		return;

	EndSampleInternal(*mActiveFrame.ViewSamples.back());
	mIsViewActive = false;
}

void ProfilerGPU::BeginSample(ProfilerString name)
{
	if(!mIsFrameActive)
	{
		BS_LOG(Error, Profiler, "Cannot begin a sample because no frame is active.");
		return;
	}

	auto sample = mSamplePool.Construct<ProfiledSample>();
	sample->Name = std::move(name);
	BeginSampleInternal(*sample, false);

	if(mActiveSamples.empty())
	{
		if(mIsViewActive)
			mActiveFrame.ViewSamples.back()->Children.push_back(sample);
		else
			mActiveFrame.UncategorizedSamples.push_back(sample);
	}
	else
	{
		ProfiledSample* parent = mActiveSamples.top();
		parent->Children.push_back(sample);
	}

	mActiveSamples.push(sample);
}

void ProfilerGPU::EndSample(const ProfilerString& name)
{
	if(mActiveSamples.empty())
		return;

	ProfiledSample* lastSample = mActiveSamples.top();
	if(lastSample->Name != name)
	{
		BS_LOG(Error, Profiler, "Attempting to end a sample that doesn't match. Got: {0}. Expected: {1}", name.c_str(), lastSample->Name.c_str());
		return;
	}

	EndSampleInternal(*lastSample);
	mActiveSamples.pop();
}

u32 ProfilerGPU::GetNumAvailableReports()
{
	Lock lock(mMutex);

	return mReportCount;
}

GPUProfilerReport ProfilerGPU::GetNextReport()
{
	Lock lock(mMutex);

	if(mReportCount == 0)
	{
		BS_LOG(Error, Profiler, "No reports are available.");
		return GPUProfilerReport();
	}

	GPUProfilerReport report = mReadyReports[mReportHeadPos];

	mReportHeadPos = (mReportHeadPos + 1) % MAX_QUEUE_ELEMENTS;
	mReportCount--;

	return report;
}

void ProfilerGPU::UpdateInternal()
{
	while(!mUnresolvedFrames.empty())
	{
		ProfiledFrame& frame = mUnresolvedFrames.front();

		// Make sure all the top-level queries have finished. If they have that implies
		// all their children have finished as well
		bool isReady = true;
		for(auto& entry : frame.ViewSamples)
		{
			if(!entry->ActiveTimeQuery->IsReady())
			{
				isReady = false;
				break;
			}
		}

		for(auto& entry : frame.UncategorizedSamples)
		{
			if(!entry->ActiveTimeQuery->IsReady())
			{
				isReady = false;
				break;
			}
		}

		if(!isReady)
			break;

		GPUProfilerReport report;
		report.ViewSamples.resize(frame.ViewSamples.size());
		report.UncategorizedSamples.resize(frame.UncategorizedSamples.size());

		for(size_t i = 0; i < frame.ViewSamples.size(); i++)
			ResolveSample(*frame.ViewSamples[i], report.ViewSamples[i]);

		for(size_t i = 0; i < frame.UncategorizedSamples.size(); i++)
			ResolveSample(*frame.UncategorizedSamples[i], report.UncategorizedSamples[i]);

		FreeFrame(frame);
		mUnresolvedFrames.pop();

		{
			Lock lock(mMutex);
			mReadyReports[(mReportHeadPos + mReportCount) % MAX_QUEUE_ELEMENTS] = report;
			if(mReportCount == MAX_QUEUE_ELEMENTS)
				mReportHeadPos = (mReportHeadPos + 1) % MAX_QUEUE_ELEMENTS;
			else
				mReportCount++;
		}
	}
}

void ProfilerGPU::FreeSample(ProfiledSample& sample)
{
	for(auto& entry : sample.Children)
	{
		FreeSample(*entry);
		mSamplePool.Destruct(entry);
	}

	sample.Children.clear();

	mFreeTimerQueries.push(sample.ActiveTimeQuery);

	if(sample.ActiveOcclusionQuery)
		mFreeOcclusionQueries.push(sample.ActiveOcclusionQuery);
}

void ProfilerGPU::FreeFrame(ProfiledFrame& frame)
{
	for(size_t i = 0; i < frame.ViewSamples.size(); i++)
	{
		FreeSample(*frame.ViewSamples[i]);
		mViewSamplePool.Destruct(frame.ViewSamples[i]);
	}

	for(size_t i = 0; i < frame.UncategorizedSamples.size(); i++)
	{
		FreeSample(*frame.UncategorizedSamples[i]);
		mSamplePool.Destruct(frame.UncategorizedSamples[i]);
	}

	frame.ViewSamples.clear();
	frame.UncategorizedSamples.clear();
}

void ProfilerGPU::ResolveSample(const ProfiledSample& sample, GPUProfileSample& reportSample)
{
	reportSample.Name.assign(sample.Name.data(), sample.Name.size());
	reportSample.TimeMs = sample.ActiveTimeQuery->GetTimeMs();

	if(sample.ActiveOcclusionQuery)
		reportSample.NumDrawnSamples = sample.ActiveOcclusionQuery->GetNumSamples();
	else
		reportSample.NumDrawnSamples = 0;

	reportSample.NumDrawCalls = (u32)(sample.EndStats.NumDrawCalls - sample.StartStats.NumDrawCalls);
	reportSample.NumRenderTargetChanges = (u32)(sample.EndStats.NumRenderTargetChanges - sample.StartStats.NumRenderTargetChanges);
	reportSample.NumPresents = (u32)(sample.EndStats.NumPresents - sample.StartStats.NumPresents);
	reportSample.NumClears = (u32)(sample.EndStats.NumClears - sample.StartStats.NumClears);

	reportSample.NumVertices = (u32)(sample.EndStats.NumVertices - sample.StartStats.NumVertices);
	reportSample.NumPrimitives = (u32)(sample.EndStats.NumPrimitives - sample.StartStats.NumPrimitives);

	reportSample.NumPipelineStateChanges = (u32)(sample.EndStats.NumPipelineStateChanges - sample.StartStats.NumPipelineStateChanges);

	reportSample.NumGpuParamBinds = (u32)(sample.EndStats.NumGpuParamBinds - sample.StartStats.NumGpuParamBinds);
	reportSample.NumVertexBufferBinds = (u32)(sample.EndStats.NumVertexBufferBinds - sample.StartStats.NumVertexBufferBinds);
	reportSample.NumIndexBufferBinds = (u32)(sample.EndStats.NumIndexBufferBinds - sample.StartStats.NumIndexBufferBinds);

	reportSample.NumResourceWrites = (u32)(sample.EndStats.NumResourceWrites - sample.StartStats.NumResourceWrites);
	reportSample.NumResourceReads = (u32)(sample.EndStats.NumResourceReads - sample.StartStats.NumResourceReads);

	reportSample.NumObjectsCreated = (u32)(sample.EndStats.NumObjectsCreated - sample.StartStats.NumObjectsCreated);
	reportSample.NumObjectsDestroyed = (u32)(sample.EndStats.NumObjectsDestroyed - sample.StartStats.NumObjectsDestroyed);

	for(auto& entry : sample.Children)
	{
		reportSample.Children.push_back(GPUProfileSample());
		ResolveSample(*entry, reportSample.Children.back());
	}
}

void ProfilerGPU::BeginSampleInternal(ProfiledSample& sample, bool issueOcclusion)
{
	sample.StartStats = RenderStats::Instance().GetData();
	sample.ActiveTimeQuery = GetTimerQuery();
	sample.ActiveTimeQuery->Begin();

	if(issueOcclusion)
	{
		sample.ActiveOcclusionQuery = GetOcclusionQuery();
		sample.ActiveOcclusionQuery->Begin();
	}
}

void ProfilerGPU::EndSampleInternal(ProfiledSample& sample)
{
	sample.EndStats = RenderStats::Instance().GetData();

	if(sample.ActiveOcclusionQuery)
		sample.ActiveOcclusionQuery->End();

	sample.ActiveTimeQuery->End();
}

SPtr<ct::TimerQuery> ProfilerGPU::GetTimerQuery() const
{
	if(!mFreeTimerQueries.empty())
	{
		SPtr<ct::TimerQuery> timerQuery = mFreeTimerQueries.top();
		mFreeTimerQueries.pop();

		return timerQuery;
	}

	return ct::TimerQuery::Create();
}

SPtr<ct::OcclusionQuery> ProfilerGPU::GetOcclusionQuery() const
{
	if(!mFreeOcclusionQueries.empty())
	{
		SPtr<ct::OcclusionQuery> occlusionQuery = mFreeOcclusionQueries.top();
		mFreeOcclusionQueries.pop();

		return occlusionQuery;
	}

	return ct::OcclusionQuery::Create(false);
}

namespace bs
{
ProfilerGPU& gProfilerGPU()
{
	return ProfilerGPU::Instance();
}
} // namespace bs

//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Profiling/B3DProfilerGPU.h"

#include "B3DApplication.h"
#include "Profiling/B3DRenderStats.h"
#include "RenderAPI/B3DTimerQuery.h"
#include "RenderAPI/B3DOcclusionQuery.h"
#include "Error/B3DException.h"
#include "RenderAPI/B3DGpuDevice.h"

using namespace b3d;

const u32 ProfilerGPU::kMaxQueueElements = 5;

ProfilerGPU::ProfilerGPU()
{
	mReadyReports = B3DNewMultiple<GPUProfilerReport>(kMaxQueueElements);
}

ProfilerGPU::~ProfilerGPU()
{
	while(!mUnresolvedFrames.empty())
	{
		ProfiledFrame& frame = mUnresolvedFrames.front();

		FreeFrame(frame);
		mUnresolvedFrames.pop();
	}

	B3DDeleteMultiple(mReadyReports, kMaxQueueElements);
}

void ProfilerGPU::BeginFrame()
{
	if(mIsFrameActive)
	{
		B3D_LOG(Error, Profiler, "Cannot begin a frame because another frame is active.");
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
		B3D_LOG(Error, Profiler, "Attempting to end a frame while a sample is active.");
		return;
	}

	if(mIsViewActive)
	{
		B3D_LOG(Error, Profiler, "Attempting to end a frame while a view is active.");
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

void ProfilerGPU::BeginView(render::GpuCommandBuffer& commandBuffer, u64 id, ProfilerString title)
{
	if(!mIsFrameActive)
	{
		B3D_LOG(Error, Profiler, "Cannot begin a view because no frame is active.");
		return;
	}

	if(mIsViewActive)
	{
		B3D_LOG(Error, Profiler, "Cannot begin a view because another view is active.");
		return;
	}

	auto sample = mViewSamplePool.Construct<ProfiledViewSample>();
	sample->ViewId = id;

	mActiveFrame.ViewSamples.push_back(sample);

	BeginSampleInternal(*sample, commandBuffer, false);
	mIsViewActive = true;
}

void ProfilerGPU::EndView(render::GpuCommandBuffer& commandBuffer)
{
	if(!mActiveSamples.empty())
	{
		B3D_LOG(Error, Profiler, "Attempting to end a view while a sample is active.");
		return;
	}

	if(!mIsViewActive)
		return;

	EndSampleInternal(*mActiveFrame.ViewSamples.back(), commandBuffer);
	mIsViewActive = false;
}

void ProfilerGPU::BeginSample(render::GpuCommandBuffer& commandBuffer, ProfilerString name)
{
	if(!mIsFrameActive)
		return;

	auto sample = mSamplePool.Construct<ProfiledSample>();
	sample->Name = std::move(name);
	BeginSampleInternal(*sample, commandBuffer, false);

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

void ProfilerGPU::EndSample(render::GpuCommandBuffer& commandBuffer, const ProfilerString& name)
{
	if(mActiveSamples.empty())
		return;

	ProfiledSample* lastSample = mActiveSamples.top();
	if(lastSample->Name != name)
	{
		B3D_LOG(Error, Profiler, "Attempting to end a sample that doesn't match. Got: {0}. Expected: {1}", name.c_str(), lastSample->Name.c_str());
		return;
	}

	EndSampleInternal(*lastSample, commandBuffer);
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
		B3D_LOG(Error, Profiler, "No reports are available.");
		return GPUProfilerReport();
	}

	GPUProfilerReport report = mReadyReports[mReportHeadPos];

	mReportHeadPos = (mReportHeadPos + 1) % kMaxQueueElements;
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
			mReadyReports[(mReportHeadPos + mReportCount) % kMaxQueueElements] = report;
			if(mReportCount == kMaxQueueElements)
				mReportHeadPos = (mReportHeadPos + 1) % kMaxQueueElements;
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
		reportSample.SamplesDrawn = sample.ActiveOcclusionQuery->GetSampleCount();
	else
		reportSample.SamplesDrawn = 0;

	reportSample.DrawCallCount = (u32)(sample.EndStats.NumDrawCalls - sample.StartStats.NumDrawCalls);
	reportSample.RenderTargetChangesCount = (u32)(sample.EndStats.NumRenderTargetChanges - sample.StartStats.NumRenderTargetChanges);
	reportSample.PresentCount = (u32)(sample.EndStats.NumPresents - sample.StartStats.NumPresents);
	reportSample.ClearCount = (u32)(sample.EndStats.NumClears - sample.StartStats.NumClears);

	reportSample.VerticesDrawn = (u32)(sample.EndStats.NumVertices - sample.StartStats.NumVertices);
	reportSample.PrimitivesDrawn = (u32)(sample.EndStats.NumPrimitives - sample.StartStats.NumPrimitives);

	reportSample.PipelineStateChangeCount = (u32)(sample.EndStats.NumPipelineStateChanges - sample.StartStats.NumPipelineStateChanges);

	reportSample.GpuParameterBindCount = (u32)(sample.EndStats.NumGpuParamBinds - sample.StartStats.NumGpuParamBinds);
	reportSample.VertexBufferBindCount = (u32)(sample.EndStats.NumVertexBufferBinds - sample.StartStats.NumVertexBufferBinds);
	reportSample.IndexBufferBindCount = (u32)(sample.EndStats.NumIndexBufferBinds - sample.StartStats.NumIndexBufferBinds);

	reportSample.ResourceWriteCount = (u32)(sample.EndStats.NumResourceWrites - sample.StartStats.NumResourceWrites);
	reportSample.ResourceReadCount = (u32)(sample.EndStats.NumResourceReads - sample.StartStats.NumResourceReads);

	reportSample.ObjectsCreatedCount = (u32)(sample.EndStats.NumObjectsCreated - sample.StartStats.NumObjectsCreated);
	reportSample.ObjectsDestroyedCount = (u32)(sample.EndStats.NumObjectsDestroyed - sample.StartStats.NumObjectsDestroyed);

	for(auto& entry : sample.Children)
	{
		reportSample.ChildSamples.push_back(GPUProfileSample());
		ResolveSample(*entry, reportSample.ChildSamples.back());
	}
}

void ProfilerGPU::BeginSampleInternal(ProfiledSample& sample, render::GpuCommandBuffer& commandBuffer, bool issueOcclusion)
{
	sample.StartStats = RenderStats::Instance().GetData();
	sample.ActiveTimeQuery = GetTimerQuery();
	sample.ActiveTimeQuery->Begin(commandBuffer);

	if(issueOcclusion)
	{
		sample.ActiveOcclusionQuery = GetOcclusionQuery();
		sample.ActiveOcclusionQuery->Begin(commandBuffer);
	}
}

void ProfilerGPU::EndSampleInternal(ProfiledSample& sample, render::GpuCommandBuffer& commandBuffer)
{
	sample.EndStats = RenderStats::Instance().GetData();

	if(sample.ActiveOcclusionQuery)
		sample.ActiveOcclusionQuery->End(commandBuffer);

	sample.ActiveTimeQuery->End(commandBuffer);
}

SPtr<render::TimerQuery> ProfilerGPU::GetTimerQuery() const
{
	if(!mFreeTimerQueries.empty())
	{
		SPtr<render::TimerQuery> timerQuery = mFreeTimerQueries.top();
		mFreeTimerQueries.pop();

		return timerQuery;
	}

	const SPtr<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();
	if(gpuDevice == nullptr)
		return nullptr;

	return gpuDevice->CreateTimerQuery();
}

SPtr<render::OcclusionQuery> ProfilerGPU::GetOcclusionQuery() const
{
	if(!mFreeOcclusionQueries.empty())
	{
		SPtr<render::OcclusionQuery> occlusionQuery = mFreeOcclusionQueries.top();
		mFreeOcclusionQueries.pop();

		return occlusionQuery;
	}

	const SPtr<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();
	if(gpuDevice == nullptr)
		return nullptr;

	return gpuDevice->CreateOcclusionQuery(false);
}

namespace b3d
{
ProfilerGPU& GetProfilerGPU()
{
	return ProfilerGPU::Instance();
}
} // namespace b3d

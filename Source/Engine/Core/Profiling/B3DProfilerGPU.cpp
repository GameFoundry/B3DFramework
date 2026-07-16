//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Profiling/B3DProfilerGPU.h"

#include "B3DApplication.h"
#include "Profiling/B3DRenderStats.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuDeviceCapabilities.h"

using namespace b3d;

GpuCommandBufferProfiler::GpuCommandBufferProfiler(render::GpuCommandBuffer& commandBuffer)
	: mCommandBufferId((u64)&commandBuffer)
{
	mTimestampQueryPool = GetGpuProfiler().FindOrCreateQueryPool();
	if(mTimestampQueryPool != nullptr)
		commandBuffer.ResetQueries(mTimestampQueryPool);
}

GpuCommandBufferProfiler::~GpuCommandBufferProfiler()
{
	if(mTimestampQueryPool != nullptr) // If it hasn't been cleared yet
		Clear();
}

void GpuCommandBufferProfiler::BeginSample(render::GpuCommandBuffer& commandBuffer, ProfilerString name)
{
	if(!B3D_ENSURE(mCommandBufferId == (u64)&commandBuffer))
		return;
	if(mTimestampQueryPool == nullptr)
		return;
	if(mSkippedSampleDepth > 0)
	{
		mSkippedSampleDepth++;
		return;
	}

	const render::GpuQueryId beginQueryId = mTimestampQueryPool->AllocateQuery();
	if(!beginQueryId.IsValid())
	{
		mSkippedSampleDepth = 1;
		return;
	}

	auto sample = mSamplePool.Construct<Sample>();
	sample->Name = std::move(name);
	sample->BeginRenderStatistics = RenderStats::Instance().GetData();
	sample->TimestampBeginQueryId = beginQueryId;

	commandBuffer.WriteTimestamp(sample->TimestampBeginQueryId, mTimestampQueryPool);

	if(mActiveSampleChain.Empty())
		mRootSamples.Add(sample);
	else
	{
		Sample* parent = mActiveSampleChain.back();
		parent->Children.Add(sample);
	}

	mActiveSampleChain.Add(sample);
}

void GpuCommandBufferProfiler::EndSample(render::GpuCommandBuffer& commandBuffer)
{
	if(!B3D_ENSURE(mCommandBufferId == (u64)&commandBuffer))
		return;
	if(mTimestampQueryPool == nullptr)
		return;
	if(mSkippedSampleDepth > 0)
	{
		mSkippedSampleDepth--;
		return;
	}

	if(!B3D_ENSURE(!mActiveSampleChain.Empty()))
		return;

	Sample* sample = mActiveSampleChain.back();
	sample->EndRenderStatistics = RenderStats::Instance().GetData();
	const render::GpuQueryId endQueryId = mTimestampQueryPool->AllocateQuery();
	if(!endQueryId.IsValid())
	{
		sample->TimestampEndQueryId = sample->TimestampBeginQueryId;
		mActiveSampleChain.Pop();
		return;
	}

	sample->TimestampEndQueryId = endQueryId;

	commandBuffer.WriteTimestamp(sample->TimestampEndQueryId, mTimestampQueryPool);

	mActiveSampleChain.Pop();
}

void GpuCommandBufferProfiler::Clear()
{
	Function<void(Sample*)> fnFreeSample = [this, &fnFreeSample](Sample* sample)
	{
		for(Sample* child : sample->Children)
			fnFreeSample(child);

		mSamplePool.Destruct(sample);
	};

	for(const auto& sample : mRootSamples)
		fnFreeSample(sample);
	
	mActiveSampleChain.Clear();
	mRootSamples.Clear();
	mSkippedSampleDepth = 0;
	mCommandBufferId = 0;

	if(mTimestampQueryPool != nullptr)
		GetGpuProfiler().ReleaseQueryPool(mTimestampQueryPool);
	mTimestampQueryPool = nullptr;
}

void GpuCommandBufferProfiler::Reset(render::GpuCommandBuffer& commandBuffer)
{
	mCommandBufferId = (u64)&commandBuffer;
	mSkippedSampleDepth = 0;

	mTimestampQueryPool = GetGpuProfiler().FindOrCreateQueryPool();
	if(mTimestampQueryPool != nullptr)
		commandBuffer.ResetQueries(mTimestampQueryPool);
}

void GpuCommandBufferProfiler::ConvertToResultSample(const Sample& sample, GpuProfilerSample& reportSample)
{
	const TShared<GpuDevice>& device = GetApplication().GetPrimaryGpuDevice();
	const u64 beginTimestamp = sample.TimestampQueryPool->GetQueryResult(sample.TimestampBeginQueryId);
	const u64 endTimestamp = sample.TimestampQueryPool->GetQueryResult(sample.TimestampEndQueryId);

	reportSample.Name.assign(sample.Name.data(), sample.Name.size());
	reportSample.TimeMs = endTimestamp >= beginTimestamp
		? device->ConvertTimestampToMilliseconds(endTimestamp - beginTimestamp)
		: 0.0f;
	reportSample.SamplesDrawn = 0;

	reportSample.DrawCallCount = (u32)(sample.EndRenderStatistics.DrawCallCount - sample.BeginRenderStatistics.DrawCallCount);
	reportSample.RenderTargetChangesCount = (u32)(sample.EndRenderStatistics.RenderTargetChangeCount - sample.BeginRenderStatistics.RenderTargetChangeCount);
	reportSample.PresentCount = (u32)(sample.EndRenderStatistics.PresentCount - sample.BeginRenderStatistics.PresentCount);
	reportSample.ClearCount = (u32)(sample.EndRenderStatistics.ClearCount - sample.BeginRenderStatistics.ClearCount);

	reportSample.VerticesDrawn = (u32)(sample.EndRenderStatistics.VertexCount - sample.BeginRenderStatistics.VertexCount);
	reportSample.PrimitivesDrawn = (u32)(sample.EndRenderStatistics.PrimitiveCount - sample.BeginRenderStatistics.PrimitiveCount);

	reportSample.PipelineStateChangeCount = (u32)(sample.EndRenderStatistics.PipelineStateChangeCount - sample.BeginRenderStatistics.PipelineStateChangeCount);

	reportSample.GpuParameterBindCount = (u32)(sample.EndRenderStatistics.GpuParameterBindCount - sample.BeginRenderStatistics.GpuParameterBindCount);
	reportSample.VertexBufferBindCount = (u32)(sample.EndRenderStatistics.VertexBufferBindCount - sample.BeginRenderStatistics.VertexBufferBindCount);
	reportSample.IndexBufferBindCount = (u32)(sample.EndRenderStatistics.IndexBufferBindCount - sample.BeginRenderStatistics.IndexBufferBindCount);

	reportSample.ResourceWriteCount = (u32)(sample.EndRenderStatistics.ResourceWriteCount - sample.BeginRenderStatistics.ResourceWriteCount);
	reportSample.ResourceReadCount = (u32)(sample.EndRenderStatistics.ResourceReadCount - sample.BeginRenderStatistics.ResourceReadCount);

	reportSample.ObjectsCreatedCount = (u32)(sample.EndRenderStatistics.ObjectsCreatedCount - sample.BeginRenderStatistics.ObjectsCreatedCount);
	reportSample.ObjectsDestroyedCount = (u32)(sample.EndRenderStatistics.ObjectsDestroyedCount - sample.BeginRenderStatistics.ObjectsDestroyedCount);

	reportSample.ChildSamples.Reserve(sample.Children.Size());
	for(auto& entry : sample.Children)
	{
		reportSample.ChildSamples.Add(GpuProfilerSample());
		ConvertToResultSample(*entry, reportSample.ChildSamples.Back());
	}
}

GpuProfilerResults GpuCommandBufferProfiler::GetResults()
{
	GpuProfilerResults output;

	output.Samples.Reserve(mRootSamples.Size());
	for(auto& sample : mRootSamples)
	{
		output.Samples.Add(GpuProfilerSample());
		ConvertToResultSample(*sample, output.Samples.Back());
	}

	return output;
}

GpuProfiler::~GpuProfiler()
{
	Lock lock(mMutex);

	B3D_ASSERT(mFreeTimestampQueryPools.Empty() && "GpuProfiler shutting down but query pools were not released. They must be released on the render thread before shutdown.");
	B3D_ASSERT(mUnresolvedProfilerData.empty() && "GpuProfiler shutting down but unresolved profiler data was not released.");
	B3D_ASSERT(mResolvedProfilerData.empty() && "GpuProfiler shutting down but resolved profiler data was not released.");

	mFreeCommandBufferProfilers.Clear();
}

void GpuProfiler::Clear()
{
	Lock lock(mMutex);

	// Clear command buffer profilers first (they will release their query pools)
	for(auto& entry : mUnresolvedProfilerData)
	{
		for(auto& profiler : entry.second.Queued)
			profiler->Clear();
	}
	mUnresolvedProfilerData.clear();

	for(auto& entry : mResolvedProfilerData)
		entry.second.LastResolved->Clear();
	mResolvedProfilerData.clear();

	for(auto& profiler : mFreeCommandBufferProfilers)
		profiler->Clear();

	// Now clear the query pools
	mFreeTimestampQueryPools.Clear();
}

void GpuProfiler::Update()
{
	Lock lock(mMutex); // Note: Potentially expensive mutex as it could be held for long

	for(auto& entry : mUnresolvedProfilerData)
	{
		UnresolvedCommandBufferProfilerData& unresolvedProfilerData = entry.second;

		bool previousEntryWasResolved = false;
		for(u32 reverseIndex = 0; reverseIndex < unresolvedProfilerData.Queued.Size();)
		{
			const u32 index = (u32)unresolvedProfilerData.Queued.Size() - 1 - reverseIndex;
			TShared<GpuCommandBufferProfiler>& unresolvedCommandBufferProfiler = unresolvedProfilerData.Queued[index];
			if(unresolvedCommandBufferProfiler->mTimestampQueryPool == nullptr || previousEntryWasResolved)
			{
				auto it = unresolvedProfilerData.Queued.Begin() + index;
				TShared<GpuCommandBufferProfiler> profiler = *it;
				unresolvedProfilerData.Queued.Erase(it);

				profiler->Clear();
				mFreeCommandBufferProfilers.Add(std::move(profiler));

				continue;
			}

			if(unresolvedCommandBufferProfiler->mTimestampQueryPool->TryResolve())
			{
				mResolvedProfilerData[entry.first].LastResolved = unresolvedCommandBufferProfiler;
				unresolvedProfilerData.Queued.Erase(unresolvedProfilerData.Queued.Begin() + index);

				previousEntryWasResolved = true;
				continue;
			}

			reverseIndex++;
		}
	}
}

TShared<GpuCommandBufferProfiler> GpuProfiler::CreateCommandBufferProfiler(render::GpuCommandBuffer& commandBuffer)
{
	Lock lock(mMutex);

	if(!mFreeCommandBufferProfilers.Empty())
	{
		TShared<GpuCommandBufferProfiler> commandBufferProfiler = mFreeCommandBufferProfilers.Back();
		mFreeCommandBufferProfilers.Pop();

		commandBufferProfiler->Reset(commandBuffer);
		return commandBufferProfiler;
	}

	return B3DMakeShared<GpuCommandBufferProfiler>(commandBuffer);
}

void GpuProfiler::ResolveProfileWhenReady(const ProfilerString& name, const TShared<GpuCommandBufferProfiler>& profiler)
{
	Lock lock(mMutex);

	if(profiler->IsEmpty())
	{
		profiler->Clear();
		mFreeCommandBufferProfilers.Add(profiler);

		return;
	}

	UnresolvedCommandBufferProfilerData& profilerData = mUnresolvedProfilerData[name];

	if(profilerData.Queued.Size() == kMaxQueuedEntries)
	{
		auto it = profilerData.Queued.Begin();
		TShared<GpuCommandBufferProfiler> existingProfiler = *it;
		profilerData.Queued.Erase(profilerData.Queued.Begin());

		existingProfiler->Clear();
		mFreeCommandBufferProfilers.Add(std::move(existingProfiler));
	}

	profilerData.Queued.Add(profiler);
}

TOptional<GpuProfilerResults> GpuProfiler::GetResults(const ProfilerString& name)
{
	if(auto found = mResolvedProfilerData.find(name); found != mResolvedProfilerData.end())
	{
		ResolvedCommandBufferProfilerData resolvedProfilerData = std::move(found->second);
		TShared<GpuCommandBufferProfiler> profiler = resolvedProfilerData.LastResolved;
		GpuProfilerResults results = resolvedProfilerData.LastResolved->GetResults();

		mResolvedProfilerData.erase(found);

		profiler->Clear();
		mFreeCommandBufferProfilers.Add(std::move(profiler));

		return results;
	}
	
	return std::nullopt;
}

TShared<render::GpuQueryPool> GpuProfiler::FindOrCreateQueryPool() const
{
	// Note: mMutex must be locked here
	const TShared<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();
	if(gpuDevice == nullptr || !gpuDevice->GetCapabilities().HasCapability(RSC_TIMER_QUERIES))
		return nullptr;

	if(!mFreeTimestampQueryPools.Empty())
	{
		TShared<render::GpuQueryPool> queryPool = mFreeTimestampQueryPools.back();
		mFreeTimestampQueryPools.Pop();

		return queryPool;
	}

	render::GpuQueryPoolCreateInformation createInformation;
	createInformation.Type = render::GpuQueryType::Timestamp;
	createInformation.PoolSize = 1024;

	return gpuDevice->CreateQueryPool(createInformation);
}

void GpuProfiler::ReleaseQueryPool(const TShared<render::GpuQueryPool>& queryPool)
{
	// Note: mMutex must be locked here

	mFreeTimestampQueryPools.Add(queryPool);
}

namespace b3d
{
GpuProfiler& GetGpuProfiler()
{
	return GpuProfiler::Instance();
}
} // namespace b3d

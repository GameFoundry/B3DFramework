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
	TShared<render::GpuQueryPool> queryPool = GetGpuProfiler().FindOrCreateQueryPool();
	if(queryPool != nullptr)
	{
		commandBuffer.ResetQueries(queryPool);
		mTimestampQueryPools.Add(std::move(queryPool));
	}
}

GpuCommandBufferProfiler::~GpuCommandBufferProfiler()
{
	if(!mTimestampQueryPools.Empty()) // If it hasn't been cleared yet
		Clear();
}

void GpuCommandBufferProfiler::BeginSample(render::GpuCommandBuffer& commandBuffer, ProfilerString name)
{
	if(!B3D_ENSURE(mCommandBufferId == (u64)&commandBuffer))
		return;
	if(mTimestampQueryPools.Empty())
		return;
	if(mSkippedSampleDepth > 0)
	{
		mSkippedSampleDepth++;
		return;
	}

	const RenderStatsData beginRenderStatistics = RenderStats::Instance().GetData();

	Timestamp beginTimestamp;
	if(!WriteTimestamp(commandBuffer, beginTimestamp))
	{
		mSkippedSampleDepth = 1;
		return;
	}

	auto sample = mSamplePool.Construct<Sample>();
	sample->Name = std::move(name);
	sample->BeginRenderStatistics = beginRenderStatistics;
	sample->BeginTimestamp = beginTimestamp;

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
	if(mTimestampQueryPools.Empty())
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

	// Without a query, the sample reports no time
	if(!WriteTimestamp(commandBuffer, sample->EndTimestamp))
		sample->EndTimestamp = sample->BeginTimestamp;

	mActiveSampleChain.Pop();
}

bool GpuCommandBufferProfiler::WriteTimestamp(render::GpuCommandBuffer& commandBuffer, Timestamp& outTimestamp)
{
	// Query pools can only be reset outside of a render pass, so switching early leaves queries for the samples within the next one
	const u32 freeQueryCount = mTimestampQueryPools.Back()->GetPoolSize() - mAllocatedQueryCount;
	if(freeQueryCount < kRenderPassQueryCount && !commandBuffer.IsInRenderPass())
	{
		TShared<render::GpuQueryPool> queryPool;
		{
			GpuProfiler& gpuProfiler = GetGpuProfiler();
			Lock lock(gpuProfiler.mMutex);
			queryPool = gpuProfiler.FindOrCreateQueryPool();
		}

		// Without a new pool, the remaining queries of the current one are still used
		if(queryPool != nullptr)
		{
			commandBuffer.ResetQueries(queryPool);
			mTimestampQueryPools.Add(std::move(queryPool));
			mAllocatedQueryCount = 0;
		}
	}

	const render::GpuQueryId queryId = mTimestampQueryPools.Back()->AllocateQuery();
	if(!queryId.IsValid())
		return false;

	mAllocatedQueryCount++;

	outTimestamp.PoolIndex = (u32)mTimestampQueryPools.Size() - 1;
	outTimestamp.QueryId = queryId;
	commandBuffer.WriteTimestamp(queryId, mTimestampQueryPools.Back());

	return true;
}

bool GpuCommandBufferProfiler::TryResolve()
{
	for(const TShared<render::GpuQueryPool>& queryPool : mTimestampQueryPools)
	{
		if(!queryPool->TryResolve())
			return false;
	}

	return true;
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

	for(const TShared<render::GpuQueryPool>& queryPool : mTimestampQueryPools)
		GetGpuProfiler().ReleaseQueryPool(queryPool);

	mTimestampQueryPools.Clear();
	mAllocatedQueryCount = 0;
}

void GpuCommandBufferProfiler::Reset(render::GpuCommandBuffer& commandBuffer)
{
	mCommandBufferId = (u64)&commandBuffer;
	mSkippedSampleDepth = 0;

	TShared<render::GpuQueryPool> queryPool = GetGpuProfiler().FindOrCreateQueryPool();
	if(queryPool != nullptr)
	{
		commandBuffer.ResetQueries(queryPool);
		mTimestampQueryPools.Add(std::move(queryPool));
	}
}

void GpuCommandBufferProfiler::ConvertToResultSample(const Sample& sample, GpuProfilerSample& reportSample)
{
	const TShared<GpuDevice>& device = GetApplication().GetPrimaryGpuDevice();
	const u64 beginTimestamp = mTimestampQueryPools[sample.BeginTimestamp.PoolIndex]->GetQueryResult(sample.BeginTimestamp.QueryId);
	const u64 endTimestamp = mTimestampQueryPools[sample.EndTimestamp.PoolIndex]->GetQueryResult(sample.EndTimestamp.QueryId);

	reportSample.Name.assign(sample.Name.data(), sample.Name.size());
	reportSample.TimeMs = endTimestamp >= beginTimestamp
		? device->ConvertTimestampToMilliseconds(endTimestamp - beginTimestamp)
		: 0.0f;
	reportSample.SamplesDrawn = 0;

	const auto fnGetDelta = [&sample](RenderStatistic statistic)
	{
		const u32 statisticIndex = (u32)statistic;
		return (u32)(sample.EndRenderStatistics.Values[statisticIndex] - sample.BeginRenderStatistics.Values[statisticIndex]);
	};

	reportSample.DrawCallCount = fnGetDelta(RenderStatistic::DrawCallCount);
	reportSample.RenderTargetChangesCount = fnGetDelta(RenderStatistic::RenderTargetChangeCount);
	reportSample.PresentCount = fnGetDelta(RenderStatistic::PresentCount);
	reportSample.ClearCount = fnGetDelta(RenderStatistic::ClearCount);

	reportSample.VerticesDrawn = fnGetDelta(RenderStatistic::VertexCount);
	reportSample.PrimitivesDrawn = fnGetDelta(RenderStatistic::PrimitiveCount);

	reportSample.PipelineStateChangeCount = fnGetDelta(RenderStatistic::PipelineStateChangeCount);

	reportSample.GpuParameterBindCount = fnGetDelta(RenderStatistic::GpuParameterBindCount);
	reportSample.VertexBufferBindCount = fnGetDelta(RenderStatistic::VertexBufferBindCount);
	reportSample.IndexBufferBindCount = fnGetDelta(RenderStatistic::IndexBufferBindCount);

	reportSample.ResourceWriteCount = fnGetDelta(RenderStatistic::ResourceWriteCount);
	reportSample.ResourceReadCount = fnGetDelta(RenderStatistic::ResourceReadCount);

	reportSample.ObjectsCreatedCount = fnGetDelta(RenderStatistic::ObjectsCreatedCount);
	reportSample.ObjectsDestroyedCount = fnGetDelta(RenderStatistic::ObjectsDestroyedCount);

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

	for(auto& profiler : mDiscardedProfilers)
		profiler->Clear();
	
	mDiscardedProfilers.Clear();

	for(auto& profiler : mFreeCommandBufferProfilers)
		profiler->Clear();

	// Now clear the query pools
	mFreeTimestampQueryPools.Clear();
}

void GpuProfiler::Update()
{
	Lock lock(mMutex); // Note: Potentially expensive mutex as it could be held for long

	for(auto& entry : mUnresolvedProfilerData)
		ResolveQueuedProfilers(entry.first, entry.second);

	RecycleDiscardedProfilers();
}

void GpuProfiler::ResolveQueuedProfilers(const ProfilerString& name, UnresolvedCommandBufferProfilerData& profilerData)
{
	// Note: mMutex must be locked here
	bool previousEntryWasResolved = false;
	for(u32 reverseIndex = 0; reverseIndex < profilerData.Queued.Size();)
	{
		const u32 index = (u32)profilerData.Queued.Size() - 1 - reverseIndex;
		TShared<GpuCommandBufferProfiler>& unresolvedCommandBufferProfiler = profilerData.Queued[index];
		if(unresolvedCommandBufferProfiler->mTimestampQueryPools.Empty() || previousEntryWasResolved)
		{
			mDiscardedProfilers.Add(unresolvedCommandBufferProfiler);
			profilerData.Queued.Erase(profilerData.Queued.Begin() + index);

			continue;
		}

		if(unresolvedCommandBufferProfiler->TryResolve())
		{
			mResolvedProfilerData[name].LastResolved = unresolvedCommandBufferProfiler;
			profilerData.Queued.Erase(profilerData.Queued.Begin() + index);

			previousEntryWasResolved = true;
			continue;
		}

		reverseIndex++;
	}
}

void GpuProfiler::RecycleDiscardedProfilers()
{
	// Note: mMutex must be locked here
	for(u32 index = 0; index < (u32)mDiscardedProfilers.Size();)
	{
		const TShared<GpuCommandBufferProfiler>& profiler = mDiscardedProfilers[index];
		if(!profiler->TryResolve())
		{
			index++;
			continue;
		}

		profiler->Clear();
		mFreeCommandBufferProfilers.Add(profiler);
		mDiscardedProfilers.Erase(mDiscardedProfilers.Begin() + index);
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

	// Without frames passing, such as during start-up or in unit tests, Update() would not free the profilers queued earlier
	ResolveQueuedProfilers(name, profilerData);

	if(profilerData.Queued.Size() == kMaxQueuedEntries)
	{
		mDiscardedProfilers.Add(profilerData.Queued[0]);
		profilerData.Queued.Erase(profilerData.Queued.Begin());
	}

	profilerData.Queued.Add(profiler);
}

TOptional<GpuProfilerResults> GpuProfiler::GetResults(const ProfilerString& name)
{
	Lock lock(mMutex);

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

TShared<render::GpuQueryPool> GpuProfiler::FindOrCreateQueryPool()
{
	// Note: mMutex must be locked here
	const TShared<GpuDevice>& gpuDevice = GetApplication().GetPrimaryGpuDevice();
	if(gpuDevice == nullptr || !gpuDevice->GetCapabilities().HasCapability(RSC_TIMER_QUERIES))
		return nullptr;

	// Profilers can be discarded without frames passing, such as during start-up, which would otherwise not reuse their pools
	if(mFreeTimestampQueryPools.Empty())
		RecycleDiscardedProfilers();

	if(!mFreeTimestampQueryPools.Empty())
	{
		TShared<render::GpuQueryPool> queryPool = mFreeTimestampQueryPools.back();
		mFreeTimestampQueryPools.Pop();

		return queryPool;
	}

	render::GpuQueryPoolCreateInformation createInformation;
	createInformation.Type = render::GpuQueryType::Timestamp;
	createInformation.PoolSize = kQueryPoolSize;

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

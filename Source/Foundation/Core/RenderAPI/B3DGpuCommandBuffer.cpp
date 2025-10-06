//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/B3DGpuCommandBuffer.h"

#include "Profiling/B3DProfilerGPU.h"

using namespace b3d;

namespace b3d { namespace render
{
GpuCommandBufferPool::GpuCommandBufferPool(GpuDevice& gpuDevice, const GpuCommandBufferPoolCreateInformation& createInformation)
	:mGpuDevice(gpuDevice), mInformation(createInformation)
{
	// Process messages related to this command buffer pool on this thread. Mostly these are command buffer resets once they are done executing.
	Scheduler* const scheduler = Scheduler::Get();
	if (B3D_ENSURE(scheduler))
	{
		mMessageQueue.ScheduleRunUntilShutdown(*scheduler, true);
	}
}

void GpuCommandBufferPool::Destroy()
{
	if (mIsDestroyed)
		return;

	mIsDestroyed = true;
}

GpuCommandBuffer::GpuCommandBuffer(GpuDevice& gpuDevice, ThreadId ownerThread, GpuQueueUsage queueType, const GpuCommandBufferCreateInformation& createInformation)
	:mGpuDevice(gpuDevice), mUsage(queueType), mOwnerThread(ownerThread), mInformation(createInformation)
{ }


GpuCommandBuffer::~GpuCommandBuffer()
{
	OnDestroyed(mIsSubmitted);
}

#if B3D_PROFILING_ENABLED
SPtr<GpuCommandBufferProfiler> GpuCommandBuffer::BeginProfiling(const ProfilerString& profilingScopeName)
{
	if(!B3D_ENSURE(mProfiler == nullptr))
		return nullptr;

	mProfiler = GetGpuProfiler().CreateCommandBufferProfiler(*this);
	mProfilingScopeName = profilingScopeName;

	return mProfiler;
}

void GpuCommandBuffer::EndProfiling()
{
	if(!B3D_ENSURE(mProfiler != nullptr))
		return;

	GetGpuProfiler().ResolveProfileWhenReady(mProfilingScopeName, mProfiler);

	mProfiler = nullptr;
	mProfilingScopeName.clear();
}
#endif
}}

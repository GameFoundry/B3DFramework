//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DGpuDevice.h"
#include "B3DGpuCommandBuffer.h"
#include "B3DGpuTransferBufferHelper.h"
#include "Image/B3DTexture.h"
#include "GpuBackend/B3DGpuBuffer.h"

using namespace b3d;

const GpuQueueMask GpuQueueMask::kNone = GpuQueueMask(0);
const GpuQueueMask GpuQueueMask::kAll = GpuQueueMask(~0u);

GpuQueue::GpuQueue(GpuDevice& gpuDevice, GpuQueueType type, u32 index)
	:mGpuDevice(gpuDevice), mType(type), mIndex(index)
{
}

u64 GpuDevice::SubmitCommandBuffer(const GpuSubmissionInformation& information, u32 queueIndex, bool flushTransferCommandBuffer)
{
	if (!B3D_ENSURE(information.CommandBuffer))
		return 0;

	const u32 queueCount = GetQueueCount(information.CommandBuffer->GetQueueType());
	if (!B3D_ENSURE(queueIndex < queueCount))
		return 0;

	const SPtr<GpuQueue>& queue = GetQueue(information.CommandBuffer->GetQueueType(), queueIndex);
	if (!B3D_ENSURE(queue))
		return 0;

	const u64 submissionIndex = mSubmissionCounter.fetch_add(1, std::memory_order_acq_rel) + 1;

	GpuSubmissionInformation augmentedInformation = information;
	augmentedInformation.SignalFences.Add(GpuTimelineFenceAndValue{ mDefaultSubmissionFence, submissionIndex });

	queue->SubmitCommandBuffer(augmentedInformation, flushTransferCommandBuffer);
	return submissionIndex;
}

void GpuDevice::SubmitCommandBuffer(const SPtr<render::GpuCommandBuffer>& commandBuffer, GpuQueueMask syncMask, u32 queueIndex)
{
	GpuSubmissionInformation information;
	information.CommandBuffer = commandBuffer;
	information.SyncMask = syncMask;

	(void)SubmitCommandBuffer(information, queueIndex);
}

bool GpuDevice::IsSubmissionComplete(u64 index) const
{
	B3D_ASSERT(mDefaultSubmissionFence != nullptr);
	return mDefaultSubmissionFence->IsSignaled(index);
}

SPtr<SamplerState> GpuDevice::FindOrCreateSamplerState(const SamplerStateCreateInformation& createInformation)
{
	Lock lock(mSamplerStateMutex);

	if (auto found = mCachedSamplerStates.find(createInformation); found != mCachedSamplerStates.end())
	{
		SPtr<SamplerState> existingSamplerState = found->second;
		if (existingSamplerState != nullptr)
			return existingSamplerState;
	}

	SPtr<SamplerState> newSamplerState = CreateSamplerState(createInformation);
	mCachedSamplerStates[createInformation] = newSamplerState;

	return newSamplerState;
}

const SPtr<render::GpuCommandBuffer>& GpuDevice::GetOrCreateTransferCommandBuffer()
{
	return mTransferBufferHelper->GetOrCreateTransferCommandBuffer();
}

void GpuDevice::SubmitTransferCommandBuffers(bool wait)
{
	mTransferBufferHelper->SubmitTransferCommandBuffer(wait);
}

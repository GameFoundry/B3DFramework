//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DGpuDevice.h"
#include "B3DGpuCommandBuffer.h"
#include "B3DGpuTransferBufferHelper.h"
#include "CoreObject/B3DRenderThread.h"
#include "Image/B3DTexture.h"
#include "GpuBackend/B3DGpuBuffer.h"

using namespace b3d;

const GpuQueueMask GpuQueueMask::kNone = GpuQueueMask(0);
const GpuQueueMask GpuQueueMask::kAll = GpuQueueMask(~0u);

GpuQueue::GpuQueue(GpuDevice& gpuDevice, GpuQueueType type, u32 index)
	:mGpuDevice(gpuDevice), mType(type), mIndex(index)
{
}

void GpuDevice::SubmitCommandBuffer(const GpuSubmissionInformation& information, u32 queueIndex, bool flushTransferCommandBuffer)
{
	if (!B3D_ENSURE(information.CommandBuffer))
		return;

	const u32 queueCount = GetQueueCount(information.CommandBuffer->GetQueueType());
	if (!B3D_ENSURE(queueIndex < queueCount))
		return;

	const SPtr<GpuQueue>& queue = GetQueue(information.CommandBuffer->GetQueueType(), queueIndex);
	if (!B3D_ENSURE(queue))
		return;

	queue->SubmitCommandBuffer(information, flushTransferCommandBuffer);
}

void GpuDevice::SubmitCommandBuffer(const SPtr<render::GpuCommandBuffer>& commandBuffer, GpuQueueMask syncMask, u32 queueIndex)
{
	GpuSubmissionInformation information;
	information.CommandBuffer = commandBuffer;
	information.SyncMask = syncMask;

	SubmitCommandBuffer(information, queueIndex);
}

bool GpuDevice::IsFrameComplete(u64 index) const
{
	const u64 currentFrame = mFrameIndex.load(std::memory_order_acquire);
	return index + RenderThread::kMaximumFramesInFlight <= currentFrame;
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

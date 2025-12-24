//************************************ B3D Framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DGpuDevice.h"
#include "B3DGpuCommandBuffer.h"
#include "B3DGpuTransferBufferHelper.h"
#include "Image/B3DTexture.h"
#include "RenderAPI/B3DGpuBuffer.h"

using namespace b3d;

const GpuQueueMask GpuQueueMask::kNone = GpuQueueMask(0);
const GpuQueueMask GpuQueueMask::kAll = GpuQueueMask(~0u);

GpuQueue::GpuQueue(GpuDevice& gpuDevice, GpuQueueType type, u32 index)
	:mGpuDevice(gpuDevice), mType(type), mIndex(index)
{
}

void GpuDevice::SubmitCommandBuffer(const SPtr<render::GpuCommandBuffer>& commandBuffer, GpuQueueMask syncMask, u32 queueIndex)
{
	if (!B3D_ENSURE(commandBuffer))
		return;

	const u32 queueCount = GetQueueCount(commandBuffer->GetQueueType());
	if (!B3D_ENSURE(queueIndex < queueCount))
		return;

	const SPtr<GpuQueue>& queue = GetQueue(commandBuffer->GetQueueType(), queueIndex);
	if (!B3D_ENSURE(queue))
		return;

	queue->SubmitCommandBuffer(commandBuffer, syncMask);
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

void GpuQueue::SubmitCommandBuffer(const SPtr<render::GpuCommandBuffer>& commandBuffer, GpuQueueMask syncMask)
{
	SubmitCommandBuffer(commandBuffer, syncMask, true);
}

const SPtr<render::GpuCommandBuffer>& GpuDevice::GetOrCreateTransferCommandBuffer()
{
	return mTransferBufferHelper->GetOrCreateTransferCommandBuffer();
}

void GpuDevice::SubmitTransferCommandBuffers(bool wait)
{
	mTransferBufferHelper->SubmitTransferCommandBuffer(wait);
}

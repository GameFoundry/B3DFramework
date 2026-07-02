//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DGpuQueue.h"

using namespace b3d;

const GpuQueueMask GpuQueueMask::kNone = GpuQueueMask(0);
const GpuQueueMask GpuQueueMask::kAll = GpuQueueMask(~0u);

GpuQueue::GpuQueue(GpuDevice& gpuDevice, GpuQueueType type, u32 index)
	:mGpuDevice(gpuDevice), mType(type), mIndex(index)
{
}

void GpuQueue::ExecuteSubmitOnSubmitThread(const TShared<render::GpuCommandBuffer>& /*commandBuffer*/, GpuQueueMask /*syncMask*/, TArrayView<const GpuTimelineFenceAndValue> /*signalFences*/)
{
	B3D_ENSURE_LOG(false, "This queue does not support submit-thread command buffer submission.");
}

void GpuQueue::RefreshCompletionState(bool /*forceWait*/, bool /*queueEmpty*/, u32 /*lastSubmitIndex*/)
{
	B3D_ENSURE_LOG(false, "This queue does not support submit-thread completion tracking.");
}

u32 GpuQueue::GetLastSubmitIndex() const
{
	B3D_ENSURE_LOG(false, "This queue does not support submit-thread completion tracking.");
	return 0;
}

void GpuQueue::WaitUntilIdleOnSubmitThread()
{
	B3D_ENSURE_LOG(false, "This queue does not support submit-thread idle waits.");
}

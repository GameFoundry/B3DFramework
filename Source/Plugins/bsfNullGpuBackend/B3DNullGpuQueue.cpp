//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DNullGpuQueue.h"
#include "B3DNullGpuDevice.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"

namespace b3d
{
	namespace render
	{
		NullGpuQueue::NullGpuQueue(GpuDevice& device, GpuQueueType type, u32 index)
			: GpuQueue(device, type, index)
		{ }

		void NullGpuQueue::SubmitCommandBuffer(const GpuSubmissionInformation& information)
		{
			if(information.CommandBuffer == nullptr)
				return;

			// The submit thread runs the uploads the command buffer uses first, and then completes it immediately
			mGpuDevice.GetSubmitThread().QueueSubmit(information.CommandBuffer, *this, information.SyncMask, information.SignalFences);
		}

		void NullGpuQueue::WaitUntilIdle()
		{
			// Waits for the submit thread to run the submissions queued before, including the copies of the uploads they use. Nothing else
			// runs during device startup and shutdown, when the submit thread doesn't exist.
			if(mGpuDevice.HasSubmitThread())
				mGpuDevice.GetSubmitThread().WaitUntilIdle(*this);
		}
	} // namespace render
} // namespace b3d

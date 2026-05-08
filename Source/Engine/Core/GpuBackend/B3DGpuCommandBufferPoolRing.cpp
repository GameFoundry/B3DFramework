//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DGpuCommandBufferPoolRing.h"
#include "B3DGpuCommandBuffer.h"
#include "B3DGpuDevice.h"

using namespace b3d;
using namespace b3d::render;

GpuCommandBufferPoolRing::GpuCommandBufferPoolRing(GpuDevice& gpuDevice, const GpuCommandBufferPoolCreateInformation& createInformation)
{
	for(u32 poolIndex = 0; poolIndex < kPoolCount; ++poolIndex)
		mPools[poolIndex] = gpuDevice.CreateGpuCommandBufferPool(createInformation);
}

GpuCommandBufferPoolRing::~GpuCommandBufferPoolRing()
{
	Destroy();
}

void GpuCommandBufferPoolRing::Destroy()
{
	for(u32 poolIndex = 0; poolIndex < kPoolCount; ++poolIndex)
	{
		if(mPools[poolIndex])
			mPools[poolIndex]->Destroy();

		mPools[poolIndex] = nullptr;
	}
}

GpuCommandBufferPool& GpuCommandBufferPoolRing::GetCurrentPool() const
{
	return *mPools[mCurrentPoolIndex];
}

void GpuCommandBufferPoolRing::AdvanceFrame()
{
	// Move to next pool in ring
	mCurrentPoolIndex = (mCurrentPoolIndex + 1) % kPoolCount;

	const TShared<GpuCommandBufferPool>& commandBufferPool = mPools[mCurrentPoolIndex];

	// Ensure the messages sent by the submit thread have been processed by this point. We need this to happen as we're hardcoding
	// our pool count to kPoolCount, so the last pool must be fully processed before we reuse it again. This needs to happen after
	// GpuDevice::EndFrame(), otherwise we have no guarantee that the submit thread has finished processing command buffers submitted for this frame.
	commandBufferPool->GetMessageQueue().PostCommand([]{ }, "Process messages", true);

	// Reset the pool we're about to use (it was last used kPoolCount frames ago)
	mPools[mCurrentPoolIndex]->Reset();
}

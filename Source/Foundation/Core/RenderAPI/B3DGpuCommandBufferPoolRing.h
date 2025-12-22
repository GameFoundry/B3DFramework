//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "B3DGpuCommandBuffer.h"
#include "CoreObject/B3DRenderThread.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup RenderAPI-Internal
		 *  @{
		 */

		/**
		 * Ring buffer of command buffer pools for managing multiple in-flight frames.
		 * Maintains one pool per frame (kMaximumFramesInFlight pools).
		 * Cycles through pools as frames advance, resetting each pool when it comes back around.
		 */
		class B3D_EXPORT GpuCommandBufferPoolRing
		{
		public:
			static constexpr u32 kPoolCount = RenderThread::kMaximumFramesInFlight;

			GpuCommandBufferPoolRing() = default;
			~GpuCommandBufferPoolRing() = default;

			/** Initializes the ring by creating all pools. */
			void Initialize(GpuDevice& gpuDevice, const GpuCommandBufferPoolCreateInformation& createInformation)
			{
				for(u32 poolIndex = 0; poolIndex < kPoolCount; ++poolIndex)
					mPools[poolIndex] = gpuDevice.CreateGpuCommandBufferPool(createInformation);
			}

			/** Destroys all pools. Must be called before the GPU device is destroyed. */
			void Destroy()
			{
				for(u32 poolIndex = 0; poolIndex < kPoolCount; ++poolIndex)
				{
					if(mPools[poolIndex])
						mPools[poolIndex]->Destroy();

					mPools[poolIndex] = nullptr;
				}
			}

			/** Returns the pool for the current frame. */
			GpuCommandBufferPool& GetCurrentPool() const { return *mPools[mCurrentPoolIndex]; }

			/**
			 * Advances to the next frame's pool and resets it.
			 * Called at frame boundaries by the renderer after ensuring all command buffers
			 * from the target pool have completed execution.
			 */
			void AdvanceFrame()
			{
				// Move to next pool in ring
				mCurrentPoolIndex = (mCurrentPoolIndex + 1) % kPoolCount;

				const SPtr<GpuCommandBufferPool>& commandBufferPool = mPools[mCurrentPoolIndex];

				// Ensure the messages sent by the submit thread have been processed by this point
				commandBufferPool->GetMessageQueue().PostCommand([]{ }, "Process messages", true);

				// Reset the pool we're about to use (it was last used kPoolCount frames ago)
				mPools[mCurrentPoolIndex]->Reset();
			}

		private:
			Array<SPtr<GpuCommandBufferPool>, kPoolCount> mPools;
			u32 mCurrentPoolIndex = 0;
		};

		/** @} */
	} // namespace render
} // namespace b3d

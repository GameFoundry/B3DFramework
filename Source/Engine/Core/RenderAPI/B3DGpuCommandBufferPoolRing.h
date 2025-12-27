//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "CoreObject/B3DRenderThread.h"

namespace b3d
{
	class GpuDevice;

	namespace render
	{
		class GpuCommandBufferPool;
		struct GpuCommandBufferPoolCreateInformation;

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

			GpuCommandBufferPoolRing(GpuDevice& gpuDevice, const GpuCommandBufferPoolCreateInformation& createInformation);
			~GpuCommandBufferPoolRing();

			/** Destroys all pools. Must be called before the GPU device is destroyed. */
			void Destroy();

			/** Returns the pool for the current frame. */
			GpuCommandBufferPool& GetCurrentPool() const;

			/**
			 * Advances to the next frame's pool and resets it.
			 * Called at frame boundaries by the renderer after ensuring all command buffers
			 * from the target pool have completed execution.
			 */
			void AdvanceFrame();

		private:
			Array<SPtr<GpuCommandBufferPool>, kPoolCount> mPools;
			u32 mCurrentPoolIndex = 0;
		};

		/** @} */
	} // namespace render
} // namespace b3d

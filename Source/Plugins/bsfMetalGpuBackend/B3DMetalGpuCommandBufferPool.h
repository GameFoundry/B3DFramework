//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DMetalPrerequisites.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"

namespace b3d
{
	namespace render
	{
		class MetalGpuDevice;

		/** @addtogroup MetalGpuBackend
		 *  @{
		 */

		/** Metal implementation of GpuCommandBufferPool. */
		class MetalGpuCommandBufferPool : public GpuCommandBufferPool
		{
			using Base = GpuCommandBufferPool;
		public:
			MetalGpuCommandBufferPool(MetalGpuDevice& device, const GpuCommandBufferPoolCreateInformation& createInformation);
			~MetalGpuCommandBufferPool() override;

			TShared<GpuCommandBuffer> Create(const GpuCommandBufferCreateInformation& createInformation) override;
			TShared<GpuCommandBuffer> FindOrCreate(const GpuCommandBufferCreateInformation& createInformation) override;
			void Reset() override {}
			void Destroy() override;

			/**
			 * Notifies the pool that a command buffer has completed on the GPU and is safe to recycle.
			 *
			 * Called from the completion-handler lambda in @c MetalGpuCommandBuffer::CommitInternal once
			 * the state has flipped to @c Done. Appends the id to the recycle free-list so the next
			 * @c FindOrCreate hand-off is O(1). Must be called on the pool's owner thread (the
			 * completion path posts back through @c GetMessageQueue() to guarantee this).
			 */
			void NotifyCommandBufferReady(u32 id);

		private:
			u32 mNextCommandBufferId = 1;
			UnorderedMap<u32, TShared<GpuCommandBuffer>> mCommandBuffers;

			// Recycle free-list. Populated by NotifyCommandBufferReady from the completion handler, and
			// drained by FindOrCreate. Keeping this as a Vector (LIFO) means the most recently-finished
			// buffer is re-handed out first, which keeps its Metal command-buffer / encoder caches warm.
			Vector<u32> mReadyIds;
		};

		/** @} */
	} // namespace render
} // namespace b3d

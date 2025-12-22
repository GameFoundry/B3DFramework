//************************************ B3D Framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "B3DGpuCommandBufferPoolRing.h"

namespace b3d
{
	class GpuQueue;

	namespace render
	{
		class GpuCommandBuffer;
		class GpuCommandBufferPoolRing;
	}

	/** @addtogroup RenderAPI-Internal
	 *  @{
	 */

	/**
	 * Helper that manages automatic creation and submission of transfer command buffers. Primarily used for buffer/texture read/write operations,
	 * in case the user doesn't provide an explicit command buffer to perform them on.
	 *
	 * Thread safe.
	 */
	class B3D_EXPORT GpuTransferBufferHelper
	{
	public:
		/** Information about transfer command buffers for a specific queue on a specific thread. */
		struct QueueData
		{
			UPtr<render::GpuCommandBufferPoolRing> PoolRing;
			SPtr<render::GpuCommandBuffer> CurrentCommandBuffer;
		};

		/** Per-thread data containing transfer buffers for all queues. */
		struct ThreadData
		{
			UnorderedMap<u32, QueueData> QueueData; /**< Per-queue transfer command buffer data. Index is the unique queue ID. */
			ThreadId OwnerThread; /**< Thread ID that owns this data. */
		};

		explicit GpuTransferBufferHelper(GpuDevice& device);
		~GpuTransferBufferHelper();

		/**
		 * Returns a command buffer for transfer operations on the current thread. If no command buffer exists for this
		 * thread, one is created. The command buffer is associated with the specified queue.
		 *
		 * @param queue		Queue that the transfer command buffer will be submitted to.
		 * @return			Transfer command buffer for the current thread.
		 */
		SPtr<render::GpuCommandBuffer> GetOrCreateTransferCommandBuffer(GpuQueue& queue);

		/**
		 * Submits the active transfer command buffer for the current thread on the specified queue.
		 * After submission, the command buffer is invalidated and a new one will be created on the next GetOrCreate call.
		 *
		 * @param queue		Queue to submit the transfer command buffer to.
		 * @param wait		If true, blocks until the command buffer finishes executing on the GPU.
		 */
		void SubmitTransferCommandBuffer(GpuQueue& queue, bool wait);

		/**
		 * Submits all active transfer command buffers for the current thread across all queues.
		 * Useful when you need to ensure all pending transfer operations from the current thread are submitted before proceeding.
		 */
		void SubmitAllTransferCommandBuffers();

		/**
		 * Advances all transfer command buffer pool rings to the next frame.
		 * Called at the end of each frame by the renderer.
		 */
		void EndFrame();

	private:

		/** Registers the current thread if not already registered. Returns the thread data. */
		ThreadData* RegisterCurrentThreadIfNeeded();

		/** Gets the thread data for the current thread, or nullptr if not registered. */
		ThreadData* GetCurrentThreadData();

		GpuDevice& mGpuDevice;

		Mutex mRegistryMutex;
		Vector<UPtr<ThreadData>> mThreadRegistry;
	};

	/** @} */

} // namespace b3d

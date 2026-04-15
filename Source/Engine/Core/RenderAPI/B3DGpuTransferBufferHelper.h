//************************************ B3D Framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "B3DGpuCommandBufferPoolRing.h"

namespace b3d
{
	class GpuQueue;
	struct GpuQueueId;

	namespace render
	{
		class GpuCommandBuffer;
		class GpuCommandBufferPoolRing;
	}

	/** @addtogroup GpuBackend-Internal
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
		/** Per-thread data containing transfer buffer state. */
		struct ThreadData
		{
			UPtr<render::GpuCommandBufferPoolRing> PoolRing; /**< Ring buffer of pools for allocating transfer command buffers. */
			SPtr<render::GpuCommandBuffer> CurrentCommandBuffer; /**< Currently active transfer buffer, if any. */
			ThreadId OwnerThread; /**< Thread ID that owns this data. */
		};

		/**
		 * Creates a transfer buffer helper for the specified device and target queue.
		 *
		 * @param device		GPU device this helper belongs to.
		 * @param targetQueue	Queue that transfer command buffers will be submitted to.
		 */
		GpuTransferBufferHelper(GpuDevice& device, GpuQueueId targetQueue);
		~GpuTransferBufferHelper();

		/**
		 * Returns a command buffer for transfer operations on the current thread. If no command buffer exists for this
		 * thread, one is created. The command buffer will be submitted to the configured target queue.
		 *
		 * This is a helper method that creates a transient transfer command buffer under the hood.
		 *
		 * @return			Transfer command buffer for the current thread.
		 */
		const SPtr<render::GpuCommandBuffer>& GetOrCreateTransferCommandBuffer();

		/**
		 * Submits the active transfer command buffer for the current thread.
		 * After submission, the command buffer is invalidated and a new one will be created on the next GetOrCreate call.
		 *
		 * @param wait		If true, blocks until the command buffer finishes executing on the GPU.
		 */
		void SubmitTransferCommandBuffer(bool wait = false);

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
		GpuQueueType mTargetQueueType;
		u32 mTargetQueueIndex;

		Mutex mRegistryMutex;
		Vector<UPtr<ThreadData>> mThreadRegistry;
	};

	/** @} */

} // namespace b3d

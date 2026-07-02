//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GpuBackend/B3DGpuQueue.h"
#include "GpuBackend/B3DGpuTimelineFence.h"
#include "Threading/B3DSignalEvent.h"
#include "Threading/B3DSingleConsumerQueue.h"

namespace b3d
{
	class GpuDevice;
	class GpuSwapChain;
}

namespace b3d::render
{
	class GpuCommandBuffer;
	class GpuCommandBufferPool;

	/** @addtogroup GpuBackend
	 *  @{
	 */

	/**
	 * Runs a worker thread responsible for executing GPU queue submit and present commands. Serializes all
	 * submission work off the render thread, tracks per-queue completion and paces frames so that at most two
	 * frames of GPU work are in flight.
	 */
	class B3D_EXPORT GpuSubmitThread
	{
		/** Groups per-frame completion tracking data together. */
		struct FrameCompletionMarker
		{
			/**
			 * Submit index of the last submission on each queue (indexed by GpuQueueId) as of this frame's boundary. Waiting on
			 * every queue up to its captured index guarantees all of the frame's GPU work has completed, not just its last
			 * command buffer.
			 */
			Array<u32, B3D_MAX_UNIQUE_QUEUES> LastSubmitIndices = {};

			/** Event signalled when this frame has been completely processed by the submit thread. */
			SignalEvent CompletionEvent;

			FrameCompletionMarker()
				: CompletionEvent(SignalEvent::Mode::ManuallyReset, true)
			{}
		};

	public:
		GpuSubmitThread(GpuDevice& gpuDevice);
		~GpuSubmitThread();

		/**
		 * Queues a command buffer submit operation to be executed on the submit thread.
		 *
		 * @param	commandBuffer	Command buffer to submit.
		 * @param	queue			Queue to submit the command buffer on.
		 * @param	syncMask		Mask that controls which other command buffers does this command buffer depend upon
		 *							(if any).
		 * @param	signalFences	Explicit list of timeline-fence + value pairs to signal when the submit completes.
		 * @param	blocking		If true the calling thread will wait until the GPU completes the operation.
		 */
		void QueueSubmit(const TShared<GpuCommandBuffer>& commandBuffer, GpuQueue& queue, GpuQueueMask syncMask, TInlineArray<GpuTimelineFenceAndValue, 2> signalFences, bool blocking = false);

		/**
		 * Queues an operation that acquires a swap chain image. Acquired images can be written to and eventually presented to the screen.
		 * Each acquire call must have a matching present call, which will unacquire the image and make it free for further acquires. Note
		 * that a limit number of images is available depending on swap chain configuration and acquire might fail.
		 */
		void QueueImageAcquire(GpuSwapChain& swapChain);

		/**
		 * Queues a GpuSwapChain::Present() operation to be executed on the submit thread.
		 *
		 * @param	queue			Queue to execute the present operation on.
		 * @param	swapChain		Swap chain whose image to present. First acquired image that hasn't yet been presented will be presented.
		 * @param	syncMask		Mask that controls which other queues does the the present depend on (if any).
		 */
		void QueuePresent(GpuQueue& queue, GpuSwapChain& swapChain, GpuQueueMask syncMask);

		/**
		 * Notifies the submit thread that last command buffer for this frame had been submitted, and waits until the previous frame's command buffers
		 * finished executing, so it's resources may be re-used.
		 */
		void QueueEndFrameAndWaitForPreviousFrame();

		/**
		 * Blocks the calling thread until all commands have finished executing.
		 *
		 * @param	performCleanupForShutdown		If true perform additional cleanup after the wait has finished. Set this to true when shutting down the submit thread.
		 */
		void WaitUntilIdle(bool performCleanupForShutdown = false);

		/** Blocks the calling thread until all commands on the provided queue have finished executing. */
		void WaitUntilIdle(GpuQueue& queue);

		/** Returns a pool that may be used for allocating command buffers for the submit thread. */
		GpuCommandBufferPool& GetCommandBufferPool(GpuQueueType queueType) const { return *mCommandBufferPools[queueType]; }

		/** Returns the id of the thread that submit work is being performed on. */
		u32 GetThreadId() const;

	protected:
		static constexpr u32 kFrameCount = 2;

		GpuDevice& mGpuDevice;
		SingleConsumerQueue mCommandQueue;
		Array<TShared<GpuCommandBufferPool>, GQT_COUNT> mCommandBufferPools;

		/** Current frame index (0 to kFrameCount-1), tracked internally by submit thread. */
		u32 mCurrentFrameIndex = 0;

		/** Per-frame completion tracking (per-queue submit-index snapshot and completion event). */
		Array<FrameCompletionMarker, kFrameCount> mFrameMarkers;
	};

	/** Asserts if the current thread isn't the submit thread. */
	B3D_EXPORT void AssertIfNotSubmitThread();

	/** @} */
} // namespace b3d::render

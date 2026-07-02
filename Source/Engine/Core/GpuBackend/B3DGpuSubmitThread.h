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
	 * Backend half of the submit thread contract. GpuSubmitThread contains the backend-agnostic orchestration
	 * (worker thread, command serialization, frame pacing) and calls into this interface for every operation
	 * that requires native GPU work. Backends that use the submit thread implement this - typically directly on
	 * their GpuDevice - and hand it to the GpuSubmitThread constructor.
	 *
	 * Implementations may downcast the provided queues/command buffers to their own types; the submit thread
	 * only ever passes objects belonging to the backend that created it.
	 */
	class B3D_EXPORT IGpuSubmitThreadBackend
	{
	public:
		virtual ~IGpuSubmitThreadBackend() = default;

		/** @name Render thread
		 *  @{
		 */

		/**
		 * Called on the command buffer's owning thread just before it is queued for submission on the submit
		 * thread. The backend should release any command buffer state that must not be touched from the submit
		 * thread.
		 */
		virtual void NotifyWillQueueForSubmit(GpuCommandBuffer& commandBuffer) = 0;

		/** @} */

		/** @name Submit thread
		 *  @{
		 */

		/**
		 * Submits a command buffer on the provided queue. The backend derives any per-command-buffer submit data
		 * it needs from @p commandBuffer internally.
		 *
		 * @param	queue			Queue to submit the command buffer on.
		 * @param	commandBuffer	Command buffer to submit.
		 * @param	syncMask		Inter-queue synchronization mask.
		 * @param	signalFences	Timeline-fence + value pairs to signal when the submit completes.
		 */
		virtual void ExecuteSubmit(GpuQueue& queue, const TShared<GpuCommandBuffer>& commandBuffer, GpuQueueMask syncMask, TArrayView<const GpuTimelineFenceAndValue> signalFences) = 0;

		/**
		 * Checks if any of the active command buffers finished executing on the provided queue and updates their
		 * states accordingly.
		 *
		 * @param	queue			Queue whose completion state to refresh.
		 * @param	forceWait		If true, waits until the relevant command buffers finish executing.
		 * @param	queueEmpty		If true, the caller guarantees the queue will be empty (e.g. on shutdown),
		 *							allowing all needed resources to be freed.
		 * @param	lastSubmitIndex	Index of the last submitted command buffer to check. If ~0u, all submitted
		 *							command buffers are checked.
		 */
		virtual void RefreshCompletionState(GpuQueue& queue, bool forceWait, bool queueEmpty = false, u32 lastSubmitIndex = ~0u) = 0;

		/**
		 * Returns the submit index of the most recently submitted work (command buffer or present) on the
		 * provided queue, or 0 if nothing has been submitted yet. Captured at a frame boundary and passed to
		 * RefreshCompletionState() to wait for all of that frame's work to complete.
		 */
		virtual u32 GetLastSubmitIndex(const GpuQueue& queue) const = 0;

		/** Blocks until all work on the device finishes executing on the GPU, using the backend's native wait. */
		virtual void ExecuteWaitUntilIdle() = 0;

		/** Blocks until all work submitted on the provided queue finishes executing on the GPU, using the backend's native wait. */
		virtual void ExecuteWaitUntilIdle(GpuQueue& queue) = 0;

		/** @} */
	};

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
		/**
		 * Starts the submit thread.
		 *
		 * @param	gpuDevice	Device whose queues the submit thread operates on.
		 * @param	backend		Backend implementation used for executing the native GPU operations. Must remain
		 *						valid for the lifetime of the submit thread.
		 */
		GpuSubmitThread(GpuDevice& gpuDevice, IGpuSubmitThreadBackend& backend);
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
		IGpuSubmitThreadBackend& mBackend;
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

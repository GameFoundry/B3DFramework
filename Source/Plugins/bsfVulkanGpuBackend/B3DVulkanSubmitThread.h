//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DVulkanPrerequisites.h"
#include "B3DVulkanGpuQueue.h"
#include "GpuBackend/B3DGpuTimelineFence.h"
#include "CoreObject/B3DRenderThread.h"
#include "Threading/B3DSignalEvent.h"
#include "Threading/B3DSingleConsumerQueue.h"
#include "Utility/B3DModule.h"

namespace b3d::render
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/** Runs a worker thread responsible for executing Vulkan queue submit and present commands. */
	class VulkanSubmitThread : public Module<VulkanSubmitThread>
	{
		/** Groups per-frame completion tracking data together. */
		struct FrameCompletionMarker
		{
			/** Last command buffer submitted for this frame. */
			TShared<VulkanGpuCommandBuffer> LastCommandBuffer;

			/** Event signalled when this frame has been completely processed by the submit thread. */
			SignalEvent CompletionEvent;

			FrameCompletionMarker()
				: CompletionEvent(SignalEvent::Mode::ManuallyReset, true)
			{}
		};

	public:
		VulkanSubmitThread(VulkanGpuDevice& gpuDevice);
		~VulkanSubmitThread();

		/**
		 * Queues a VulkanCmdBuffer::Submit() operation to be executed on the submit thread.
		 *
		 * @param	commandBuffer	Command buffer to submit.
		 * @param	queue			Queue to submit the command buffer on.
		 * @param	syncMask		Mask that controls which other command buffers does this command buffer depend upon
		 *							(if any).
		 * @param	signalFences	Explicit list of timeline-fence + value pairs to signal when the submit completes.
		 * @param	blocking		If true the calling thread will wait until the GPU completes the operation.
		 */
		void QueueSubmit(const TShared<VulkanGpuCommandBuffer>& commandBuffer, VulkanGpuQueue& queue, GpuQueueMask syncMask, TInlineArray<GpuTimelineFenceAndValue, 2> signalFences, bool blocking = false);

		/**
		 * Queues an operation that acquires a swap chain image. Acquired images can be written to and eventually presented to the screen.
		 * Each acquire call must have a matching present call, which will unacquire the image and make it free for further acquires. Note
		 * that a limit number of images is available depending on swap chain configuration and acquire might fail.
		 */
		void QueueImageAcquire(VulkanSwapChain& swapChain);

		/**
		 * Queues a VulkanSwapChain::Present() operation to be executed on the submit thread. 
		 *
		 * @param	queue			Queue to execute the present operation on.
		 * @param	swapChain		Swap chain whose image to present. First acquired image that hasn't yet been presented will be presented.
		 * @param	syncMask		Mask that controls which other queues does the the present depend on (if any). 
		 */
		void QueuePresent(VulkanGpuQueue& queue, VulkanSwapChain& swapChain, GpuQueueMask syncMask);

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
		void WaitUntilIdle(VulkanGpuQueue& queue);

		/** Returns a pool that may be used for allocating command buffers for the submit thread. */
		VulkanGpuCommandBufferPool& GetCommandBufferPool(GpuQueueType queueType) const { return *mCommandBufferPools[queueType]; }

		/** Returns the id of the thread that submit work is being performed on. */
		u32 GetThreadId() const;

	protected:
		static constexpr u32 kFrameCount = 2;

		VulkanGpuDevice& mGpuDevice;
		SingleConsumerQueue mCommandQueue;
		Array<TShared<VulkanGpuCommandBufferPool>, GQT_COUNT> mCommandBufferPools;

		/** Current frame index (0 to kFrameCount-1), tracked internally by submit thread. */
		u32 mCurrentFrameIndex = 0;

		/** Per-frame completion tracking (last command buffer and completion event). */
		Array<FrameCompletionMarker, kFrameCount> mFrameMarkers;

		/** Tracks the command buffer being built up as "last" during the current frame. */
		TShared<VulkanGpuCommandBuffer> mCurrentFrameLastCommandBuffer;
	};

	/** Retrieves an instance of VulkanSubmitThread. */
	VulkanSubmitThread& GetVulkanSubmitThread();

	/**	Asserts if the current thread isn't the Vulkan submit thread. */
	void AssertIfNotVulkanSubmitThread();

	/** @} */
} // namespace b3d::render

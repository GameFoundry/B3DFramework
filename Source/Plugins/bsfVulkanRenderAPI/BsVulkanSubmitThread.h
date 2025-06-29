//************************************ B3D Framework - Copyright 2022 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "Threading/BsSignal.h"
#include "Threading/BsSingleConsumerQueue.h"
#include "Utility/BsModule.h"

namespace b3d::render
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/** Runs a worker thread responsible for executing Vulkan queue submit and present commands. */
	class VulkanSubmitThread : public Module<VulkanSubmitThread>
	{
	public:
		VulkanSubmitThread(VulkanGpuDevice& gpuDevice);
		~VulkanSubmitThread();

		/**
		 * Queues a VulkanCmdBuffer::Submit() operation to be executed on the submit thread.
		 *
		 * @param	commandBuffer	Command buffer to submit.
		 * @param	queue			Queue to submit the command buffer on.
		 * @param	syncMask		Mask that controls which other command buffers does this command buffer depend upon
		 *							(if any). See description of @p syncMask parameter in RenderAPI::ExecuteCommands().
		 * @param	blocking		If true the calling thread will wait until the GPU completes the operation.
		 */
		void QueueSubmit(const SPtr<VulkanGpuCommandBuffer>& commandBuffer, VulkanGpuQueue& queue, u32 syncMask, bool blocking = false);

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
		 * @param	syncMask		Mask that controls which command buffers submissions does the present depend on
		 *							(if any). See description of @p syncMask parameter in RenderAPI::ExecuteCommands().
		 */
		void QueuePresent(VulkanGpuQueue& queue, VulkanSwapChain& swapChain, u32 syncMask);

		/**
		 * Queues an operation that checks the completion status of any command buffers submitted on the provided device. This needs to be followed by
		 * RefreshCommandBufferCompletionStates() in order for the change to register on the render thread.
		 */
		void QueueRefreshCommandBufferCompletionStates(const VulkanGpuDevice* device);

		/**
		 * Blocks the calling thread until all commands have finished executing.
		 *
		 * @param	performCleanupForShutdown		If true perform additional cleanup after the wait has finished. Set this to true when shutting down the submit thread.
		 */
		void WaitUntilIdle(bool performCleanupForShutdown = false);

		/** Blocks the calling thread until all commands on the provided queue have finished executing. */
		void WaitUntilIdle(VulkanGpuQueue& queue);

		/** Returns a pool that may be used for allocating command buffers for the submit thread. */
		VulkanGpuCommandBufferPool& GetCommandBufferPool(GpuQueueUsage queueUsage) const { return *mCommandBufferPools[queueUsage]; }

		/** Returns the id of the thread that submit work is being performed on. */
		u32 GetThreadId() const;

	protected:
		VulkanGpuDevice& mGpuDevice;
		SingleConsumerQueue mCommandQueue;
		Array<SPtr<VulkanGpuCommandBufferPool>, GQT_COUNT> mCommandBufferPools;
	};

	/** Retrieves an instance of VulkanSubmitThread. */
	VulkanSubmitThread& GetVulkanSubmitThread();

	/**	Asserts if the current thread isn't the Vulkan submit thread. */
	void AssertIfNotVulkanSubmitThread();

	/** @} */
} // namespace b3d::render

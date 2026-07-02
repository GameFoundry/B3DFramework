//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GpuBackend/B3DGpuQueue.h"
#include "GpuBackend/Allocators/B3DGpuResource.h"

namespace b3d
{
	class SingleConsumerQueue;

	/** @addtogroup GpuBackend
	 *  @{
	 */

	/**
	 * Represents a set of images that can be acquired for rendering and eventually presented to the display. 
	 * Used by the submit thread at acquire back buffer images and present acquired images. Used by the
	 * render thread so the command buffer can retrieve the active back buffer index.
	 */
	class B3D_EXPORT GpuSwapChain : public IGpuSwapChainResource
	{
	public:
		GpuSwapChain(GpuResourceManager* owner, const StringView& name)
			: IGpuSwapChainResource(owner, name)
		{}

		/** Returns a thread-safe message queue that may be used for posting messages to the thread responsible for the swap chain (usually the render thread). */
		virtual SingleConsumerQueue& GetMessageQueue() = 0;

		/** @name Submit thread
		 *  @{
		 */

		/**
		 * Attempts to acquire an image that can be rendered to and eventually presented. Each acquire must be
		 * matched by a present, which frees the image for future acquires. Only a limited number of images is
		 * available depending on swap chain configuration, and acquire may fail (e.g. an out-of-date swap chain),
		 * in which case a swap chain rebuild should be attempted.
		 *
		 * @note	Submit thread only.
		 */
		virtual void AcquireImage() = 0;

		/**
		 * Presents a previously acquired image on the provided queue. The image must have been acquired and not
		 * yet presented.
		 *
		 * @param	imageIndex	Index of the image to present. Must have been previously acquired.
		 * @param	queue		Queue to submit the operation on. Must support present operations.
		 * @param	syncMask	Mask that controls which other queues the present depends upon (if any).
		 *
		 * @note	Submit thread only.
		 */
		virtual void Present(u32 imageIndex, GpuQueue& queue, GpuQueueMask syncMask) = 0;

		/** @} */

		/** @name Render thread
		 *  @{
		 */

		/**
		 * Retrieves the image index of the first acquired image that hasn't yet been queued for present. Returns
		 * false if no such image is available. After NotifyWasPresentQueued() is called this starts returning the
		 * next available acquired image, if any.
		 */
		virtual bool TryGetFirstAcquiredImageIndex(u32& outImageIndex) const = 0;

		/** Notifies that an image has been queued for acquire on the submit thread. */
		virtual void NotifyWasImageAcquireQueued() = 0;

		/** Notifies the swap chain that the specified image has been queued for present. This prevents it from being returned by TryGetFirstAcquiredImageIndex(). */
		virtual void NotifyWasPresentQueued(u32 imageIndex) = 0;

		/** Checks if the swap chain is retired. A retired swap chain can still present already-acquired images, but cannot acquire new ones. */
		virtual bool IsRetired() const = 0;

		/** @} */

	protected:
		GpuSwapChain() = default;
	};

	/** @} */
} // namespace b3d

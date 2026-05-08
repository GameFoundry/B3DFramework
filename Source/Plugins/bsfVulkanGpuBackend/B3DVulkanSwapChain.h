//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DVulkanPrerequisites.h"
#include "B3DVulkanFramebuffer.h"
#include "B3DVulkanGpuQueue.h"
#include "Threading/B3DSingleConsumerQueue.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup Vulkan
		 *  @{
		 */

		/** Description of a single swap chain surface. */
		struct SwapChainImage
		{
			VulkanImage* Image = VK_NULL_HANDLE;
			VulkanSemaphore* WaitSemaphore = VK_NULL_HANDLE; /**< Semaphore to wait on, only valid for acquired images. */
			bool Acquired = false;
			bool NeedsWait = false;

			VulkanFramebuffer* Framebuffer = nullptr;
			VulkanFramebufferInformation FramebufferInformation;
		};

		/** Result of a swap chain acquire operation. */
		struct ImageAcquireResult
		{
			ImageAcquireResult(VkResult resultCode = VK_ERROR_UNKNOWN, u32 acquiredImageIndex = 0)
				: ResultCode(resultCode), AcquiredImageIndex(acquiredImageIndex)
			{
			}

			VkResult ResultCode = VK_ERROR_UNKNOWN;
			u32 AcquiredImageIndex = 0;
		};

		/** Wraps a Vulkan surface. */
		class VulkanSurface
		{
		public:
			VulkanSurface(VkSurfaceKHR surface)
				: mSurface(surface)
			{}

			~VulkanSurface();

			/** Returns the underlying Vulkan surface object. */
			VkSurfaceKHR GetVkHandle() const { return mSurface; }

		private:
			VkSurfaceKHR mSurface;
		};

		/** Vulkan swap chain containing two or more buffers for rendering and presenting onto the screen. */
		class VulkanSwapChain : public VulkanResource, INonCopyable
		{
			using Base = VulkanResource;
		public:
			/**
			 * Creates the swap chain with the provided properties. Destroys any previously existing swap chain. Caller must
			 * ensure the swap chain is not used at the device when this is called.
			 */
			VulkanSwapChain(VulkanResourceManager* owner, const TShared<VulkanSurface>& surface, u32 width, u32 height, bool vsync, VkFormat colorFormat, VkColorSpaceKHR colorSpace, bool createDepth, VkFormat depthFormat, VulkanSwapChain* oldSwapChain = nullptr, const StringView& name = "");
			~VulkanSwapChain();

			void Destroy() override;

			/** Returns a thread safe message queue that may be used for posting messages to the thread responsible for the swap chain. */
			SingleConsumerQueue& GetMessageQueue() { return mMessageQueue; }

			/**
			 * Returns the actual width of the swap chain, in pixels. This might differ from the requested size in case it
			 * wasn't supported.
			 */
			u32 GetWidth() const { return mWidth; }

			/**
			 * Returns the actual height of the swap chain, in pixels. This might differ from the requested size in case it
			 * wasn't supported.
			 */
			u32 GetHeight() const { return mHeight; }

			/**
			 * Attempts to acquire a new swap chain image. Caller can retrieve the surface by calling GetImage(). Caller
			 * must wait on the semaphore provided by the surface before rendering to it. Method might fail if the swap
			 * chain is no longer valid, and failure result will be returned. If this happens a swap chain rebuild
			 * should be attempted. If successful index of the returned image will be returned.
			 *
			 * @note	Submit thread only.
			 */
			ImageAcquireResult AcquireImage();

			/**
			 * Blocks the calling thread until acquire operations for all swap chains complete. If there are multiple acquire operations queued for the 
			 * swap chain then the method only waits until the first queued operation completes.
			 */
			void WaitUntilFirstImageAcquired();

			/**
			 * Retrieves the image index of the first acquired image. Returns false if no image is acquired. After
			 * NotifyImagePresented() is called this method will start returning the next available acquired image index, if
			 * any was acquired. To notify a new image was acquired used NotifyImageAcquired().
			 */
			bool TryGetFirstAcquiredImageIndex(u32& outImageIndex) const;

			/**
			 * Issues the swap chain present operation on the provided queue. The first acquired but not presented image will
			 * be presented.
			 *
			 * @param	imageIndex	Index of the image to present. Must have been previously acquired, and image semaphore waited on
			 *						before presenting.
			 * @param	queue		Queue to submit the operation on. Queue must support present operations.
			 * @param	syncMask	Mask that controls which other command buffers does the present depend upon
			 *						(if any). 
			 *
			 * @note	Submit thread only.
			 */
			void Present(u32 imageIndex, VulkanGpuQueue& queue, GpuQueueMask syncMask);

			/** Returns the number of available color images. */
			u32 GetColorImageCount() const { return (u32)mSurfaces.size(); }

			/** Returns the internal swap chain handle. */
			VkSwapchainKHR GetHandle() const { return mSwapChain; }

			/** Returns a framebuffer that can be used for rendering to the requested swap chain image. */
			VulkanFramebuffer* GetFramebufferForImage(u32 imageIndex) const { return mSurfaces[imageIndex].Framebuffer; }

			/**
			 * If the image at the provided index requires a wait before it can be rendered to, this will append the required
			 * semaphores to @p outSemaphores.
			 * 
			 * @param	imageIndex		Index of the swap chain image which might require the wait.
			 * @param	outSemaphores	Array in which to add the wait semaphore if needed.
			 * @return					True if the wait semaphore was added, or false otherwise.
			 */
			bool AppendWaitSemaphoreIfRequired(u32 imageIndex, TInlineArray<VulkanSemaphore*, 8>& outSemaphores);

			/** Lets the swap chain know that it is invalid and it should rebuilt itself. */
			void MarkAsInvalid() { mIsValid = false; }

			/** Swap chain that was marked as invalid cannot be used for acquiring more images, instead the caller should trigger a swap chain rebuild. */
			bool IsValid() const { return mIsValid; }

			/** Marks the swap chain as retired. Retired swap chain can still be used for presenting images that were already acquired, but you cannot acquire new images. */
			void MarkAsRetired() { mIsRetired = true; }

			/** Checks if the swap chain is retired. */
			bool IsRetired() const { return mIsRetired; }

			/** Notifies that an image has been queued for acquire on the submit thread. */
			void NotifyWasImageAcquireQueued();

			/** Notifies the swap chain that the specified image has been queued for present. This prevents it from being returned by GetFirstAcquiredImageIndex(). */
			void NotifyWasPresentQueued(u32 imageIndex);
		private:
			VkDevice mDevice = VK_NULL_HANDLE;
			VkSwapchainKHR mSwapChain = VK_NULL_HANDLE;
			TShared<VulkanSurface> mSurface;

			u32 mWidth = 0;
			u32 mHeight = 0;
			TInlineArray<SwapChainImage, 4> mSurfaces;
			TInlineArray<VulkanSemaphore*, 4> mSemaphores;
			TInlineArray<u32, 4> mAcquiredImageIndicesOnRenderThread;
			u32 mAcquiredImageCountOnSubmitThread = 0;

			u32 mLastAcquiredSemaphoreIndex = 0;
			bool mIsSwapChainOutdated = false;
			bool mIsValid = true;
			bool mIsRetired = false;

			Mutex mImageAcquireMutex;
			Signal mImageAcquireSignal;
			u32 mQueuedImageAcquireOperationCount = 0;
			TInlineArray<ImageAcquireResult, 4> mImageAcquireResults;

			VulkanImage* mDepthStencilImage = nullptr;
			TInlineArray<VulkanSemaphore*, 8> mSemaphoresBuffer;

			SingleConsumerQueue mMessageQueue;
		};

		/** @} */
	} // namespace render
} // namespace b3d

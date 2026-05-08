//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DVulkanPrerequisites.h"
#include "GpuBackend/B3DRenderWindow.h"

namespace b3d::render
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	class VulkanSubmitThread;
	class VulkanGpuQueue;

	/**
	 * Vulkan-specific interface for render window surfaces. Used as a common interface for regular Vulkan surfaces backed
	 * by a window, and faux surfaces for headless (offscreen) rendering scenarios.
	 */
	class IVulkanRenderWindowSurface : public IRenderWindowSurface
	{
	public:
		/**
		 * Returns the framebuffer associated with the currently active swap chain image.
		 * If no swap chain image has been acquired yet (e.g. a new swap chain), swap chain images
		 * will be queued for acquisition and the method will wait until one is available. If
		 * @p acquireIfUnavailable is false, the method will return nullptr instead of waiting.
		 */
		virtual VulkanFramebuffer* GetActiveFramebuffer(bool acquireIfUnavailable = true) = 0;

		/**
		 * If the currently active image requires a wait before it can be rendered to, this will append the required
		 * semaphores to @p outSemaphores.
		 * 
		 * @param	outSemaphores	Array in which to add the wait semaphore if needed.
		 * @return					True if the wait semaphore was added, or false otherwise.
		 */
		virtual bool AppendWaitSemaphoresIfRequired(TInlineArray<VulkanSemaphore*, 8>& outSemaphores) = 0;

		/**
		 * Returns true if the underlying swap chain is valid and can be used for acquiring images. If it's not valid
		 * you should rebuild the swap chain from the owning RenderWindow, as most likely the window size changed.
		 */
		virtual bool IsSwapChainValid() const = 0;

		/** Returns the underlying Vulkan swap chain, or nullptr if the surface doesn't use a swap chain (e.g. headless). */
		virtual VulkanSwapChain* GetSwapChain() const { return nullptr; }

		/** Returns the VulkanImage for the currently active color surface. Used for reading back the rendered frame. */
		virtual VulkanImage* GetCurrentColorImage() const = 0;

		/** Returns the pixel format of the color surface. Used for reading back the rendered frame. */
		virtual PixelFormat GetColorPixelFormat() const = 0;

		TAsyncOp<TShared<PixelData>> ReadAsync(GpuCommandBuffer& commandBuffer) override;
	};

	/** @} */
} // namespace b3d::render

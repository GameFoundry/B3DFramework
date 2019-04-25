//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "RenderAPI/BsRenderWindow.h"

namespace bs
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/**
	 * Render window implementation for Windows and Vulkan.
	 *
	 * @note	Sim thread only.
	 */
	class Win32RenderWindow : public RenderWindow
	{
	public:
		~Win32RenderWindow() = default;

		/** @copydoc RenderWindow::screenToWindowPos */
		void getCustomAttribute(const String& name, void* pData) const override;

		/** @copydoc RenderWindow::screenToWindowPos */
		Vector2I screenToWindowPos(const Vector2I& screenPos) const override;

		/** @copydoc RenderWindow::windowToScreenPos */
		Vector2I windowToScreenPos(const Vector2I& windowPos) const override;

		/** @copydoc RenderWindow::getCore */
		SPtr<ct::Win32RenderWindow> getCore() const;

		/**	Retrieves internal window handle. */
		HWND getHWnd() const;

	protected:
		friend class VulkanRenderWindowManager;
		friend class ct::Win32RenderWindow;

		Win32RenderWindow(const RENDER_WINDOW_DESC& desc, UINT32 windowId);

		/** @copydoc RenderWindow::getProperties */
		const RenderTargetProperties& getPropertiesInternal() const override { return mProperties; }

		/** @copydoc RenderWindow::syncProperties */
		void syncProperties() override;

		/** @copydoc RenderWindow::createCore */
		SPtr<ct::CoreObject> createCore() const override;

	private:
		RenderWindowProperties mProperties;
	};

	namespace ct
	{
	/**
	 * Render window implementation for Windows and Vulkan.
	 *
	 * @note	Core thread only.
	 */
	class Win32RenderWindow : public RenderWindow
	{
	public:
		Win32RenderWindow(const RENDER_WINDOW_DESC& desc, UINT32 windowId, VulkanRenderAPI& renderAPI);
		~Win32RenderWindow();

		/** @copydoc RenderWindow::move */
		void move(INT32 left, INT32 top) override;

		/** @copydoc RenderWindow::resize */
		void resize(UINT32 width, UINT32 height) override;

		/** @copydoc RenderWindow::setHidden */
		void setHidden(bool hidden) override;

		/** @copydoc RenderWindow::setActive */
		void setActive(bool state) override;

		/** @copydoc RenderWindow::minimize */
		void minimize() override;

		/** @copydoc RenderWindow::maximize */
		void maximize() override;

		/** @copydoc RenderWindow::restore */
		void restore() override;

		/** @copydoc RenderWindow::setFullscreen(UINT32, UINT32, float, UINT32) */
		void setFullscreen(UINT32 width, UINT32 height, float refreshRate = 60.0f, UINT32 monitorIdx = 0) override;

		/** @copydoc RenderWindow::setFullscreen(const VideoMode&) */
		void setFullscreen(const VideoMode& videoMode) override;

		/** @copydoc RenderWindow::setWindowed */
		void setWindowed(UINT32 width, UINT32 height) override;
	
		/** @copydoc RenderWindow::setVSync */
		void setVSync(bool enabled, UINT32 interval = 1) override;

		/** Prepares the back buffer for rendering. Should be called before it is bound to the GPU. */
		void acquireBackBuffer();

		/** @copydoc RenderWindow::swapBuffers */
		void swapBuffers(UINT32 syncMask = 0xFFFFFFFF) override;

		/** @copydoc RenderWindow::getCustomAttribute */
		void getCustomAttribute(const String& name, void* data) const override;

		/** @copydoc RenderWindow::_windowMovedOrResized */
		void _windowMovedOrResized() override;

		/**	Returns internal window handle. */
		HWND _getWindowHandle() const;
	protected:
		friend class bs::Win32RenderWindow;

		/** @copydoc CoreObject::initialize */
		void initialize() override;

		/** @copydoc RenderWindow::getProperties */
		const RenderTargetProperties& getPropertiesInternal() const override { return mProperties; }

		/** @copydoc RenderWindow::getSyncedProperties */
		RenderWindowProperties& getSyncedProperties() override { return mSyncedProperties; }

		/** @copydoc RenderWindow::syncProperties */
		void syncProperties() override;

		/** Rebuilds the swap chain according to the currently set properties. */
		void rebuildSwapChain();

	protected:
		Win32Window* mWindow;
		bool mIsChild;
		bool mShowOnSwap;
		INT32 mDisplayFrequency;

		VulkanRenderAPI& mRenderAPI;
		VkSurfaceKHR mSurface;
		VkColorSpaceKHR mColorSpace;
		VkFormat mColorFormat;
		VkFormat mDepthFormat;
		UINT32 mPresentQueueFamily;
		VulkanSwapChain* mSwapChain = nullptr;
		VulkanSemaphore* mSemaphoresTemp[BS_MAX_UNIQUE_QUEUES + 1]; // +1 for present semaphore
		bool mRequiresNewBackBuffer;

		RenderWindowProperties mProperties;
		RenderWindowProperties mSyncedProperties;
	};	
	}
	
	/** @} */
}

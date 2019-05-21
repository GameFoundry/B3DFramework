//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "RenderAPI/BsRenderWindow.h"

namespace bs
{
	class CocoaWindow;

	namespace ct
	{
		class MacOSRenderWindow;
	}

	/** @addtogroup Vulkan
	 *  @{
	 */

	/**
	 * Render window implementation for MacOS.
	 *
	 * @note	Sim thread only.
	 */
	class MacOSRenderWindow : public RenderWindow
	{
	public:
		~MacOSRenderWindow() = default;

		/** @copydoc RenderWindow::getCustomAttribute */
		void getCustomAttribute(const String& name, void* pData) const override;

		/** @copydoc RenderWindow::screenToWindowPos */
		Vector2I screenToWindowPos(const Vector2I& screenPos) const override;

		/** @copydoc RenderWindow::windowToScreenPos */
		Vector2I windowToScreenPos(const Vector2I& windowPos) const override;

		/** @copydoc RenderWindow::resize */
		void resize(UINT32 width, UINT32 height) override;

		/** @copydoc RenderWindow::move */
		void move(INT32 left, INT32 top) override;

		/** @copydoc RenderWindow::hide */
		void hide() override;

		/** @copydoc RenderWindow::show */
		void show() override;

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

		/** @copydoc RenderWindow::getCore */
		SPtr<ct::MacOSRenderWindow> getCore() const;

		/** Called when window is moved or resized. */
		void _windowMovedOrResized() override;

	protected:
		friend class VulkanRenderWindowManager;
		friend class ct::MacOSRenderWindow;

		MacOSRenderWindow(const RENDER_WINDOW_DESC& desc, UINT32 windowId);

		/** Changes the display mode (resolution, refresh rate) of the specified output device. */
		void setDisplayMode(const VideoOutputInfo& output, const VideoMode& mode);

		/** @copydoc RenderWindow::getProperties */
		const RenderTargetProperties& getPropertiesInternal() const override { return mProperties; }

		/** @copydoc RenderWindow::syncProperties */
		void syncProperties() override;

		/** @copydoc CoreObject::initialize() */
		void initialize() override;

		/** @copydoc CoreObject::destroy() */
		void destroy() override;

		/** @copydoc RenderTarget::createCore */
		SPtr<ct::CoreObject> createCore() const override;

	private:
		CocoaWindow* mWindow = nullptr;
		bool mIsChild = false;

		RenderWindowProperties mProperties;
	};

	namespace ct
	{
		/**
		 * Render window implementation for MacOS.
		 *
		 * @note	Core thread only.
		 */
		class MacOSRenderWindow : public RenderWindow
		{
		public:
			MacOSRenderWindow(const RENDER_WINDOW_DESC& desc, UINT32 renderWindowId, UINT32 cocoaWindowId);
			~MacOSRenderWindow();

			/** @copydoc RenderWindow::move */
			void move(INT32 left, INT32 top) override;

			/** @copydoc RenderWindow::resize */
			void resize(UINT32 width, UINT32 height) override;

			/** @copydoc RenderWindow::setVSync */
			void setVSync(bool enabled, UINT32 interval = 1) override;

			/** Prepares the back buffer for rendering. Should be called before it is bound to the GPU. */
			void acquireBackBuffer();

			/** @copydoc RenderWindow::swapBuffers */
			void swapBuffers(UINT32 syncMask) override;

			/** @copydoc RenderWindow::getCustomAttribute */
			void getCustomAttribute(const String& name, void* pData) const override;

			/** Returns a lock that can be used for accessing synced properties. */
			SpinLock& _getPropertiesLock() { return mLock;}

		protected:
			friend class MacOSGLSupport;

			/** @copydoc RenderWindow::getProperties */
			const RenderTargetProperties& getPropertiesInternal() const override { return mProperties; }

			/** @copydoc RenderWindow::getSyncedProperties */
			RenderWindowProperties& getSyncedProperties() override { return mSyncedProperties; }

			/** @copydoc RenderWindow::syncProperties */
			void syncProperties() override;

			/** @copydoc RenderWindow::initialize */
			void initialize() override;

			/** Rebuilds the swap chain according to the currently set properties. */
			void rebuildSwapChain();
		protected:
			friend class bs::MacOSRenderWindow;

			VulkanRenderAPI& mRenderAPI;
			VkSurfaceKHR mSurface;
			VkColorSpaceKHR mColorSpace;
			VkFormat mColorFormat;
			VkFormat mDepthFormat;
			UINT32 mPresentQueueFamily;
			VulkanSwapChain* mSwapChain = nullptr;
			VulkanSemaphore* mSemaphoresTemp[BS_MAX_UNIQUE_QUEUES + 1]; // +1 for present semaphore
			bool mRequiresNewBackBuffer = true;
			bool mShowOnSwap;
			UINT32 mCocoaWindowId;

			RenderWindowProperties mProperties;
			RenderWindowProperties mSyncedProperties;
		};
	}

	/** @} */
}


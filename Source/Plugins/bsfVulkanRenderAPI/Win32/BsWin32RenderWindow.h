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

		void GetCustomAttribute(const String& name, void* pData) const override;
		Vector2I ScreenToWindowPos(const Vector2I& screenPos) const override;
		Vector2I WindowToScreenPos(const Vector2I& windowPos) const override;

		/** @copydoc RenderWindow::GetCore */
		SPtr<ct::Win32RenderWindow> GetCore() const;

		/**	Retrieves internal window handle. */
		HWND GetHWnd() const;

	protected:
		friend class VulkanRenderWindowManager;
		friend class ct::Win32RenderWindow;

		Win32RenderWindow(const RENDER_WINDOW_DESC& desc, u32 windowId);

		const RenderTargetProperties& GetPropertiesInternal() const override { return mProperties; }
		void SyncProperties() override;
		SPtr<ct::CoreObject> CreateCore() const override;

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
			Win32RenderWindow(const RENDER_WINDOW_DESC& desc, u32 windowId, VulkanRenderAPI& renderAPI);
			~Win32RenderWindow();

			void Move(i32 left, i32 top) override;
			void Resize(u32 width, u32 height) override;
			void SetHidden(bool hidden) override;
			void SetActive(bool state) override;
			void Minimize() override;
			void Maximize() override;
			void Restore() override;
			void SetFullscreen(u32 width, u32 height, float refreshRate = 60.0f, u32 monitorIdx = 0) override;
			void SetFullscreen(const VideoMode& videoMode) override;
			void SetWindowed(u32 width, u32 height) override;
			void SetVSync(bool enabled, u32 interval = 1) override;

			/** Prepares the back buffer for rendering. Should be called before it is bound to the GPU. */
			void AcquireBackBuffer();
			void SwapBuffers(u32 syncMask = 0xFFFFFFFF) override;
			void GetCustomAttribute(const String& name, void* data) const override;
			void WindowMovedOrResizedInternal() override;

			/**	Returns internal window handle. */
			HWND GetWindowHandleInternal() const;

		protected:
			friend class bs::Win32RenderWindow;

			void Initialize() override;
			const RenderTargetProperties& GetPropertiesInternal() const override { return mProperties; }
			RenderWindowProperties& GetSyncedProperties() override { return mSyncedProperties; }
			void SyncProperties() override;

			/** Rebuilds the swap chain according to the currently set properties. */
			void RebuildSwapChain();

		protected:
			Win32Window* mWindow;
			bool mIsChild;
			bool mShowOnSwap;
			i32 mDisplayFrequency;

			VulkanRenderAPI& mRenderAPI;
			VkSurfaceKHR mSurface;
			VkColorSpaceKHR mColorSpace;
			VkFormat mColorFormat;
			VkFormat mDepthFormat;
			u32 mPresentQueueFamily;
			VulkanSwapChain* mSwapChain = nullptr;
			VulkanSemaphore* mSemaphoresTemp[BS_MAX_UNIQUE_QUEUES + 1]; // +1 for present semaphore
			bool mRequiresNewBackBuffer;

			RenderWindowProperties mProperties;
			RenderWindowProperties mSyncedProperties;
		};
	} // namespace ct

	/** @} */
} // namespace bs

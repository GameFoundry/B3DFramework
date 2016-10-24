//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsRenderWindow.h"

namespace BansheeEngine
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	class Win32RenderWindow;

	/**	Contains various properties that describe a render window. */
	class Win32RenderWindowProperties : public RenderWindowProperties
	{
	public:
		Win32RenderWindowProperties(const RENDER_WINDOW_DESC& desc);
		virtual ~Win32RenderWindowProperties() { }

	private:
		friend class Win32RenderWindowCore;
		friend class Win32RenderWindow;
	};

	/**
	 * Render window implementation for Windows and Vulkan.
	 *
	 * @note	Core thread only.
	 */
	class Win32RenderWindowCore : public RenderWindowCore
	{
	public:
		Win32RenderWindowCore(const RENDER_WINDOW_DESC& desc, UINT32 windowId, VulkanRenderAPI& renderAPI);
		~Win32RenderWindowCore();

		/** @copydoc RenderWindowCore::move */
		void move(INT32 left, INT32 top) override;

		/** @copydoc RenderWindowCore::resize */
		void resize(UINT32 width, UINT32 height) override;

		/** @copydoc RenderWindowCore::setHidden */
		void setHidden(bool hidden) override;

		/** @copydoc RenderWindowCore::setActive */
		void setActive(bool state) override;

		/** @copydoc RenderWindowCore::minimize */
		void minimize() override;

		/** @copydoc RenderWindowCore::maximize */
		void maximize() override;

		/** @copydoc RenderWindowCore::restore */
		void restore() override;

		/** @copydoc RenderWindowCore::setFullscreen(UINT32, UINT32, float, UINT32) */
		void setFullscreen(UINT32 width, UINT32 height, float refreshRate = 60.0f, UINT32 monitorIdx = 0) override;

		/** @copydoc RenderWindowCore::setFullscreen(const VideoMode&) */
		void setFullscreen(const VideoMode& videoMode) override;

		/** @copydoc RenderWindowCore::setWindowed */
		void setWindowed(UINT32 width, UINT32 height) override;

		/** 
		 * Copies the contents of a frame buffer into the pre-allocated buffer. 
		 *
		 * @param[out]	dst		Previously allocated buffer to read the contents into. Must be of valid size.
		 * @param[in]	buffer	Frame buffer to read the contents from.
		 */
		void copyToMemory(PixelData &dst, FrameBuffer buffer);

		/** @copydoc RenderWindowCore::swapBuffers */
		void swapBuffers() override;

		/** @copydoc RenderWindowCore::getCustomAttribute */
		void getCustomAttribute(const String& name, void* pData) const override;

		/** @copydoc RenderWindowCore::_windowMovedOrResized */
		void _windowMovedOrResized() override;

		/**	Returns internal window handle. */
		HWND _getWindowHandle() const;
	protected:
		friend class Win32RenderWindow;

		/** @copydoc CoreObjectCore::initialize */
		void initialize() override;

		/** @copydoc RenderWindowCore::getProperties */
		const RenderTargetProperties& getPropertiesInternal() const override { return mProperties; }

		/** @copydoc RenderWindowCore::getSyncedProperties */
		RenderWindowProperties& getSyncedProperties() override { return mSyncedProperties; }

		/** @copydoc RenderWindowCore::syncProperties */
		void syncProperties() override;

	protected:
		Win32Window* mWindow;
		bool mIsChild;
		bool mShowOnSwap;
		INT32 mDisplayFrequency;

		VulkanRenderAPI& mRenderAPI;
		VkSurfaceKHR mSurface;
		VkColorSpaceKHR mColorSpace;
		VkFormat mColorFormat;
		SPtr<VulkanSwapChain> mSwapChain;

		Win32RenderWindowProperties mProperties;
		Win32RenderWindowProperties mSyncedProperties;
	};

	/**
	 * Render window implementation for Windows and Vulkan.
	 *
	 * @note	Sim thread only.
	 */
	class Win32RenderWindow : public RenderWindow
	{
	public:
		~Win32RenderWindow() { }

		/** @copydoc RenderWindow::screenToWindowPos */
		void getCustomAttribute(const String& name, void* pData) const override;

		/** @copydoc RenderWindow::screenToWindowPos */
		Vector2I screenToWindowPos(const Vector2I& screenPos) const override;

		/** @copydoc RenderWindow::windowToScreenPos */
		Vector2I windowToScreenPos(const Vector2I& windowPos) const override;

		/** @copydoc RenderWindow::getCore */
		SPtr<Win32RenderWindowCore> getCore() const;

		/**	Retrieves internal window handle. */
		HWND getHWnd() const;

	protected:
		friend class VulkanRenderWindowManager;
		friend class Win32RenderWindowCore;

		Win32RenderWindow(const RENDER_WINDOW_DESC& desc, UINT32 windowId);

		/** @copydoc RenderWindowCore::getProperties */
		const RenderTargetProperties& getPropertiesInternal() const override { return mProperties; }

		/** @copydoc RenderWindow::syncProperties */
		void syncProperties() override;

	private:
		Win32RenderWindowProperties mProperties;
	};
	
	/** @} */
}
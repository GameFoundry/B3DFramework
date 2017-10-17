//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "Win32/BsWin32Prerequisites.h"
#include "Renderapi/BsRenderWindow.h"

namespace bs
{
	/** @addtogroup GL
	 *  @{
	 */

	/**
	 * Render window implementation for Windows.
	 *
	 * @note	Sim thread only.
	 */
	class Win32RenderWindow : public RenderWindow
	{
	public:
		~Win32RenderWindow();

		/** @copydoc RenderWindow::screenToWindowPos */
		void getCustomAttribute(const String& name, void* pData) const override;

		/** @copydoc RenderWindow::screenToWindowPos */
		Vector2I screenToWindowPos(const Vector2I& screenPos) const override;

		/** @copydoc RenderWindow::windowToScreenPos */
		Vector2I windowToScreenPos(const Vector2I& windowPos) const override;

		/** @copydoc RenderWindow::getCore */
		SPtr<ct::Win32RenderWindow> getCore() const;

	protected:
		friend class GLRenderWindowManager;
		friend class ct::Win32GLSupport;
		friend class ct::Win32RenderWindow;

		Win32RenderWindow(const RENDER_WINDOW_DESC& desc, UINT32 windowId, ct::Win32GLSupport& glsupport);

		/** @copydoc RenderWindow::getProperties */
		const RenderTargetProperties& getPropertiesInternal() const override { return mProperties; }

		/** @copydoc RenderWindow::syncProperties */
		void syncProperties() override;

		/**	Retrieves internal window handle. */
		HWND getHWnd() const;

	private:
		ct::Win32GLSupport& mGLSupport;
		RenderWindowProperties mProperties;
	};

	namespace ct
	{
	/**
	 * Render window implementation for Windows.
	 *
	 * @note	Core thread only.
	 */
	class Win32RenderWindow : public RenderWindow
	{
	public:
		Win32RenderWindow(const RENDER_WINDOW_DESC& desc, UINT32 windowId, Win32GLSupport &glsupport);
		~Win32RenderWindow();

		/** @copydoc RenderWindow::setFullscreen(UINT32, UINT32, float, UINT32) */
		void setFullscreen(UINT32 width, UINT32 height, float refreshRate = 60.0f, UINT32 monitorIdx = 0) override;

		/** @copydoc RenderWindow::setFullscreen(const VideoMode&) */
		void setFullscreen(const VideoMode& videoMode) override;

		/** @copydoc RenderWindow::setWindowed */
		void setWindowed(UINT32 width, UINT32 height) override;

		/** @copydoc RenderWindow::setHidden */
		void setHidden(bool hidden) override;

		/** @copydoc RenderWindow::minimize */
		void minimize() override;

		/** @copydoc RenderWindow::maximize */
		void maximize() override;

		/** @copydoc RenderWindow::restore */
		void restore() override;

		/** @copydoc RenderWindow::move */
		void move(INT32 left, INT32 top) override;

		/** @copydoc RenderWindow::resize */
		void resize(UINT32 width, UINT32 height) override;

		/** 
		 * Copies the contents of a frame buffer into the pre-allocated buffer. 
		 *
		 * @param[out]	dst		Previously allocated buffer to read the contents into. Must be of valid size.
		 * @param[in]	buffer	Frame buffer to read the contents from.
		 */
		void copyToMemory(PixelData& dst, FrameBuffer buffer);

		/** @copydoc RenderWindow::swapBuffers */
		void swapBuffers(UINT32 syncMask) override;

		/** @copydoc RenderWindow::getCustomAttribute */
		void getCustomAttribute(const String& name, void* pData) const override;

		/** @copydoc RenderWindow::setActive */
		void setActive(bool state) override;

		/** @copydoc RenderWindow::_windowMovedOrResized */
		void _windowMovedOrResized() override;

		/**	Returns handle to device context associated with the window. */
		HDC _getHDC() const { return mHDC; }

		/**	Returns internal window handle. */
		HWND _getHWnd() const;

	protected:
		friend class Win32GLSupport;

		/** @copydoc CoreObject::initialize */
		void initialize() override;

		/** @copydoc RenderWindow::getProperties */
		const RenderTargetProperties& getPropertiesInternal() const override { return mProperties; }

		/** @copydoc RenderWindow::getSyncedProperties */
		RenderWindowProperties& getSyncedProperties() override { return mSyncedProperties; }

		/** @copydoc RenderWindow::syncProperties */
		void syncProperties() override;

	protected:
		friend class bs::Win32RenderWindow;

		Win32Window* mWindow;
		Win32GLSupport& mGLSupport;
		HDC	mHDC;
		bool mIsChild;
		char* mDeviceName;
		int mDisplayFrequency;
		bool mShowOnSwap;
		SPtr<Win32Context> mContext;
		RenderWindowProperties mProperties;
		RenderWindowProperties mSyncedProperties;
	};
	}

	/** @} */
}
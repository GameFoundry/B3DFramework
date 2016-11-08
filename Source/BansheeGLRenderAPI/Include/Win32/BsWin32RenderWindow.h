//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsWin32Prerequisites.h"
#include "BsRenderWindow.h"

namespace BansheeEngine 
{
	/** @addtogroup GL
	 *  @{
	 */

	class Win32RenderWindow;

	/**	Contains various properties that describe a render window. */
	class BS_RSGL_EXPORT Win32RenderWindowProperties : public RenderWindowProperties
	{
	public:
		Win32RenderWindowProperties(const RENDER_WINDOW_DESC& desc);
		virtual ~Win32RenderWindowProperties() { }

	private:
		friend class Win32RenderWindowCore;
		friend class Win32RenderWindow;
	};

	/**
	 * Render window implementation for Windows.
	 *
	 * @note	Core thread only.
	 */
    class BS_RSGL_EXPORT Win32RenderWindowCore : public RenderWindowCore
    {
    public:
		Win32RenderWindowCore(const RENDER_WINDOW_DESC& desc, UINT32 windowId, Win32GLSupport &glsupport);
		~Win32RenderWindowCore();

		/** @copydoc RenderWindowCore::setFullscreen(UINT32, UINT32, float, UINT32) */
		void setFullscreen(UINT32 width, UINT32 height, float refreshRate = 60.0f, UINT32 monitorIdx = 0) override;

		/** @copydoc RenderWindowCore::setFullscreen(const VideoMode&) */
		void setFullscreen(const VideoMode& videoMode) override;

		/** @copydoc RenderWindowCore::setWindowed */
		void setWindowed(UINT32 width, UINT32 height) override;

		/** @copydoc RenderWindowCore::setHidden */
		void setHidden(bool hidden) override;

		/** @copydoc RenderWindowCore::minimize */
		void minimize() override;

		/** @copydoc RenderWindowCore::maximize */
		void maximize() override;

		/** @copydoc RenderWindowCore::restore */
		void restore() override;

		/** @copydoc RenderWindowCore::move */
		void move(INT32 left, INT32 top) override;

		/** @copydoc RenderWindowCore::resize */
		void resize(UINT32 width, UINT32 height) override;

		/** 
		 * Copies the contents of a frame buffer into the pre-allocated buffer. 
		 *
		 * @param[out]	dst		Previously allocated buffer to read the contents into. Must be of valid size.
		 * @param[in]	buffer	Frame buffer to read the contents from.
		 */
		void copyToMemory(PixelData& dst, FrameBuffer buffer);

		/** @copydoc RenderWindowCore::swapBuffers */
		void swapBuffers(UINT32 syncMask) override;

		/** @copydoc RenderWindowCore::getCustomAttribute */
		void getCustomAttribute(const String& name, void* pData) const override;

		/** @copydoc RenderWindowCore::setActive */
		void setActive(bool state) override;

		/** @copydoc RenderWindowCore::_windowMovedOrResized */
		void _windowMovedOrResized() override;

		/**	Returns handle to device context associated with the window. */
		HDC _getHDC() const { return mHDC; }

		/**	Returns internal window handle. */
		HWND _getHWnd() const;

	protected:
		friend class Win32GLSupport;

		/** @copydoc CoreObjectCore::initialize */
		void initialize() override;

		/** @copydoc RenderWindowCore::getProperties */
		const RenderTargetProperties& getPropertiesInternal() const override { return mProperties; }

		/** @copydoc RenderWindowCore::getSyncedProperties */
		RenderWindowProperties& getSyncedProperties() override { return mSyncedProperties; }

		/** @copydoc RenderWindowCore::syncProperties */
		void syncProperties() override;

	protected:
		friend class Win32RenderWindow;

		Win32Window* mWindow;
		Win32GLSupport& mGLSupport;
		HDC	mHDC;
		bool mIsChild;
		char* mDeviceName;
		int mDisplayFrequency;
		bool mShowOnSwap;
		SPtr<Win32Context> mContext;
		Win32RenderWindowProperties mProperties;
		Win32RenderWindowProperties mSyncedProperties;
    };

	/**
	 * Render window implementation for Windows.
	 *
	 * @note	Sim thread only.
	 */
	class BS_RSGL_EXPORT Win32RenderWindow : public RenderWindow
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

	protected:
		friend class GLRenderWindowManager;
		friend class Win32GLSupport;
		friend class Win32RenderWindowCore;

		Win32RenderWindow(const RENDER_WINDOW_DESC& desc, UINT32 windowId, Win32GLSupport& glsupport);

		/** @copydoc RenderWindow::getProperties */
		const RenderTargetProperties& getPropertiesInternal() const override { return mProperties; }

		/** @copydoc RenderWindow::syncProperties */
		void syncProperties() override;

		/**	Retrieves internal window handle. */
		HWND getHWnd() const;

	private:
		Win32GLSupport& mGLSupport;
		Win32RenderWindowProperties mProperties;
	};

	/** @} */
}
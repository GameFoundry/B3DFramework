//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
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
		void GetCustomAttribute(const String& name, void* pData) const;

		/** @copydoc RenderWindow::screenToWindowPos */
		Vector2I ScreenToWindowPos(const Vector2I& screenPos) const;

		/** @copydoc RenderWindow::windowToScreenPos */
		Vector2I WindowToScreenPos(const Vector2I& windowPos) const;

		/** @copydoc RenderWindow::getCore */
		SPtr<ct::Win32RenderWindow> GetCore() const;

	protected:
		friend class GLRenderWindowManager;
		friend class ct::Win32GLSupport;
		friend class ct::Win32RenderWindow;

		Win32RenderWindow(const RENDER_WINDOW_DESC& desc, u32 windowId, ct::Win32GLSupport& glsupport);

		/** @copydoc RenderWindow::getProperties */
		const RenderTargetProperties& GetPropertiesInternal() const { return mProperties; }

		/** @copydoc RenderWindow::syncProperties */
		void SyncProperties();

		/** @copydoc RenderWindow::createCore */
		SPtr<ct::CoreObject> CreateCore() const;

		/**	Retrieves internal window handle. */
		HWND GetHWnd() const;

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
			Win32RenderWindow(const RENDER_WINDOW_DESC& desc, u32 windowId, Win32GLSupport& glsupport);
			~Win32RenderWindow();

			/** @copydoc RenderWindow::setFullscreen(u32, u32, float, u32) */
			void SetFullscreen(u32 width, u32 height, float refreshRate = 60.0f, u32 monitorIdx = 0);

			/** @copydoc RenderWindow::setFullscreen(const VideoMode&) */
			void SetFullscreen(const VideoMode& videoMode);

			/** @copydoc RenderWindow::setWindowed */
			void SetWindowed(u32 width, u32 height);

			/** @copydoc RenderWindow::setHidden */
			void SetHidden(bool hidden);

			/** @copydoc RenderWindow::minimize */
			void Minimize();

			/** @copydoc RenderWindow::maximize */
			void Maximize();

			/** @copydoc RenderWindow::restore */
			void Restore();

			/** @copydoc RenderWindow::move */
			void Move(i32 left, i32 top);

			/** @copydoc RenderWindow::resize */
			void Resize(u32 width, u32 height) override;

			/** @copydoc RenderWindow::setVSync */
			void SetVSync(bool enabled, u32 interval = 1) override;

			/**
			 * Copies the contents of a frame buffer into the pre-allocated buffer.
			 *
			 * @param[out]	dst		Previously allocated buffer to read the contents into. Must be of valid size.
			 * @param[in]	buffer	Frame buffer to read the contents from.
			 */
			void CopyToMemory(PixelData& dst, FrameBuffer buffer);

			/** @copydoc RenderWindow::swapBuffers */
			void SwapBuffers(u32 syncMask) override;

			/** @copydoc RenderWindow::getCustomAttribute */
			void GetCustomAttribute(const String& name, void* pData) const override;

			/** @copydoc RenderWindow::setActive */
			void SetActive(bool state) override;

			/** @copydoc RenderWindow::_windowMovedOrResized */
			void WindowMovedOrResizedInternal() override;

			/**	Returns handle to device context associated with the window. */
			HDC GetHDCInternal() const { return mHDC; }

			/**	Returns internal window handle. */
			HWND GetHWndInternal() const;

		protected:
			friend class Win32GLSupport;

			/** @copydoc CoreObject::initialize */
			void Initialize() override;

			/** @copydoc RenderWindow::getProperties */
			const RenderTargetProperties& GetPropertiesInternal() const override { return mProperties; }

			/** @copydoc RenderWindow::getSyncedProperties */
			RenderWindowProperties& GetSyncedProperties() override { return mSyncedProperties; }

			/** @copydoc RenderWindow::syncProperties */
			void SyncProperties() override;

		protected:
			friend class bs::Win32RenderWindow;

			Win32Window* mWindow;
			Win32GLSupport& mGLSupport;
			HDC mHDC;
			bool mIsChild;
			char* mDeviceName;
			int mDisplayFrequency;
			bool mShowOnSwap;
			SPtr<Win32Context> mContext;
			RenderWindowProperties mProperties;
			RenderWindowProperties mSyncedProperties;
		};
	} // namespace ct

	/** @} */
} // namespace bs

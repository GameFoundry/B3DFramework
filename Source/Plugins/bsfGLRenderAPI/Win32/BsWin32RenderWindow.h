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

		void GetCustomAttribute(const String& name, void* pData) const override;
		Vector2I ScreenToWindowPos(const Vector2I& screenPos) const override;
		Vector2I WindowToScreenPos(const Vector2I& windowPos) const override;

		/** @copydoc RenderWindow::GetCore */
		SPtr<ct::Win32RenderWindow> GetCore() const;

	protected:
		friend class GLRenderWindowManager;
		friend class ct::Win32GLSupport;
		friend class ct::Win32RenderWindow;

		Win32RenderWindow(const RENDER_WINDOW_DESC& desc, u32 windowId, ct::Win32GLSupport& glsupport);

		const RenderTargetProperties& GetPropertiesInternal() const override { return mProperties; }
		void SyncProperties() override;
		SPtr<ct::CoreObject> CreateCore() const override;

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

			void SetFullscreen(u32 width, u32 height, float refreshRate = 60.0f, u32 monitorIdx = 0) override;
			void SetFullscreen(const VideoMode& videoMode) override;
			void SetWindowed(u32 width, u32 height) override;
			void SetHidden(bool hidden) override;
			void Minimize() override;
			void Maximize() override;
			void Restore() override;
			void Move(i32 left, i32 top) override;
			void Resize(u32 width, u32 height) override;
			void SetVSync(bool enabled, u32 interval = 1) override;

			/**
			 * Copies the contents of a frame buffer into the pre-allocated buffer.
			 *
			 * @param[out]	dst		Previously allocated buffer to read the contents into. Must be of valid size.
			 * @param[in]	buffer	Frame buffer to read the contents from.
			 */
			void CopyToMemory(PixelData& dst, FrameBuffer buffer);
			void SwapBuffers(u32 syncMask) override;
			void GetCustomAttribute(const String& name, void* pData) const override;
			void SetActive(bool state) override;
			void WindowMovedOrResizedInternal() override;

			/**	Returns handle to device context associated with the window. */
			HDC GetHDCInternal() const { return mHDC; }

			/**	Returns internal window handle. */
			HWND GetHWndInternal() const;

		protected:
			friend class Win32GLSupport;

			void Initialize() override;
			const RenderTargetProperties& GetPropertiesInternal() const override { return mProperties; }
			RenderWindowProperties& GetSyncedProperties() override { return mSyncedProperties; }
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

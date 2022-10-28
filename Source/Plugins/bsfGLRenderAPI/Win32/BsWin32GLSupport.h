//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Win32/BsWin32Prerequisites.h"
#include "BsGLSupport.h"
#include "BsGLRenderAPI.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup GL
		 *  @{
		 */

		/**	Handles OpenGL initialization, window creation and extensions on Windows. */
		class Win32GLSupport : public GLSupport
		{
		public:
			Win32GLSupport();

			SPtr<bs::RenderWindow> NewWindow(RENDER_WINDOW_DESC& desc, u32 windowId, SPtr<bs::RenderWindow> parentWindow) override;
			void Start() override;
			void Stop() override;
			void* GetProcAddress(const String& procname) override;
			void InitializeExtensions() override;

			/**
			 * Creates a new OpenGL context.
			 *
			 * @param[in]	hdc				Handle to device context to create the context from.
			 * @param[in]	externalGlrc	(Optional) Handle to external OpenGL context. If not provided new context will be
			 *								created.
			 * @return						Newly created GLContext class referencing the created or external context handle.
			 */
			SPtr<Win32Context> CreateContext(HDC hdc, HGLRC externalGlrc = 0);

			/**
			 * Selects and sets an appropriate pixel format based on the provided parameters.
			 *
			 * @param[in]	hdc			Handle to device context to create the context from.
			 * @param[in]	colorDepth	Wanted color depth of the pixel format, in bits.
			 * @param[in]	multisample	Amount of multisampling wanted, if any.
			 * @param[in]	hwGamma		Should the format support automatic gamma conversion on write/read.
			 * @param[in]	depth		Should the pixel format contain the depth/stencil buffer.
			 * @return					True if a pixel format was successfully set.
			 */
			bool SelectPixelFormat(HDC hdc, int colorDepth, int multisample, bool hwGamma, bool depth);

			SPtr<VideoModeInfo> GetVideoModeInfo() const override;

			/** Notifies the manager that a new window has been created. */
			void NotifyWindowCreatedInternal(Win32RenderWindow* window);

		private:
			/**	Initializes windows specific OpenGL extensions needed for advanced context creation. */
			void InitialiseWgl();

			/**	Dummy window procedure used when creating the initial dummy OpenGL context. */
			static LRESULT CALLBACK DummyWndProc(HWND hwnd, UINT umsg, WPARAM wp, LPARAM lp);

			Vector<DEVMODE> mDevModes;
			Win32RenderWindow* mInitialWindow = nullptr;
			Vector<int> mMultisampleLevels;
			bool mHasPixelFormatARB = false;
			bool mHasMultisample = false;
			bool mHasHardwareGamma = false;
			bool mHasAdvancedContext = false;
		};

		/** @} */
	} // namespace ct
} // namespace bs

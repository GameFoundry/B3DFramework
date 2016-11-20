//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsWin32Prerequisites.h"
#include "BsGLSupport.h"
#include "BsGLRenderAPI.h"

namespace bs
{
	/** @addtogroup GL
	 *  @{
	 */

	/**	Handles OpenGL initialization, window creation and extensions on Windows. */
	class BS_RSGL_EXPORT Win32GLSupport : public GLSupport
	{
	public:
        Win32GLSupport();

		/** @copydoc GLSupport::newWindow */
		SPtr<RenderWindow> newWindow(RENDER_WINDOW_DESC& desc, UINT32 windowId, SPtr<RenderWindow> parentWindow) override;

		/** @copydoc GLSupport::newWindowCore */
		SPtr<RenderWindowCore> newWindowCore(RENDER_WINDOW_DESC& desc, UINT32 windowId) override;

		/** @copydoc GLSupport::start */
		void start() override;

		/** @copydoc GLSupport::stop */
		void stop() override;

		/** @copydoc GLSupport::getProcAddress */
		void* getProcAddress(const String& procname) override;

		/** @copydoc GLSupport::initializeExtensions */
		void initializeExtensions() override;
		
		/**
		 * Creates a new OpenGL context.
		 *
		 * @param[in]	hdc				Handle to device context to create the context from.
		 * @param[in]	externalGlrc	(Optional) Handle to external OpenGL context. If not provided new context will be
		 *								created.
		 * @return						Newly created GLContext class referencing the created or external context handle.
		 */
		SPtr<Win32Context> createContext(HDC hdc, HGLRC externalGlrc = 0);

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
		bool selectPixelFormat(HDC hdc, int colorDepth, int multisample, bool hwGamma, bool depth);

		/** @copydoc GLSupport::getVideoModeInfo */
		SPtr<VideoModeInfo> getVideoModeInfo() const override;

	private:
		/**	Initializes windows specific OpenGL extensions needed for advanced context creation. */
		void initialiseWGL();

		/**	Dummy window procedure used when creating the initial dummy OpenGL context. */
		static LRESULT CALLBACK dummyWndProc(HWND hwnd, UINT umsg, WPARAM wp, LPARAM lp);

		Vector<DEVMODE> mDevModes;
		Win32RenderWindowCore *mInitialWindow;
		Vector<int> mMultisampleLevels;
		bool mHasPixelFormatARB;
        bool mHasMultisample;
		bool mHasHardwareGamma;
		bool mHasAdvancedContext;
	};

	/** @} */
}
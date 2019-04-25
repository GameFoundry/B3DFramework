//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Win32/BsWin32Prerequisites.h"
#include "BsGLSupport.h"
#include "BsGLRenderAPI.h"

namespace bs { namespace ct
{
	/** @addtogroup GL
	 *  @{
	 */

	/**	Handles OpenGL initialization, window creation and extensions on Windows. */
	class Win32GLSupport : public GLSupport
	{
	public:
		Win32GLSupport();

		/** @copydoc GLSupport::newWindow */
		SPtr<bs::RenderWindow> newWindow(RENDER_WINDOW_DESC& desc, UINT32 windowId, SPtr<bs::RenderWindow> parentWindow) override;

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

		/** Notifies the manager that a new window has been created. */
		void _notifyWindowCreated(Win32RenderWindow* window);

	private:
		/**	Initializes windows specific OpenGL extensions needed for advanced context creation. */
		void initialiseWGL();

		/**	Dummy window procedure used when creating the initial dummy OpenGL context. */
		static LRESULT CALLBACK dummyWndProc(HWND hwnd, UINT umsg, WPARAM wp, LPARAM lp);

		Vector<DEVMODE>    mDevModes;
		Win32RenderWindow* mInitialWindow = nullptr;
		Vector<int>        mMultisampleLevels;
		bool               mHasPixelFormatARB = false;
		bool               mHasMultisample = false;
		bool               mHasHardwareGamma = false;
		bool               mHasAdvancedContext = false;
	};

	/** @} */
}}
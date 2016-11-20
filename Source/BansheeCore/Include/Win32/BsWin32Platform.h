//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPlatform.h"
#include "BsWin32Defs.h"

namespace bs
{
	/** @addtogroup Platform-Internal
	 *  @{
	 */

	/**
	 * Contains the main message loop.
	 *
	 * @note	
	 * This is separated from the main Platform because we don't want to include various Windows defines in a lot of our 
	 * code that includes "Platform.h".
	 */
	class BS_CORE_EXPORT Win32Platform : public Platform
	{
	public:
		/** Main message loop callback that processes messages received from windows. */
		static LRESULT CALLBACK _win32WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	};

	/** @} */
}
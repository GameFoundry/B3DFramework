//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#if BS_PLATFORM == BS_PLATFORM_WIN32

#include "Win32/BsWin32GLSupport.h"
#include "Win32/BsWin32VideoModeInfo.h"

namespace bs 
{
	/** @addtogroup GL
	 *  @{
	 */

	/**	Helper method that returns a platform specific GL support object. */
	GLSupport* getGLSupport()
	{
		return bs_new<Win32GLSupport>();
	}

	/** @} */
};

#endif
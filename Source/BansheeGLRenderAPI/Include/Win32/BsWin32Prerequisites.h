//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include <windows.h>
#include <BsGLPrerequisites.h>

namespace bs
{
	/** @addtogroup GL
	 *  @{
	 */

    class Win32GLSupport;
    class Win32RenderWindow;
    class Win32Context;
	class Win32RenderWindowCore;

	/**	Retrieves last Windows API error and returns a description of it. */
	String translateWGLError();

	/** @} */
}
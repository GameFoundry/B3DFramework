//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include <windows.h>
#include <BsGLPrerequisites.h>

namespace bs { namespace ct
{
	/** @addtogroup GL
	 *  @{
	 */

    class Win32GLSupport;
    class Win32Context;
	class Win32RenderWindow;

	/**	Retrieves last Windows API error and returns a description of it. */
	String translateWGLError();

	/** @} */
}}
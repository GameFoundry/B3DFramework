//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include <windows.h>
#include <BsGLPrerequisites.h>

namespace bs
{
	namespace ct
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
	} // namespace ct
} // namespace bs

//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#if BS_PLATFORM == BS_PLATFORM_WIN32

#	include "Win32/BsWin32GLSupport.h"
#	include "Win32/BsWin32VideoModeInfo.h"

namespace bs
{
	namespace ct
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
	} // namespace ct
} // namespace bs

#elif BS_PLATFORM == BS_PLATFORM_LINUX

#	include "Linux/BsLinuxGLSupport.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup GL
		 *  @{
		 */

		/**	Helper method that returns a platform specific GL support object. */
		GLSupport* getGLSupport()
		{
			return bs_new<LinuxGLSupport>();
		}

		/** @} */
	} // namespace ct
} // namespace bs

#elif BS_PLATFORM == BS_PLATFORM_OSX

#	include "MacOS/BsMacOSGLSupport.h"

namespace bs::ct
{
	/** @addtogroup GL
	 *  @{
	 */

	/**	Helper method that returns a platform specific GL support object. */
	GLSupport* getGLSupport()
	{
		return bs_new<MacOSGLSupport>();
	}

	/** @} */
} // namespace bs::ct
#endif

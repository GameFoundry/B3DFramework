//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once
#include "BsPrerequisitesUtil.h"
#include <windows.h>

namespace bs
{
	/** @addtogroup Internal-Utility
	 *  @{
	 */

	/** @addtogroup Platform-Utility-Internal
	 *  @{
	 */

	/** Provides access to various Windows specific utility functions. */
	class BS_UTILITY_EXPORT Win32PlatformUtility
	{
	public:
		/**
		 * Creates a new bitmap usable by various Win32 methods from the provided pixel data. Caller must ensure to call 
		 * DeleteObject() on the bitmap handle when finished.
		 */
		static HBITMAP createBitmap(const Color* pixels, UINT32 width, UINT32 height, bool premultiplyAlpha);
	};

	/** @} */
	/** @} */
}
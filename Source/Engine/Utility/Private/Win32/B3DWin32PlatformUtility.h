//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once
#include "B3DUtilityPrerequisites.h"
#include <windows.h>

namespace b3d
{
	/** @addtogroup Internal-Utility
	 *  @{
	 */

	/** @addtogroup Platform-Utility-Internal
	 *  @{
	 */

	/** Provides access to various Windows specific utility functions. */
	class B3D_EXPORT Win32PlatformUtility
	{
	public:
		/**
		 * Creates a new bitmap usable by various Win32 methods from the provided pixel data. Caller must ensure to call
		 * DeleteObject() on the bitmap handle when finished.
		 */
		static HBITMAP CreateBitmap(const Color* pixels, u32 width, u32 height, bool premultiplyAlpha);
	};

	/** @} */
	/** @} */
} // namespace b3d

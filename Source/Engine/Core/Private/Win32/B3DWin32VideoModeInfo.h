//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "RenderAPI/B3DVideoModeInfo.h"

#define WIN32_LEAN_AND_MEAN
#if !defined(NOMINMAX) && defined(_MSC_VER)
#	define NOMINMAX // Required to stop windows.h messing up std::min
#endif

#include "windows.h"

namespace b3d::render
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/** @copydoc VideoMode */
	class B3D_EXPORT Win32VideoMode : public VideoMode
	{
	public:
		Win32VideoMode(u32 width, u32 height, float refreshRate, u32 outputIdx);

	private:
		friend class Win32VideoOutputInfo;
	};

	/** @copydoc VideoOutputInfo */
	class B3D_EXPORT Win32VideoOutputInfo : public VideoOutputInfo
	{
	public:
		Win32VideoOutputInfo(HMONITOR monitorHandle, u32 outputIdx);

		/**	Gets a Win32 handle to the monitor referenced by this object. */
		HMONITOR GetMonitorHandle() const { return mMonitorHandle; }

	private:
		HMONITOR mMonitorHandle;
	};

	/** @copydoc VideoModeInfo */
	class B3D_EXPORT Win32VideoModeInfo : public VideoModeInfo
	{
	public:
		Win32VideoModeInfo();
	};

	/** @} */
} // namespace b3d::render

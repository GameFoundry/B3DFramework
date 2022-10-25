//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsGLPrerequisites.h"
#include "RenderAPI/BsVideoModeInfo.h"
#include <X11/extensions/Xrandr.h>

namespace bs::ct
{
	/** @addtogroup GL
	 *  @{
	 */

	/** @copydoc VideoMode */
	class LinuxVideoMode : public VideoMode
	{
	public:
		LinuxVideoMode(u32 width, u32 height, float refreshRate, u32 outputIdx);

		/** Returns internal RandR video mode id. */
		RRMode GetModeIDInternal() const { return mModeID; }

	private:
		LinuxVideoMode(u32 width, u32 height, float refreshRate, u32 outputIdx, RRMode modeID);
		friend class LinuxVideoOutputInfo;

		RRMode mModeID;
	};

	/** @copydoc VideoOutputInfo */
	class LinuxVideoOutputInfo : public VideoOutputInfo
	{
	public:
		LinuxVideoOutputInfo(::Display* x11Display, i32 screen, XRROutputInfo* outputInfo, XRRCrtcInfo* crtcInfo, XRRScreenResources* screenRes, RROutput outputID, u32 outputIdx);

		/** Returns internal RandR output device id. */
		RROutput GetOutputIDInternal() const { return mOutputID; }

		/** Returns X11 screen this output renders to. One screen can contain multiple output devices. */
		i32 GetScreenInternal() const { return mScreen; }

	private:
		RROutput mOutputID;
		i32 mScreen;
	};

	/** @copydoc VideoModeInfo */
	class LinuxVideoModeInfo : public VideoModeInfo
	{
	public:
		LinuxVideoModeInfo();
	};

	/** @} */
} // namespace bs::ct

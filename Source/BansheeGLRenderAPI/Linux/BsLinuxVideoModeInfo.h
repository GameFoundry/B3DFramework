//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsGLPrerequisites.h"
#include "RenderAPI/BsVideoModeInfo.h"
#include <X11/extensions/Xrandr.h>

namespace bs { namespace ct
{
	/** @addtogroup GL
	 *  @{
	 */

	/** @copydoc VideoMode */
	class LinuxVideoMode : public VideoMode
	{
	public:
		LinuxVideoMode(UINT32 width, UINT32 height, float refreshRate, UINT32 outputIdx);

		/** Returns internal RandR video mode id. */
		RRMode _getModeID() const { return mModeID; }

	private:
		LinuxVideoMode(UINT32 width, UINT32 height, float refreshRate, UINT32 outputIdx, RRMode modeID);
		friend class LinuxVideoOutputInfo;

		RRMode mModeID;
	};

	/** @copydoc VideoOutputInfo */
	class LinuxVideoOutputInfo : public VideoOutputInfo
	{
	public:
		LinuxVideoOutputInfo(::Display* x11Display, INT32 screen, XRROutputInfo* outputInfo, XRRCrtcInfo* crtcInfo,
			 XRRScreenResources* screenRes, RROutput outputID, UINT32 outputIdx);

		/** Returns internal RandR output device id. */
		RROutput _getOutputID() const { return mOutputID; }

		/** Returns X11 screen this output renders to. One screen can contain multiple output devices. */
		INT32 _getScreen() const { return mScreen;}
	private:
		RROutput mOutputID;
		INT32 mScreen;
	};

	/** @copydoc VideoModeInfo */
	class LinuxVideoModeInfo : public VideoModeInfo
	{
	public:
		LinuxVideoModeInfo();
	};

	/** @} */
}}


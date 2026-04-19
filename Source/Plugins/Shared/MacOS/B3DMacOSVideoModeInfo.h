//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "RenderAPI/B3DVideoModeInfo.h"
#include <CoreGraphics/CoreGraphics.h>
#include <CoreVideo/CoreVideo.h>

namespace b3d
{
	namespace render
	{
		/** @addtogroup GL
		 *  @{
		 */

		/** @copydoc VideoMode */
		class MacOSVideoMode : public VideoMode
		{
		public:
			MacOSVideoMode(u32 width, u32 height, float refreshRate, u32 outputIdx);
			~MacOSVideoMode() override;

			/** Returns internal Core Graphics video mode reference. */
			CGDisplayModeRef GetModeRefInternal() const { return mModeRef; }

		private:
			MacOSVideoMode(CGDisplayModeRef modeRef, CVDisplayLinkRef linkRef, u32 outputIdx);
			friend class MacOSVideoOutputInfo;

			CGDisplayModeRef mModeRef;
		};

		/** @copydoc VideoOutputInfo */
		class MacOSVideoOutputInfo : public VideoOutputInfo
		{
		public:
			MacOSVideoOutputInfo(CGDirectDisplayID displayID, u32 outputIdx);

			/** Returns the Core Graphics identifier for this display. */
			CGDirectDisplayID GetDisplayIDInternal() const { return mDisplayID; }

		private:
			CGDirectDisplayID mDisplayID;
		};

		/** @copydoc VideoModeInfo */
		class MacOSVideoModeInfo : public VideoModeInfo
		{
		public:
			MacOSVideoModeInfo();
		};

		/** @} */
	} // namespace render
} // namespace b3d

//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "RenderAPI/BsVideoModeInfo.h"
#include <CoreGraphics/CoreGraphics.h>
#include <CoreVideo/CoreVideo.h>

namespace bs { namespace ct
	{
		/** @addtogroup GL
		 *  @{
		 */

		/** @copydoc VideoMode */
		class MacOSVideoMode : public VideoMode
		{
		public:
			MacOSVideoMode(UINT32 width, UINT32 height, float refreshRate, UINT32 outputIdx);
			~MacOSVideoMode() override;

			/** Returns internal Core Graphics video mode reference. */
			CGDisplayModeRef _getModeRef() const { return mModeRef; }

		private:
			MacOSVideoMode(CGDisplayModeRef modeRef, CVDisplayLinkRef linkRef, UINT32 outputIdx);
			friend class MacOSVideoOutputInfo;

			CGDisplayModeRef mModeRef;
		};

		/** @copydoc VideoOutputInfo */
		class MacOSVideoOutputInfo : public VideoOutputInfo
		{
		public:
			MacOSVideoOutputInfo(CGDirectDisplayID displayID, UINT32 outputIdx);

			/** Returns the Core Graphics identifier for this display. */
			CGDirectDisplayID _getDisplayID() const { return mDisplayID; }
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
	}}


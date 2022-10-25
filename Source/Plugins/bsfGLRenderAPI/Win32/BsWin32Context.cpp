//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#ifndef _WIN32_WINNT
#	define _WIN32_WINNT 0x0500
#endif

#include "Win32/BsWin32Context.h"
#include "Error/BsException.h"

namespace bs
{
	namespace ct
	{
		Win32Context::Win32Context(HDC hdc, HGLRC glrc, bool ownsContext)
			: mHDC(hdc), mGlrc(glrc), mOwnsContext(ownsContext)
		{
		}

		Win32Context::~Win32Context()
		{
			if(mOwnsContext)
				ReleaseContext();
		}

		void Win32Context::SetCurrent(const RenderWindow& window)
		{
			if(wglMakeCurrent(mHDC, mGlrc) != TRUE)
				BS_EXCEPT(RenderingAPIException, "wglMakeCurrent failed: " + translateWGLError());
		}

		void Win32Context::EndCurrent()
		{
			if(wglMakeCurrent(mHDC, 0) != TRUE)
				BS_EXCEPT(RenderingAPIException, "wglMakeCurrent failed: " + translateWGLError());
		}

		void Win32Context::ReleaseContext()
		{
			if(mGlrc != 0)
			{
				if(wglDeleteContext(mGlrc) != TRUE)
					BS_EXCEPT(RenderingAPIException, "wglDeleteContext failed: " + translateWGLError());

				mGlrc = 0;
				mHDC = 0;
			}
		}
	} // namespace ct
} // namespace bs

//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Linux/BsLinuxContext.h"
#include "Private/Linux/BsLinuxPlatform.h"
#include "Linux/BsLinuxGLSupport.h"

namespace bs { namespace ct
{
	typedef int(*ErrorHandlerProc)(::Display*, XErrorEvent*);
	int contextErrorHandler(::Display* display, XErrorEvent* error)
	{
		// Do nothing
		return 0;
	}

	LinuxContext::LinuxContext(::Display* display, XVisualInfo& visualInfo)
	: mDisplay(display)
	{
		LinuxPlatform::lockX();

		INT32 dummy;
		XVisualInfo* windowVisualInfo = XGetVisualInfo(display, VisualIDMask | VisualScreenMask, &visualInfo, &dummy);

		INT32 majorVersion, minorVersion;
		glXQueryVersion(display, &majorVersion, &minorVersion);

		GLXContext context = 0;

		// createContextAttrib was added in GLX version 1.3
		bool hasCreateContextAttrib = extGLX_ARB_create_context && (majorVersion > 1 || minorVersion >= 3);
		if(hasCreateContextAttrib)
		{
			// Find the config used to create the window's visual
			GLXFBConfig* windowConfig = nullptr;

			INT32 numConfigs;
			GLXFBConfig* configs = glXChooseFBConfig(display, DefaultScreen(display), nullptr, &numConfigs);

			for (INT32 i = 0; i < numConfigs; ++i)
			{
				XVisualInfo* configVisualInfo = glXGetVisualFromFBConfig(display, configs[i]);

				if(!configVisualInfo)
					continue;

				if(windowVisualInfo->visualid == configVisualInfo->visualid)
				{
					windowConfig = &configs[i];
					break;
				}
			}

			if(windowConfig)
			{
				int32_t attributes[] =
				{
					GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
					GLX_CONTEXT_MINOR_VERSION_ARB, 5,
					0, 0, // Core profile
					0, 0, // Debug flags
					0 // Terminator
				};

				if(extGLX_ARB_create_context_profile)
				{
					attributes[4] = GLX_CONTEXT_PROFILE_MASK_ARB;
					attributes[5] = GLX_CONTEXT_CORE_PROFILE_BIT_ARB;
				}

#if BS_DEBUG_MODE
					attributes[6] = GLX_CONTEXT_FLAGS_ARB;
					attributes[7] = GLX_CONTEXT_DEBUG_BIT_ARB;
#endif

				// Add error handler so the application doesn't crash on error
				ErrorHandlerProc oldErrorHandler = XSetErrorHandler(&contextErrorHandler);
				context = glXCreateContextAttribsARB(display, *windowConfig, 0, True, attributes);
				XSetErrorHandler(oldErrorHandler);
			}

			XFree(configs);
		}

		// If createContextAttribs failed or isn't supported, fall back to glXCreateContext
		if(!context)
			context = glXCreateContext(display, windowVisualInfo, 0, True);

		XFree(windowVisualInfo);

		mContext = context;

		LinuxPlatform::unlockX();
	}

	LinuxContext::~LinuxContext()
	{
		releaseContext();
	}

	void LinuxContext::setCurrent(const RenderWindow& window)
	{
		window.getCustomAttribute("WINDOW", &mCurrentWindow);

		LinuxPlatform::lockX();
		glXMakeCurrent(mDisplay, mCurrentWindow, mContext);
		LinuxPlatform::unlockX();
	}

	void LinuxContext::endCurrent()
	{
		LinuxPlatform::lockX();
		glXMakeCurrent(mDisplay, 0, 0);
		LinuxPlatform::unlockX();
	}

	void LinuxContext::releaseContext()
	{
		if (mContext)
		{
			LinuxPlatform::lockX();

			glXDestroyContext(mDisplay, mContext);
			mContext = 0;

			LinuxPlatform::unlockX();
		}
	}
}}
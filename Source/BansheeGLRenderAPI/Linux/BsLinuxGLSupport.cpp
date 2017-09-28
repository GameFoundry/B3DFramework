//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Linux/BsLinuxPlatform.h"
#include "Linux/BsLinuxGLSupport.h"
#include "Linux/BsLinuxContext.h"
#include "Linux/BsLinuxRenderWindow.h"
#include "GL/wglew.h"
#include "BsLinuxVideoModeInfo.h"
#include "BsGLRenderAPI.h

namespace bs { namespace ct
{
	bool extGLX_ARB_multisample = false;
	bool extGLX_ARB_framebuffer_sRGB = false;
	bool extGLX_EXT_framebuffer_sRGB = false;
	bool extGLX_ARB_create_context = false;
	bool extGLX_ARB_create_context_profile = false;
	bool extGLX_EXT_swap_control = false;
	bool extGLX_MESA_swap_control = false;
	bool extGLX_SGI_swap_control = false;

	typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
	glXCreateContextAttribsARBProc glXCreateContextAttribsARB = nullptr;

	bool Load_ARB_create_context()
	{
		glXCreateContextAttribsARB =
				(glXCreateContextAttribsARBProc)glXGetProcAddressARB((const GLubyte*)"glXCreateContextAttribsARB");

		return glXCreateContextAttribsARB != nullptr;
	}

	typedef void (*glXSwapIntervalEXTProc)(::Display*, GLXDrawable, int);
	typedef int (*glXSwapIntervalMESAProc)(int);
	typedef int (*glXSwapIntervalSGIProc)(int);

	glXSwapIntervalEXTProc glXSwapIntervalEXT = nullptr;
	glXSwapIntervalMESAProc glXSwapIntervalMESA = nullptr;
	glXSwapIntervalSGIProc glXSwapIntervalSGI = nullptr;

	bool Load_EXT_swap_control()
	{
		glXSwapIntervalEXT = (glXSwapIntervalEXTProc)glXGetProcAddressARB((const GLubyte*)"glXSwapIntervalEXT");

		return glXSwapIntervalEXT != nullptr;
	}

	bool Load_MESA_swap_control()
	{
		glXSwapIntervalMESA = (glXSwapIntervalMESAProc)glXGetProcAddressARB((const GLubyte*)"glXSwapIntervalMESA");

		return glXSwapIntervalMESA != nullptr;
	}

	bool Load_SGI_swap_control()
	{
		glXSwapIntervalSGI = (glXSwapIntervalSGIProc)glXGetProcAddressARB((const GLubyte*)"glXSwapIntervalSGI");

		return glXSwapIntervalSGI != nullptr;
	}

	typedef bool (*ExtensionFunc)(void);

	struct GLExtension
	{
		const char* name;
		bool* status;
		ExtensionFunc func;
	};

	static GLExtension gExtensionMap[] = {
		{ "GLX_ARB_multisample", &extGLX_ARB_multisample, nullptr },
		{ "GLX_ARB_framebuffer_sRGB", &extGLX_ARB_framebuffer_sRGB, nullptr },
		{ "GLX_EXT_framebuffer_sRGB", &extGLX_EXT_framebuffer_sRGB, nullptr },
		{ "GLX_ARB_create_context", &extGLX_ARB_create_context, Load_ARB_create_context },
		{ "GLX_ARB_create_context_profile", &extGLX_ARB_create_context_profile, nullptr },
		{ "GLX_EXT_swap_control", &extGLX_EXT_swap_control, Load_EXT_swap_control },
		{ "GLX_MESA_swap_control", &extGLX_MESA_swap_control, Load_MESA_swap_control },
		{ "GLX_SGI_swap_control", &extGLX_SGI_swap_control, Load_SGI_swap_control },
	};

	LinuxGLSupport::LinuxGLSupport()
	{ }

	SPtr<bs::RenderWindow> LinuxGLSupport::newWindow(RENDER_WINDOW_DESC& desc, UINT32 windowId,
		SPtr<bs::RenderWindow> parentWindow)
	{
		if(parentWindow != nullptr)
		{
			::Window x11window;
			parentWindow->getCustomAttribute("WINDOW", &x11window);
			desc.platformSpecific["parentWindowHandle"] = toString((UINT64)x11window);
		}

		bs::LinuxRenderWindow* window = new (bs_alloc<bs::LinuxRenderWindow>()) bs::LinuxRenderWindow(desc, windowId, *this);
		return SPtr<bs::RenderWindow>(window, &bs::CoreObject::_delete<bs::LinuxRenderWindow, GenAlloc>);
	}

	SPtr<RenderWindow> LinuxGLSupport::newWindowCore(RENDER_WINDOW_DESC& desc, UINT32 windowId)
	{
		LinuxRenderWindow* window = new (bs_alloc<LinuxRenderWindow>()) LinuxRenderWindow(desc, windowId, *this);

		return bs_shared_ptr<LinuxRenderWindow>(window);
	}

	void LinuxGLSupport::start()
	{
		// Retrieve all essential extensions
		LinuxPlatform::lockX();

		::Display* display = LinuxPlatform::getXDisplay();
		const char* glExtensions = glXQueryExtensionsString(display, DefaultScreen(display));

		const char* iter = glExtensions;
		do
		{
			const char* start = iter;
			while(*iter != ' ' && *iter)
				iter++;

			const char* end = iter;
			const char* name = std::string(start, end).c_str();

			uint32_t numExtensions = sizeof(gExtensionMap) / sizeof(gExtensionMap[0]);
			for (int i = 0; i < numExtensions; ++i)
			{
				if(strcmp(name, gExtensionMap[i].name) == 0)
				{
					if(gExtensionMap[i].func != nullptr)
						*gExtensionMap[i].status = gExtensionMap[i].func();
					else
						*gExtensionMap[i].status = true;
				}

			}

		} while(*iter++);

		LinuxPlatform::unlockX();
	}

	void LinuxGLSupport::stop()
	{
		// Do nothing
	}

	SPtr<LinuxContext> LinuxGLSupport::createContext(::Display* x11display, XVisualInfo& visualInfo)
	{
		GLRenderAPI* rapi = static_cast<GLRenderAPI*>(RenderAPI::instancePtr());

		// If RenderAPI has initialized a context use that, otherwise we create our own
		if (!rapi->_isContextInitialized())
			return bs_shared_ptr_new<LinuxContext>(x11display, visualInfo);
		else
		{
			SPtr<LinuxContext> context = rapi->getMainContext();
			context->setCurrent();

			return context;
		}
	}

	void* LinuxGLSupport::getProcAddress(const String& procname)
	{
		return (void*)glXGetProcAddressARB((const GLubyte*)procname.c_str());
	}

	GLVisualConfig LinuxGLSupport::findBestVisual(::Display* display, bool depthStencil, UINT32 multisample, bool srgb) const
	{
		static constexpr INT32 VISUAL_ATTRIBS[] =
		{
			GLX_X_RENDERABLE, 		True,
			GLX_DRAWABLE_TYPE, 		GLX_WINDOW_BIT,
			GLX_RENDER_TYPE,		GLX_RGBA_BIT,
			GLX_X_VISUAL_TYPE,		GLX_TRUE_COLOR,
			GLX_RED_SIZE,			8,
			GLX_GREEN_SIZE,			8,
			GLX_BLUE_SIZE,			8,
			GLX_ALPHA_SIZE,			8
		};

		INT32 numConfigs;
		GLXFBConfig* configs = glXChooseFBConfig(display, DefaultScreen(display), VISUAL_ATTRIBS, &numConfigs);
		GLVisualCapabilities* caps = bs_stack_new<GLVisualCapabilities>(numConfigs);

		// Find a config that best matches the requested properties
		INT32 bestScore = 0;
		INT32 bestConfig = -1;
		for (int i = 0; i < numConfigs; ++i)
		{
			UINT32 configScore = 0;

			// Depth buffer contributes the most to score
			if(depthStencil)
			{
				INT32 depth, stencil;
				glXGetFBConfigAttrib(display, configs[i], GLX_DEPTH_SIZE, &depth);
				glXGetFBConfigAttrib(display, configs[i], GLX_STENCIL_SIZE, &stencil);

				UINT32 score = 0;
				if(depth == 24 && stencil == 8)
					score = 10000;
				else if(depth == 32 && stencil == 8)
					score = 9000;
				else if(depth == 32)
					score = 8000;
				else if(depth == 16)
					score = 7000;

				if(score > 0)
				{
					configScore += score;
					caps[i].depthStencil = true;
				}
			}

			// sRGB contributes second most
			if(srgb)
			{
				int32_t hasSRGB = 0;

				if(extGLX_EXT_framebuffer_sRGB)
					glXGetFBConfigAttrib(display, configs[i], GLX_FRAMEBUFFER_SRGB_CAPABLE_EXT, &hasSRGB);

				if(!hasSRGB && extGLX_ARB_framebuffer_sRGB)
					glXGetFBConfigAttrib(display, configs[i], GLX_FRAMEBUFFER_SRGB_CAPABLE_EXT, &hasSRGB);

				if(hasSRGB)
				{
					configScore += 500;
					caps[i].srgb = true;
				}
			}

			if((multisample >= 1) && extGLX_ARB_multisample)
			{
				int32_t hasMultisample, numSamples;
				glXGetFBConfigAttrib(display, configs[i], GLX_SAMPLE_BUFFERS, &hasMultisample);
				glXGetFBConfigAttrib(display, configs[i], GLX_SAMPLES, &numSamples);

				if(hasMultisample && (numSamples <= multisample))
				{
					configScore += (32 - (multisample - numSamples)) * 10;
					caps[i].numSamples = numSamples;
				}
			}

			if(configScore > bestScore)
			{
				bestScore = configScore;
				bestConfig = i;
			}
		}

		GLVisualConfig output;

		if(bestConfig == -1)
		{
			// Something went wrong
			XFree(configs);
			bs_stack_delete(caps, numConfigs);

			return output;
		}

		XVisualInfo* visualInfo = glXGetVisualFromFBConfig(display, configs[bestConfig]);

		output.visualInfo = *visualInfo;
		output.caps = caps[bestConfig];

		XFree(configs);
		XFree(visualInfo);
		bs_stack_delete(caps, numConfigs);

		return output;
	}

	SPtr<VideoModeInfo> LinuxGLSupport::getVideoModeInfo() const
	{
		return bs_shared_ptr_new<LinuxVideoModeInfo>();
	}
}}
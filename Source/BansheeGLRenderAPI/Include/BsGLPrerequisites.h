//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"

#if BS_THREAD_SUPPORT == 1
#	define GLEW_MX
#endif

#if BS_PLATFORM == BS_PLATFORM_WIN32
#if !defined( __MINGW32__ )
#   define WIN32_LEAN_AND_MEAN
#  ifndef NOMINMAX
#	define NOMINMAX // required to stop windows.h messing up std::min
#  endif
#endif
#   include <windows.h>
#   include <wingdi.h>
#   include <GL/glew.h>
#   include <GL/wglew.h>
#   include <GL/glu.h>
#elif BS_PLATFORM == BS_PLATFORM_LINUX
#   include <GL/glew.h>
#   include <GL/glu.h>
#   define GL_GLEXT_PROTOTYPES
#elif BS_PLATFORM == BS_PLATFORM_OSX
#   include <GL/glew.h>
#   include <OpenGL/glu.h>
#endif

#if BS_THREAD_SUPPORT == 1
	GLEWContext * glewGetContext();

#	if BS_PLATFORM == BS_PLATFORM_WIN32
	WGLEWContext * wglewGetContext();
#	endif

#endif

// Lots of generated code in here which triggers the new VC CRT security warnings
#if !defined( _CRT_SECURE_NO_DEPRECATE )
#define _CRT_SECURE_NO_DEPRECATE
#endif

#if (BS_PLATFORM == BS_PLATFORM_WIN32) && !defined(__MINGW32__) && !defined(BS_STATIC_LIB)
#	ifdef BS_RSGL_EXPORTS
#		define BS_RSGL_EXPORT __declspec(dllexport)
#	else
#       if defined( __MINGW32__ )
#           define BS_RSGL_EXPORT
#       else
#    		define BS_RSGL_EXPORT __declspec(dllimport)
#       endif
#	endif
#elif defined ( BS_GCC_VISIBILITY )
#    define BS_RSGL_EXPORT  __attribute__ ((visibility("default")))
#else
#    define BS_RSGL_EXPORT
#endif

/** @addtogroup Plugins
 *  @{
 */

/** @defgroup GL BansheeGLRenderAPI
 *	Wrapper around the OpenGL render API.
 */

/** @} */

namespace bs { namespace ct
{
	extern const char* MODULE_NAME;

    class GLSupport;
    class GLRenderAPI;
    class GLTexture;
	class GLVertexBuffer;
    class GLContext;
    class GLRTTManager;
    class GLPixelBuffer;
	class GLGpuParamBlock;
	class GLSLGpuProgram;
	class GLVertexArrayObject;
	struct GLSLProgramPipeline;
	class GLSLProgramPipelineManager;

	/** @addtogroup GL
	 *  @{
	 */

	/**	OpenGL specific types to track resource statistics for. */
	enum GLRenderStatResourceType
	{
		RenderStatObject_PipelineObject = 100,
		RenderStatObject_FrameBufferObject,
		RenderStatObject_VertexArrayObject
	};

	/** @} */
}}

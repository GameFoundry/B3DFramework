//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"

#if BS_THREAD_SUPPORT == 1
#	define GLEW_MX
#endif

// 4.1 is the minimum supported version for OpenGL
#define BS_OPENGL_4_1 1

#if BS_PLATFORM != BS_PLATFORM_OSX
	#define BS_OPENGL_4_2 1
	#define BS_OPENGL_4_3 1
	#define BS_OPENGL_4_4 1
	#define BS_OPENGL_4_5 1
	#define BS_OPENGL_4_6 0
#endif

// 3.1 is the minimum supported version for OpenGL ES
#define BS_OPENGLES_3_1 0
#define BS_OPENGLES_3_2 0

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
#elif BS_PLATFORM == BS_PLATFORM_LINUX
#   include <GL/glew.h>
#   include <GL/glu.h>
#   define GL_GLEXT_PROTOTYPES
#elif BS_PLATFORM == BS_PLATFORM_OSX
#   include <OpenGL/gl3.h>
#   include <OpenGL/gl3ext.h>
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

/** @addtogroup Plugins
 *  @{
 */

/** @defgroup GL BansheeGLRenderAPI
 *	Wrapper around the OpenGL render API.
 */

/** @} */

namespace bs { namespace ct
{
	/** Translated an OpenGL error code enum to an error code string. */
	const char* bs_get_gl_error_string(GLenum errorCode);

	/** Checks if there have been any OpenGL errors since the last call, and if so reports them. */
	void bs_check_gl_error(const char* function, const char* file, INT32 line);

#if BS_DEBUG_MODE && (!BS_OPENGL_4_3 && !BS_OPENGLES_3_2)
	#define BS_CHECK_GL_ERROR() bs_check_gl_error(__PRETTY_FUNCTION__, __FILE__,__LINE__)
#else
	#define BS_CHECK_GL_ERROR()
#endif

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
	class GLTextureView;

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

set(BS_GLRENDERAPI_SRC_GLSL
	"GLSL/BsGLSLProgramPipelineManager.cpp"
	"GLSL/BsGLSLProgramFactory.cpp"
	"GLSL/BsGLSLGpuProgram.cpp"
	"GLSL/BsGLSLParamParser.cpp"
)

set(BS_GLRENDERAPI_INC_NOFILTER
	"BsGLVertexArrayObjectManager.h"
	"BsGLVertexBuffer.h"
	"BsGLTimerQuery.h"
	"BsGLTextureManager.h"
	"BsGLTexture.h"
	"BsGLSupport.h"
	"BsGLRenderWindowManager.h"
	"BsGLRenderTexture.h"
	"BsGLQueryManager.h"
	"BsGLPrerequisites.h"
	"BsGLPixelFormat.h"
	"BsGLPixelBuffer.h"
	"BsGLOcclusionQuery.h"
	"BsGLIndexBuffer.h"
	"BsGLHardwareBufferManager.h"
	"BsGLGpuParamBlockBuffer.h"
	"BsGLGpuBuffer.h"
	"BsGLFrameBufferObject.h"
	"BsGLEventQuery.h"
	"BsGLContext.h"
	"BsGLRenderAPI.h"
	"BsGLRenderAPIFactory.h"
	"BsGLUtil.h"
	"BsGLHardwareBuffer.h"
	"BsGLCommandBuffer.h"
	"BsGLCommandBufferManager.h"
	"BsGLTextureView.h"
)

set(BS_GLRENDERAPI_SRC_NOFILTER
	"BsGLVertexArrayObjectManager.cpp"
	"BsGLVertexBuffer.cpp"
	"BsGLTimerQuery.cpp"
	"BsGLTextureManager.cpp"
	"BsGLTexture.cpp"
	"BsGLSupport.cpp"
	"BsGLRenderWindowManager.cpp"
	"BsGLRenderTexture.cpp"
	"BsGLQueryManager.cpp"
	"BsGLPixelFormat.cpp"
	"BsGLPixelBuffer.cpp"
	"BsGLOcclusionQuery.cpp"
	"BsGLIndexBuffer.cpp"
	"BsGLHardwareBufferManager.cpp"
	"BsGLGpuParamBlockBuffer.cpp"
	"BsGLGpuBuffer.cpp"
	"BsGLFrameBufferObject.cpp"
	"BsGLEventQuery.cpp"
	"BsGLContext.cpp"
	"BsGLRenderAPI.cpp"
	"BsGLRenderAPIFactory.cpp"
	"BsGLPlugin.cpp"
	"BsGLHardwareBuffer.cpp"
	"BsGLCommandBuffer.cpp"
	"BsGLCommandBufferManager.cpp"
	"BsGLTextureView.cpp"
)

set(BS_GLRENDERAPI_INC_GLSL
	"GLSL/BsGLSLProgramPipelineManager.h"
	"GLSL/BsGLSLProgramFactory.h"
	"GLSL/BsGLSLParamParser.h"
	"GLSL/BsGLSLGpuProgram.h"
)

set(BS_GLRENDERAPI_INC_WIN32
	"Win32/BsWin32Context.h"
	"Win32/BsWin32GLSupport.h"
	"Win32/BsWin32Prerequisites.h"
	"Win32/BsWin32RenderWindow.h"
	"Win32/BsWin32VideoModeInfo.h"
)

set(BS_GLRENDERAPI_SRC_WIN32
	"Win32/BsWin32Context.cpp"
	"Win32/BsWin32GLSupport.cpp"
	"Win32/BsWin32RenderWindow.cpp"
	"Win32/BsWin32VideoModeInfo.cpp"
	"glew.cpp"
	"Win32/BsGLWin32Resource.rc"
)

set(BS_GLRENDERAPI_INC_LINUX
	"Linux/BsLinuxContext.h"
	"Linux/BsLinuxGLSupport.h"
	"Linux/BsLinuxRenderWindow.h"
	"Linux/BsLinuxVideoModeInfo.h"
)

set(BS_GLRENDERAPI_SRC_LINUX
	"Linux/BsLinuxContext.cpp"
	"Linux/BsLinuxGLSupport.cpp"
	"Linux/BsLinuxRenderWindow.cpp"
	"Linux/BsLinuxVideoModeInfo.cpp"
	"glew.cpp"
)

set_source_files_properties ("Linux/BsLinuxContext.cpp" PROPERTIES
		COTIRE_EXCLUDED "True")

set(BS_GLRENDERAPI_INC_MACOS
	"MacOS/BsMacOSContext.h"
	"MacOS/BsMacOSGLSupport.h"
	"MacOS/BsMacOSRenderWindow.h"
	"../Shared/MacOS/BsMacOSVideoModeInfo.h"
)

set(BS_GLRENDERAPI_SRC_MACOS
	"MacOS/BsMacOSContext.mm"
	"MacOS/BsMacOSGLSupport.cpp"
	"MacOS/BsMacOSRenderWindow.mm"
	"../Shared/MacOS/BsMacOSVideoModeInfo.cpp"
)

source_group("" FILES ${BS_GLRENDERAPI_INC_NOFILTER} ${BS_GLRENDERAPI_SRC_NOFILTER})
source_group("Win32" FILES ${BS_GLRENDERAPI_INC_WIN32} ${BS_GLRENDERAPI_SRC_WIN32})
source_group("GLSL" FILES ${BS_GLRENDERAPI_INC_GLSL} ${BS_GLRENDERAPI_SRC_GLSL})

set(BS_GLRENDERAPI_SRC
	${BS_GLRENDERAPI_SRC_GLSL}
	${BS_GLRENDERAPI_INC_NOFILTER}
	${BS_GLRENDERAPI_SRC_NOFILTER}
	${BS_GLRENDERAPI_INC_GLSL}
)

if(WIN32)
	list(APPEND BS_GLRENDERAPI_SRC
		${BS_GLRENDERAPI_INC_WIN32}
		${BS_GLRENDERAPI_SRC_WIN32}
	)
elseif(LINUX)
	list(APPEND BS_GLRENDERAPI_SRC
		${BS_GLRENDERAPI_INC_LINUX}
		${BS_GLRENDERAPI_SRC_LINUX}
	)
elseif(APPLE)
	list(APPEND BS_GLRENDERAPI_SRC
		${BS_GLRENDERAPI_INC_MACOS}
		${BS_GLRENDERAPI_SRC_MACOS}
	)
endif()

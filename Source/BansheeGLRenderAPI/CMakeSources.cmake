set(BS_BANSHEEGLRENDERAPI_SRC_GLSL
	"GLSL/BsGLSLProgramPipelineManager.cpp"
	"GLSL/BsGLSLProgramFactory.cpp"
	"GLSL/BsGLSLGpuProgram.cpp"
	"GLSL/BsGLSLParamParser.cpp"
)

set(BS_BANSHEEGLRENDERAPI_INC_NOFILTER
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
	"BsGLBuffer.h"
	"BsGLCommandBuffer.h"
	"BsGLCommandBufferManager.h"
	"BsGLTextureView.h"
)

set(BS_BANSHEEGLRENDERAPI_SRC_NOFILTER
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
	"BsGLBuffer.cpp"
	"BsGLCommandBuffer.cpp"
	"BsGLCommandBufferManager.cpp"
	"BsGLTextureView.cpp"
)

set(BS_BANSHEEGLRENDERAPI_INC_GLSL
	"GLSL/BsGLSLProgramPipelineManager.h"
	"GLSL/BsGLSLProgramFactory.h"
	"GLSL/BsGLSLParamParser.h"
	"GLSL/BsGLSLGpuProgram.h"
)

set(BS_BANSHEEGLRENDERAPI_INC_WIN32
	"Win32/BsWin32Context.h"
	"Win32/BsWin32GLSupport.h"
	"Win32/BsWin32Prerequisites.h"
	"Win32/BsWin32RenderWindow.h"
	"Win32/BsWin32VideoModeInfo.h"
)

set(BS_BANSHEEGLRENDERAPI_SRC_WIN32
	"Win32/BsWin32Context.cpp"
	"Win32/BsWin32GLSupport.cpp"
	"Win32/BsWin32RenderWindow.cpp"
	"Win32/BsWin32VideoModeInfo.cpp"
	"glew.cpp"
)

set(BS_BANSHEEGLRENDERAPI_INC_LINUX
	"Linux/BsLinuxContext.h"
	"Linux/BsLinuxGLSupport.h"
	"Linux/BsLinuxRenderWindow.h"
	"Linux/BsLinuxVideoModeInfo.h"
)

set(BS_BANSHEEGLRENDERAPI_SRC_LINUX
	"Linux/BsLinuxContext.cpp"
	"Linux/BsLinuxGLSupport.cpp"
	"Linux/BsLinuxRenderWindow.cpp"
	"Linux/BsLinuxVideoModeInfo.cpp"
	"glew.cpp"
)

set(BS_BANSHEEGLRENDERAPI_INC_MACOS
	"MacOS/BsMacOSContext.h"
	"MacOS/BsMacOSGLSupport.h"
	"MacOS/BsMacOSRenderWindow.h"
	"MacOS/BsMacOSVideoModeInfo.h"
)

set(BS_BANSHEEGLRENDERAPI_SRC_MACOS
	"MacOS/BsMacOSContext.mm"
	"MacOS/BsMacOSGLSupport.cpp"
	"MacOS/BsMacOSRenderWindow.mm"
	"MacOS/BsMacOSVideoModeInfo.cpp"
)

source_group("Source Files\\GLSL" FILES ${BS_BANSHEEGLRENDERAPI_SRC_GLSL})
source_group("Header Files" FILES ${BS_BANSHEEGLRENDERAPI_INC_NOFILTER})
source_group("Source Files\\Win32" FILES ${BS_BANSHEEGLRENDERAPI_SRC_WIN32})
source_group("Source Files" FILES ${BS_BANSHEEGLRENDERAPI_SRC_NOFILTER})
source_group("Header Files\\GLSL" FILES ${BS_BANSHEEGLRENDERAPI_INC_GLSL})
source_group("Header Files\\Win32" FILES ${BS_BANSHEEGLRENDERAPI_INC_WIN32})

set(BS_BANSHEEGLRENDERAPI_SRC
	${BS_BANSHEEGLRENDERAPI_SRC_GLSL}
	${BS_BANSHEEGLRENDERAPI_INC_NOFILTER}
	${BS_BANSHEEGLRENDERAPI_SRC_NOFILTER}
	${BS_BANSHEEGLRENDERAPI_INC_GLSL}
)

if(WIN32)
	list(APPEND BS_BANSHEEGLRENDERAPI_SRC
		${BS_BANSHEEGLRENDERAPI_INC_WIN32}
		${BS_BANSHEEGLRENDERAPI_SRC_WIN32}
	)
elseif(LINUX)
	list(APPEND BS_BANSHEEGLRENDERAPI_SRC
		${BS_BANSHEEGLRENDERAPI_INC_LINUX}
		${BS_BANSHEEGLRENDERAPI_SRC_LINUX}
	)
elseif(APPLE)
	list(APPEND BS_BANSHEEGLRENDERAPI_SRC
		${BS_BANSHEEGLRENDERAPI_INC_MACOS}
		${BS_BANSHEEGLRENDERAPI_SRC_MACOS}
	)
endif()

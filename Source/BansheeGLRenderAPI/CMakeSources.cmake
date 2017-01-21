set(BS_BANSHEEGLRENDERAPI_SRC_GLSL
	"Source/GLSL/src/BsGLSLProgramPipelineManager.cpp"
	"Source/GLSL/src/BsGLSLProgramFactory.cpp"
	"Source/GLSL/src/BsGLSLGpuProgram.cpp"
	"Source/BsGLSLParamParser.cpp"
)

set(BS_BANSHEEGLRENDERAPI_INC_NOFILTER
	"Include/BsGLVertexArrayObjectManager.h"
	"Include/BsGLVertexBuffer.h"
	"Include/BsGLTimerQuery.h"
	"Include/BsGLTextureManager.h"
	"Include/BsGLTexture.h"
	"Include/BsGLSupport.h"
	"Include/BsGLRenderWindowManager.h"
	"Include/BsGLRenderTexture.h"
	"Include/BsGLQueryManager.h"
	"Include/BsGLPrerequisites.h"
	"Include/BsGLPixelFormat.h"
	"Include/BsGLPixelBuffer.h"
	"Include/BsGLOcclusionQuery.h"
	"Include/BsGLIndexBuffer.h"
	"Include/BsGLHardwareBufferManager.h"
	"Include/BsGLGpuParamBlockBuffer.h"
	"Include/BsGLGpuBuffer.h"
	"Include/BsGLFrameBufferObject.h"
	"Include/BsGLEventQuery.h"
	"Include/BsGLContext.h"
	"Include/BsGLRenderAPI.h"
	"Include/BsGLRenderAPIFactory.h"
	"Include/BsGLUtil.h"
	"Include/BsGLBuffer.h"
	"Include/BsGLCommandBuffer.h"
	"Include/BsGLCommandBufferManager.h"
	"Include/BsGLTextureView.h"
)

set(BS_BANSHEEGLRENDERAPI_SRC_WIN32
	"Source/Win32/BsWin32Context.cpp"
	"Source/Win32/BsWin32GLSupport.cpp"
	"Source/Win32/BsWin32RenderWindow.cpp"
	"Source/Win32/BsWin32VideoModeInfo.cpp"
)

set(BS_BANSHEEGLRENDERAPI_SRC_NOFILTER
	"Source/glew.cpp"
	"Source/BsGLVertexArrayObjectManager.cpp"
	"Source/BsGLVertexBuffer.cpp"
	"Source/BsGLTimerQuery.cpp"
	"Source/BsGLTextureManager.cpp"
	"Source/BsGLTexture.cpp"
	"Source/BsGLSupport.cpp"
	"Source/BsGLRenderWindowManager.cpp"
	"Source/BsGLRenderTexture.cpp"
	"Source/BsGLQueryManager.cpp"
	"Source/BsGLPixelFormat.cpp"
	"Source/BsGLPixelBuffer.cpp"
	"Source/BsGLOcclusionQuery.cpp"
	"Source/BsGLIndexBuffer.cpp"
	"Source/BsGLHardwareBufferManager.cpp"
	"Source/BsGLGpuParamBlockBuffer.cpp"
	"Source/BsGLGpuBuffer.cpp"
	"Source/BsGLFrameBufferObject.cpp"
	"Source/BsGLEventQuery.cpp"
	"Source/BsGLContext.cpp"
	"Source/BsGLRenderAPI.cpp"
	"Source/BsGLRenderAPIFactory.cpp"
	"Source/BsGLPlugin.cpp"
	"Source/BsGLBuffer.cpp"
	"Source/BsGLCommandBuffer.cpp"
	"Source/BsGLCommandBufferManager.cpp"
	"Source/BsGLTextureView.cpp"
)

set(BS_BANSHEEGLRENDERAPI_INC_GLSL
	"Source/GLSL/include/BsGLSLProgramPipelineManager.h"
	"Source/GLSL/include/BsGLSLProgramFactory.h"
	"Source/GLSL/include/BsGLSLParamParser.h"
	"Source/GLSL/include/BsGLSLGpuProgram.h"
)

set(BS_BANSHEEGLRENDERAPI_INC_WIN32
	"Include/Win32/BsWin32Context.h"
	"Include/Win32/BsWin32GLSupport.h"
	"Include/Win32/BsWin32Prerequisites.h"
	"Include/Win32/BsWin32RenderWindow.h"
	"Include/Win32/BsWin32VideoModeInfo.h"
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
endif()

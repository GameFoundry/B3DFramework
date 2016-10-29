set(BS_BANSHEEVULKANRENDERAPI_INC_NOFILTER
	"Include/Win32/BsWin32VideoModeInfo.h"
	"Include/Win32/BsWin32RenderWindow.h"	
	"Include/BsVulkanVertexBuffer.h"
	"Include/BsVulkanTimerQuery.h"
	"Include/BsVulkanTexture.h"
	"Include/BsVulkanRenderTexture.h"
	"Include/BsVulkanPrerequisites.h"
	"Include/BsVulkanOcclusionQuery.h"
	"Include/BsVulkanMappings.h"
	"Include/BsVulkanIndexBuffer.h"
	"Include/BsVulkanHardwareBuffer.h"
	"Include/BsVulkanGpuProgram.h"
	"Include/BsVulkanGpuParamBlockBuffer.h"
	"Include/BsVulkanGpuBuffer.h"
	"Include/BsVulkanEventQuery.h"
	"Include/BsVulkanRenderAPI.h"
	"Include/BsVulkanCommandBuffer.h"
	"Include/BsVulkanDevice.h"
	"Include/BsVulkanGpuPipelineState.h"
	"Include/BsVulkanSwapChain.h"
	"Include/BsVulkanFramebuffer.h"
	"Include/BsVulkanUtility.h"
	"Include/BsVulkanGpuParams.h"
	"Include/BsVulkanDescriptorPool.h"
	"Include/BsVulkanDescriptorLayout.h"
	"Include/BsVulkanResource.h"
	"Include/BsVulkanQueue.h"
)

set(BS_BANSHEEVULKANRENDERAPI_INC_MANAGERS
	"Include/BsVulkanTextureManager.h"
	"Include/BsVulkanRenderWindowManager.h"
	"Include/BsVulkanQueryManager.h"
	"Include/BsVulkanGLSLProgramFactory.h"
	"Include/BsVulkanHardwareBufferManager.h"
	"Include/BsVulkanRenderAPIFactory.h"
	"Include/BsVulkanCommandBufferManager.h"
	"Include/BsVulkanRenderStateManager.h"
	"Include/BsVulkanVertexInputManager.h"
	"Include/BsVulkanDescriptorManager.h"
)

set(BS_BANSHEEVULKANRENDERAPI_SRC_NOFILTER
	"Source/Win32/BsWin32VideoModeInfo.cpp"
	"Source/Win32/BsWin32RenderWindow.cpp"	
	"Source/BsVulkanVertexBuffer.cpp"
	"Source/BsVulkanTimerQuery.cpp"
	"Source/BsVulkanTexture.cpp"
	"Source/BsVulkanRenderTexture.cpp"
	"Source/BsVulkanPlugin.cpp"
	"Source/BsVulkanOcclusionQuery.cpp"
	"Source/BsVulkanMappings.cpp"
	"Source/BsVulkanIndexBuffer.cpp"
	"Source/BsVulkanHardwareBuffer.cpp"
	"Source/BsVulkanGpuProgram.cpp"
	"Source/BsVulkanGpuParamBlockBuffer.cpp"
	"Source/BsVulkanGpuBuffer.cpp"
	"Source/BsVulkanEventQuery.cpp"
	"Source/BsVulkanRenderAPI.cpp"
	"Source/BsVulkanCommandBuffer.cpp"
	"Source/BsVulkanDevice.cpp"
	"Source/BsVulkanGpuPipelineState.cpp"
	"Source/BsVulkanSwapChain.cpp"
	"Source/BsVulkanFramebuffer.cpp"
	"Source/BsVulkanUtility.cpp"
	"Source/BsVulkanGpuParams.cpp"
	"Source/BsVulkanDescriptorPool.cpp"
	"Source/BsVulkanDescriptorLayout.cpp"
	"Source/BsVulkanResource.cpp"
	"Source/BsVulkanQueue.cpp"
)

set(BS_BANSHEEVULKANRENDERAPI_SRC_MANAGERS
	"Source/BsVulkanTextureManager.cpp"
	"Source/BsVulkanRenderWindowManager.cpp"
	"Source/BsVulkanQueryManager.cpp"
	"Source/BsVulkanGLSLProgramFactory.cpp"
	"Source/BsVulkanHardwareBufferManager.cpp"
	"Source/BsVulkanRenderAPIFactory.cpp"
	"Source/BsVulkanCommandBufferManager.cpp"
	"Source/BsVulkanRenderStateManager.cpp"
	"Source/BsVulkanVertexInputManager.cpp"
	"Source/BsVulkanDescriptorManager.cpp"
)

source_group("Header Files" FILES ${BS_BANSHEEVULKANRENDERAPI_INC_NOFILTER})
source_group("Header Files\\Managers" FILES ${BS_BANSHEEVULKANRENDERAPI_INC_MANAGERS})
source_group("Source Files" FILES ${BS_BANSHEEVULKANRENDERAPI_SRC_NOFILTER})
source_group("Source Files\\Managers" FILES ${BS_BANSHEEVULKANRENDERAPI_SRC_MANAGERS})

set(BS_BANSHEEVULKANRENDERAPI_SRC
	${BS_BANSHEEVULKANRENDERAPI_INC_NOFILTER}
	${BS_BANSHEEVULKANRENDERAPI_SRC_NOFILTER}
	${BS_BANSHEEVULKANRENDERAPI_INC_MANAGERS}
	${BS_BANSHEEVULKANRENDERAPI_SRC_MANAGERS}
)
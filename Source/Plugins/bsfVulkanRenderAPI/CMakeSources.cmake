set(BS_VULKANRENDERAPI_INC_NOFILTER
	"BsVulkanVertexBuffer.h"
	"BsVulkanTimerQuery.h"
	"BsVulkanTexture.h"
	"BsVulkanRenderTexture.h"
	"BsVulkanPrerequisites.h"
	"BsVulkanOcclusionQuery.h"
	"BsVulkanIndexBuffer.h"
	"BsVulkanHardwareBuffer.h"
	"BsVulkanGpuProgram.h"
	"BsVulkanGpuParamBlockBuffer.h"
	"BsVulkanGpuBuffer.h"
	"BsVulkanEventQuery.h"
	"BsVulkanRenderAPI.h"
	"BsVulkanCommandBuffer.h"
	"BsVulkanDevice.h"
	"BsVulkanGpuPipelineState.h"
	"BsVulkanSwapChain.h"
	"BsVulkanFramebuffer.h"
	"BsVulkanUtility.h"
	"BsVulkanGpuParams.h"
	"BsVulkanDescriptorPool.h"
	"BsVulkanDescriptorLayout.h"
	"BsVulkanResource.h"
	"BsVulkanQueue.h"
	"BsVulkanDescriptorSet.h"
	"BsVulkanSamplerState.h"
	"BsVulkanGpuPipelineParamInfo.h"
)

set(BS_VULKANRENDERAPI_INC_MANAGERS
	"Managers/BsVulkanTextureManager.h"
	"Managers/BsVulkanRenderWindowManager.h"
	"Managers/BsVulkanQueryManager.h"
	"Managers/BsVulkanGLSLProgramFactory.h"
	"Managers/BsVulkanHardwareBufferManager.h"
	"Managers/BsVulkanRenderAPIFactory.h"
	"Managers/BsVulkanCommandBufferManager.h"
	"Managers/BsVulkanRenderStateManager.h"
	"Managers/BsVulkanVertexInputManager.h"
	"Managers/BsVulkanDescriptorManager.h"
)

set(BS_VULKANRENDERAPI_SRC_NOFILTER
	"BsVulkanVertexBuffer.cpp"
	"BsVulkanTimerQuery.cpp"
	"BsVulkanTexture.cpp"
	"BsVulkanRenderTexture.cpp"
	"BsVulkanPlugin.cpp"
	"BsVulkanOcclusionQuery.cpp"
	"BsVulkanIndexBuffer.cpp"
	"BsVulkanHardwareBuffer.cpp"
	"BsVulkanGpuProgram.cpp"
	"BsVulkanGpuParamBlockBuffer.cpp"
	"BsVulkanGpuBuffer.cpp"
	"BsVulkanEventQuery.cpp"
	"BsVulkanRenderAPI.cpp"
	"BsVulkanCommandBuffer.cpp"
	"BsVulkanDevice.cpp"
	"BsVulkanGpuPipelineState.cpp"
	"BsVulkanSwapChain.cpp"
	"BsVulkanFramebuffer.cpp"
	"BsVulkanUtility.cpp"
	"BsVulkanGpuParams.cpp"
	"BsVulkanDescriptorPool.cpp"
	"BsVulkanDescriptorLayout.cpp"
	"BsVulkanResource.cpp"
	"BsVulkanQueue.cpp"
	"BsVulkanDescriptorSet.cpp"
	"BsVulkanSamplerState.cpp"
	"BsVulkanGpuPipelineParamInfo.cpp"
)

set(BS_VULKANRENDERAPI_SRC_MANAGERS
	"Managers/BsVulkanTextureManager.cpp"
	"Managers/BsVulkanRenderWindowManager.cpp"
	"Managers/BsVulkanQueryManager.cpp"
	"Managers/BsVulkanGLSLProgramFactory.cpp"
	"Managers/BsVulkanHardwareBufferManager.cpp"
	"Managers/BsVulkanRenderAPIFactory.cpp"
	"Managers/BsVulkanCommandBufferManager.cpp"
	"Managers/BsVulkanRenderStateManager.cpp"
	"Managers/BsVulkanVertexInputManager.cpp"
	"Managers/BsVulkanDescriptorManager.cpp"
)

set(BS_VULKANRENDERAPI_INC_WIN32
	"Win32/BsWin32VideoModeInfo.h"
	"Win32/BsWin32RenderWindow.h"
)

set(BS_VULKANRENDERAPI_SRC_WIN32
	"Win32/BsWin32VideoModeInfo.cpp"
	"Win32/BsWin32RenderWindow.cpp"
	"Win32/BsVulkanWin32Resource.rc"
)

set(BS_VULKANRENDERAPI_INC_LINUX
	"Linux/BsLinuxVideoModeInfo.h"
	"Linux/BsLinuxRenderWindow.h"
)

set(BS_VULKANRENDERAPI_SRC_LINUX
	"Linux/BsLinuxVideoModeInfo.cpp"
	"Linux/BsLinuxRenderWindow.cpp"
)

source_group("" FILES ${BS_VULKANRENDERAPI_INC_NOFILTER} ${BS_VULKANRENDERAPI_SRC_NOFILTER})
source_group("Managers" FILES ${BS_VULKANRENDERAPI_INC_MANAGERS} ${BS_VULKANRENDERAPI_SRC_MANAGERS})
source_group("Win32" FILES ${BS_VULKANRENDERAPI_INC_WIN32} ${BS_VULKANRENDERAPI_SRC_WIN32})

set(BS_VULKANRENDERAPI_SRC
	${BS_VULKANRENDERAPI_INC_NOFILTER}
	${BS_VULKANRENDERAPI_SRC_NOFILTER}
	${BS_VULKANRENDERAPI_INC_MANAGERS}
	${BS_VULKANRENDERAPI_SRC_MANAGERS}
)

if(WIN32)
	list(APPEND BS_VULKANRENDERAPI_SRC
		${BS_VULKANRENDERAPI_INC_WIN32}
		${BS_VULKANRENDERAPI_SRC_WIN32}
	)
elseif(LINUX)
	list(APPEND BS_VULKANRENDERAPI_SRC
		${BS_VULKANRENDERAPI_INC_LINUX}
		${BS_VULKANRENDERAPI_SRC_LINUX}
	)
endif()

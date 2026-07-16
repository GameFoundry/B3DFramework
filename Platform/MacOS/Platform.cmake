# See Platform/Win32/Platform.cmake for interface.
if(APPLE)
	list(APPEND B3D_GPU_BACKEND_CHOICES Metal Vulkan Null)
	set(B3D_GPU_BACKEND_DEFAULT Metal)

	set(B3D_GPU_BACKEND_LIB_Metal bsfMetalGpuBackend)
	set(B3D_GPU_BACKEND_LIB_Vulkan bsfVulkanGpuBackend)
	set(B3D_GPU_BACKEND_LIB_Null bsfNullGpuBackend)

	# Needed for marl
	enable_language(ASM)
endif()

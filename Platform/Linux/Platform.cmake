# See Platform/Win32/Platform.cmake for interface.
if(LINUX)
	list(APPEND B3D_GPU_BACKEND_CHOICES Vulkan Null)
	set(B3D_GPU_BACKEND_DEFAULT Vulkan)

	# Needed for marl
	enable_language(ASM)
endif()

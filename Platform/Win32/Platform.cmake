# Each platform contributes to the GPU backend collectors:
#   B3D_GPU_BACKEND_CHOICES        - list of selectable backend names (cache STRINGS)
#   B3D_GPU_BACKEND_DEFAULT        - default backend when the user hasn't chosen one
if(WIN32)
	list(APPEND B3D_GPU_BACKEND_CHOICES Vulkan DirectX12 Null)
	set(B3D_GPU_BACKEND_DEFAULT Vulkan)
endif()

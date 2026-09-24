# Each platform contributes to the GPU backend collectors:
#   B3D_GPU_BACKEND_CHOICES        - list of selectable backend names (cache STRINGS)
#   B3D_GPU_BACKEND_DEFAULT        - default backend when the user hasn't chosen one
#
# A custom platform may also declare the package server holding its prebuilt packages (see B3DGetPackageServer):
#   B3D_PLATFORM_<Name>_PACKAGE_URL   - base URL of the server
#   B3D_PLATFORM_<Name>_PACKAGE_TOKEN - read-only token the server requires as "Authorization: Bearer", if any
#
# A custom platform's Platform.cmake is read in every tree where its overlay is present, so it sets those first and
# returns early unless it is the build target (B3D_PLATFORM).
if(WIN32)
	list(APPEND B3D_GPU_BACKEND_CHOICES Vulkan DirectX12 Null)
	set(B3D_GPU_BACKEND_DEFAULT Vulkan)
endif()

# Source files and their filters
## Note: The shared RenderDoc frame capture (Plugins/Shared) is Vulkan-specific and deliberately not globbed here.
B3DGlobSourceFiles(${CMAKE_CURRENT_SOURCE_DIR} "" "" BS_D3D12GPUBACKEND_SRC)

# Target
B3DAddPlugin(bsfD3D12GpuBackend ENGINE ${BS_D3D12GPUBACKEND_SRC})

# Common flags
B3DSetDefaultLinkAndCompileFlags(bsfD3D12GpuBackend)

# Includes
target_include_directories(bsfD3D12GpuBackend PRIVATE
	"./"
)

# Definitions
target_compile_definitions(bsfD3D12GpuBackend PRIVATE
	-DB3D_D3D12_EXPORTS
)

# Libraries
## Windows-specific D3D12 libraries
if(WIN32)
	target_link_libraries(bsfD3D12GpuBackend PRIVATE
		d3d12.lib
		dxgi.lib
		dxguid.lib
		d3dcompiler.lib
	)
endif()

# IDE specific
set_property(TARGET bsfD3D12GpuBackend PROPERTY FOLDER Plugins)

# Install
if(B3D_GPU_BACKEND MATCHES "DirectX12" OR B3D_BUILD_ALL_PLUGINS)
	B3DSetUpPostBuildAndInstallSteps(bsfD3D12GpuBackend)
endif()

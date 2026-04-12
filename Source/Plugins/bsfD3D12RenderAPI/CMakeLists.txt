# Source files and their filters
B3DGlobSourceFiles(${CMAKE_CURRENT_SOURCE_DIR} "" "" BS_D3D12RENDERAPI_SRC)
B3DGlobSourceFiles(${CMAKE_CURRENT_SOURCE_DIR} "../Shared/" "" BS_D3D12RENDERAPI_SRC)

# Packages
if(B3D_RENDER_BACKEND MATCHES "DirectX12" OR B3D_BUILD_ALL_PLUGINS)
	find_package(glslang)
	find_package(SPIRVCross)

	if(WIN32)
		find_package(RenderDoc)
	endif()
endif()

# Target
B3DAddPlugin(bsfD3D12RenderAPI ENGINE ${BS_D3D12RENDERAPI_SRC})

# Common flags
B3DSetDefaultLinkAndCompileFlags(bsfD3D12RenderAPI)

# Includes
target_include_directories(bsfD3D12RenderAPI PRIVATE
	"./"
	"../Shared/"
)

# Definitions
target_compile_definitions(bsfD3D12RenderAPI PRIVATE
	-DB3D_D3D12_EXPORTS
)

# Libraries
## External libs: D3D12, DXGI, glslang, SPIR-V Cross

### Note: It's important to link glslang before D3D12, as it contains its own libraries. This ensures the search finds our local version first.
target_link_libraries(bsfD3D12RenderAPI PRIVATE ${glslang_LIBRARIES})
target_link_libraries(bsfD3D12RenderAPI PRIVATE ${SPIRVCross_LIBRARIES})

## Windows-specific D3D12 libraries
if(WIN32)
	target_link_libraries(bsfD3D12RenderAPI PRIVATE
		d3d12.lib
		dxgi.lib
		dxguid.lib
	)

	target_link_libraries(bsfD3D12RenderAPI PRIVATE ${RenderDoc_LIBRARIES})
endif()

## Local libs
target_link_libraries(bsfD3D12RenderAPI PRIVATE bsf)

# IDE specific
set_property(TARGET bsfD3D12RenderAPI PROPERTY FOLDER Plugins)

# Install
if(B3D_RENDER_BACKEND MATCHES "DirectX12" OR B3D_BUILD_ALL_PLUGINS)
	B3DSetUpPostBuildAndInstallSteps(bsfD3D12RenderAPI)
endif()

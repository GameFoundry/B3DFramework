# Source files and their filters
include(CMakeSources.cmake)
	
# Target
add_library(bsfD3D11RenderAPI SHARED ${BS_D3D11RENDERAPI_SRC})

# Common flags
add_common_flags(bsfD3D11RenderAPI)

# Includes
target_include_directories(bsfD3D11RenderAPI PRIVATE "./")

# Libraries
if(B3D_IS_64BIT)
	link_directories("$(DXSDK_DIR)Lib/x64")
else()
	link_directories("$(DXSDK_DIR)Lib/x86")
endif()
	
## External lib: DirectX 11
target_link_libraries(bsfD3D11RenderAPI d3d11 dxgi dxguid D3DCompiler)	
	
## Local libs
target_link_libraries(bsfD3D11RenderAPI bsf)

# IDE specific
set_property(TARGET bsfD3D11RenderAPI PROPERTY FOLDER Plugins)

# Install
if(B3D_RENDER_BACKEND MATCHES "DirectX 11" OR B3D_BUILD_ALL_RENDER_BACKENDS)
	install_bsf_target(bsfD3D11RenderAPI)
endif()

conditional_cotire(bsfD3D11RenderAPI)

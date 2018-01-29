# Source files and their filters
include(CMakeSources.cmake)

# Includes
set(BansheeD3D11RenderAPI_INC 
	"./" 
	"../BansheeUtility" 
	"../BansheeCore")

include_directories(${BansheeD3D11RenderAPI_INC})	
	
# Target
add_library(BansheeD3D11RenderAPI SHARED ${BS_BANSHEED3D11RENDERAPI_SRC})

# Defines
target_compile_definitions(BansheeD3D11RenderAPI PRIVATE -DBS_RSD3D11_EXPORTS)

# Libraries
if(BS_64BIT)
	link_directories("$(DXSDK_DIR)Lib/x64")
else()
	link_directories("$(DXSDK_DIR)Lib/x86")
endif()
	
## External lib: DirectX 11
target_link_libraries(BansheeD3D11RenderAPI d3d11 dxgi dxguid D3DCompiler)	
	
## Local libs
target_link_libraries(BansheeD3D11RenderAPI BansheeUtility BansheeCore)

# IDE specific
set_property(TARGET BansheeD3D11RenderAPI PROPERTY FOLDER Plugins)

# Install
if(RENDER_API_MODULE MATCHES "DirectX 11")
	install(
		TARGETS BansheeD3D11RenderAPI
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib
	)
endif()
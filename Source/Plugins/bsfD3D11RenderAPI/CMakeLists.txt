# Source files and their filters
include(CMakeSources.cmake)
	
# Target
add_library(bsfD3D11RenderAPI SHARED ${BS_D3D11RENDERAPI_SRC})

# Defines
target_compile_definitions(bsfD3D11RenderAPI PRIVATE -DBS_RSD3D11_EXPORTS)

# Includes
target_include_directories(bsfD3D11RenderAPI PRIVATE "./")

# Libraries
if(BS_64BIT)
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
if(RENDER_API_MODULE MATCHES "DirectX 11")
	install(
		TARGETS bsfD3D11RenderAPI
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib
	)
	
	if(WIN32)
		install(
			FILES $<TARGET_PDB_FILE:bsfD3D11RenderAPI> 
			DESTINATION bin 
			OPTIONAL
		)
	endif()
endif()
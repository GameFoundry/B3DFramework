# Source files and their filters
include(CMakeSources.cmake)

# Find packages
if(LINUX)
	find_package(OpenGL REQUIRED)
endif()
	
# Target
add_library(bsfGLRenderAPI SHARED ${BS_GLRENDERAPI_SRC})

# Includes
target_include_directories(bsfGLRenderAPI PRIVATE "./")

# Defines
target_compile_definitions(bsfGLRenderAPI PRIVATE -DBS_RSGL_EXPORTS -DGLEW_STATIC)

# Libraries
if(WIN32)
	## External lib: OpenGL
	target_link_libraries(bsfGLRenderAPI PRIVATE glu32 opengl32)
elseif(LINUX)
	## External lib: OpenGL
	target_link_libraries(bsfGLRenderAPI PRIVATE ${OPENGL_LIBRARIES})
elseif(APPLE) # MacOS
	target_link_framework(bsfGLRenderAPI Foundation)
	target_link_framework(bsfGLRenderAPI CoreGraphics)
	target_link_framework(bsfGLRenderAPI CoreVideo)
	target_link_framework(bsfGLRenderAPI OpenGL)
	target_link_framework(bsfGLRenderAPI IOKit)
	target_link_framework(bsfGLRenderAPI AppKit)
endif()

## Local libs
target_link_libraries(bsfGLRenderAPI PUBLIC bsf)

# IDE specific
set_property(TARGET bsfGLRenderAPI PROPERTY FOLDER Plugins)

# Install
if(RENDER_API_MODULE MATCHES "OpenGL")
	install(
		TARGETS bsfGLRenderAPI
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib
	)
	
	if(WIN32)
		install(
			FILES $<TARGET_PDB_FILE:bsfGLRenderAPI> 
			DESTINATION bin 
			OPTIONAL
		)
	endif()
endif()
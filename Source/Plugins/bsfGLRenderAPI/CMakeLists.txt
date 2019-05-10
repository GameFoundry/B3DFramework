# Source files and their filters
include(CMakeSources.cmake)

# Find packages
if(LINUX)
	find_package(OpenGL REQUIRED)
	find_package(X11)
endif()
	
# Target
add_library(bsfGLRenderAPI SHARED ${BS_GLRENDERAPI_SRC})

# Common flags
add_common_flags(bsfGLRenderAPI)

# Includes
target_include_directories(bsfGLRenderAPI PRIVATE
	"./"
	"../Shared/"
)

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
if(RENDER_API_MODULE MATCHES "OpenGL" OR BUILD_ALL_RENDER_API)
	install_bsf_target(bsfGLRenderAPI)
endif()

# Precompiled headers (Cotire)
set_directory_properties(PROPERTIES
    COTIRE_PREFIX_HEADER_IGNORE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../;${X11_INCLUDE_DIR}/X11;${OPENGL_INCLUDE_DIR}/GL")

conditional_cotire(bsfGLRenderAPI)

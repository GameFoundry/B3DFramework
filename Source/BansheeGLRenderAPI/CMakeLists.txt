# Source files and their filters
include(CMakeSources.cmake)

# Find packages
if(LINUX)
	find_package(OpenGL REQUIRED)
endif()

# Includes
set(BansheeGLRenderAPI_INC 
	"./" 
	"../BansheeUtility" 
	"../BansheeCore"
	"GLSL/include")

if(WIN32)
	set(BansheeGLRenderAPI_INC ${BansheeGLRenderAPI_INC} "Source/Win32")
elseif(LINUX)
	set(BansheeGLRenderAPI_INC ${BansheeGLRenderAPI_INC} ${OPENGL_INCLUDE_DIR})
endif()
	
include_directories(${BansheeGLRenderAPI_INC})	
	
# Target
add_library(BansheeGLRenderAPI SHARED ${BS_BANSHEEGLRENDERAPI_SRC})

# Defines
target_compile_definitions(BansheeGLRenderAPI PRIVATE -DBS_RSGL_EXPORTS -DGLEW_STATIC)

# Libraries
if(WIN32)
	## External lib: OpenGL
	target_link_libraries(BansheeGLRenderAPI PRIVATE glu32 opengl32)
elseif(LINUX)
	## External lib: OpenGL
	target_link_libraries(BansheeGLRenderAPI PRIVATE ${OPENGL_LIBRARIES})
elseif(APPLE) # MacOS
	target_link_framework(BansheeGLRenderAPI Foundation)
	target_link_framework(BansheeGLRenderAPI CoreGraphics)
	target_link_framework(BansheeGLRenderAPI CoreVideo)
	target_link_framework(BansheeGLRenderAPI OpenGL)
	target_link_framework(BansheeGLRenderAPI IOKit)
	target_link_framework(BansheeGLRenderAPI AppKit)
endif()

## Local libs
target_link_libraries(BansheeGLRenderAPI PUBLIC BansheeUtility BansheeCore)

# IDE specific
set_property(TARGET BansheeGLRenderAPI PROPERTY FOLDER Plugins)

# Install
if(RENDER_API_MODULE MATCHES "OpenGL")
	install(
		TARGETS BansheeGLRenderAPI
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib
	)
endif()
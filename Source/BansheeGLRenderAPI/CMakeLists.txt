# Source files and their filters
include(CMakeSources.cmake)

# Includes
set(BansheeGLRenderAPI_INC 
	"Include" 
	"../BansheeUtility/Include" 
	"../BansheeCore/Include"
	"Source/GLSL/include")

if(WIN32)
	set(BansheeGLRenderAPI_INC ${BansheeGLRenderAPI_INC} "Source/Win32")
else()
# TODO_OTHER_PLATFORMS_GO_HERE
endif()
	
include_directories(${BansheeGLRenderAPI_INC})	
	
# Target
add_library(BansheeGLRenderAPI SHARED ${BS_BANSHEEGLRENDERAPI_SRC})

# Defines
target_compile_definitions(BansheeGLRenderAPI PRIVATE -DBS_RSGL_EXPORTS -DGLEW_STATIC)

# Libraries
## External lib: OpenGL
target_link_libraries(BansheeGLRenderAPI glu32 opengl32)	

## Local libs
target_link_libraries(BansheeGLRenderAPI BansheeUtility BansheeCore)

# IDE specific
set_property(TARGET BansheeGLRenderAPI PROPERTY FOLDER Plugins)
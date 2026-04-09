# Source files and their filters
B3DGlobSourceFiles(${CMAKE_CURRENT_SOURCE_DIR} "" "" BS_NULLRENDERAPI_SRC)

# Target
add_library(bsfNullRenderAPI SHARED ${BS_NULLRENDERAPI_SRC})

# Common flags
B3DSetDefaultLinkAndCompileFlags(bsfNullRenderAPI)

# Includes
target_include_directories(bsfNullRenderAPI PRIVATE "./")
	
# Libraries
## Local libs
target_link_libraries(bsfNullRenderAPI PRIVATE bsf)

# IDE specific
set_property(TARGET bsfNullRenderAPI PROPERTY FOLDER Plugins)

# Install
if(B3D_RENDER_BACKEND MATCHES "Null" OR B3D_BUILD_ALL_PLUGINS)
	B3DSetUpPostBuildAndInstallSteps(bsfNullRenderAPI)
endif()

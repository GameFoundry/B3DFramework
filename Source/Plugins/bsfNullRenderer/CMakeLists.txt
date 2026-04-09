# Source files and their filters
B3DGlobSourceFiles(${CMAKE_CURRENT_SOURCE_DIR} "" "" BS_NULLRENDERER_SRC)

# Target
add_library(bsfNullRenderer SHARED ${BS_NULLRENDERER_SRC})

# Common flags
B3DSetDefaultLinkAndCompileFlags(bsfNullRenderer)

# Includes
target_include_directories(bsfNullRenderer PRIVATE "./")

# Libraries
## Local libs
target_link_libraries(bsfNullRenderer bsf)

# IDE specific
set_property(TARGET bsfNullRenderer PROPERTY FOLDER Plugins)

# Install
if(B3D_RENDERER MATCHES "Null" OR B3D_BUILD_ALL_PLUGINS)
	B3DSetUpPostBuildAndInstallSteps(bsfNullRenderer)
endif()

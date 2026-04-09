# Source files and their filters
B3DGlobSourceFiles(${CMAKE_CURRENT_SOURCE_DIR} "" "" BS_RENDERBEAST_SRC)

# Target
add_library(bsfRenderBeast SHARED ${BS_RENDERBEAST_SRC})

# Common flags
B3DSetDefaultLinkAndCompileFlags(bsfRenderBeast)

# Includes
target_include_directories(bsfRenderBeast PRIVATE "./")

# Libraries
## Local libs
target_link_libraries(bsfRenderBeast bsf)

# IDE specific
set_property(TARGET bsfRenderBeast PROPERTY FOLDER Plugins)

# Install
if(B3D_RENDERER MATCHES "RenderBeast" OR B3D_BUILD_ALL_PLUGINS)
	B3DSetUpPostBuildAndInstallSteps(bsfRenderBeast)
endif()

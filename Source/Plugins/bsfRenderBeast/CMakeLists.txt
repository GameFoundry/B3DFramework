# Source files and their filters
B3DGlobSourceFiles(${CMAKE_CURRENT_SOURCE_DIR} "" "" BS_RENDERBEAST_SRC)

# Target
B3DAddPlugin(bsfRenderBeast ENGINE ${BS_RENDERBEAST_SRC})

# Loading the plugin registers its renderer materials, which the shader cook needs
B3DRegisterShaderCookModule(bsfRenderBeast)

# Common flags
B3DSetDefaultLinkAndCompileFlags(bsfRenderBeast)

# Includes
target_include_directories(bsfRenderBeast PRIVATE "./")

# IDE specific
set_property(TARGET bsfRenderBeast PROPERTY FOLDER Plugins)

# Install
if(B3D_RENDERER MATCHES "RenderBeast" OR B3D_BUILD_ALL_PLUGINS)
	B3DSetUpPostBuildAndInstallSteps(bsfRenderBeast)
endif()

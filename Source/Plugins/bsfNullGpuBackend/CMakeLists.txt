# Source files and their filters
B3DGlobSourceFiles(${CMAKE_CURRENT_SOURCE_DIR} "" "" BS_NULLGPUBACKEND_SRC)

# Target
B3DAddPlugin(bsfNullGpuBackend ENGINE ${BS_NULLGPUBACKEND_SRC})
B3DRegisterGpuBackend(Null bsfNullGpuBackend nullsl)

# Common flags
B3DSetDefaultLinkAndCompileFlags(bsfNullGpuBackend)

# Includes
target_include_directories(bsfNullGpuBackend PRIVATE "./")

# IDE specific
set_property(TARGET bsfNullGpuBackend PROPERTY FOLDER Plugins)

# Install
if(B3D_GPU_BACKEND MATCHES "Null" OR B3D_BUILD_ALL_PLUGINS)
	B3DSetUpPostBuildAndInstallSteps(bsfNullGpuBackend)
endif()

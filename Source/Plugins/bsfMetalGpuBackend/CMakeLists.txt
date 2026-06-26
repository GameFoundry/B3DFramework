# Metal backend is only available on Apple platforms.
if(NOT APPLE)
	return()
endif()

# Source files and their filters
B3DGlobSourceFiles(${CMAKE_CURRENT_SOURCE_DIR} "" "" BS_METALGPUBACKEND_SRC)
B3DGlobSourceFiles(${CMAKE_CURRENT_SOURCE_DIR} "../Shared/" "" BS_METALGPUBACKEND_SRC)

# Target
B3DAddPlugin(bsfMetalGpuBackend ENGINE ${BS_METALGPUBACKEND_SRC})

# Common flags
B3DSetDefaultLinkAndCompileFlags(bsfMetalGpuBackend)

# Includes
target_include_directories(bsfMetalGpuBackend PRIVATE
	"./"
	"../Shared/"
)

# Libraries
## VKSL -> SPIR-V -> MSL compiler. Brings in glslang + SPIRV-Cross
target_link_libraries(bsfMetalGpuBackend PRIVATE bsfShaderBackendMSL)

# Apple frameworks required by the Metal backend
B3DTargetLinkFramework(bsfMetalGpuBackend Foundation)
B3DTargetLinkFramework(bsfMetalGpuBackend Metal)
B3DTargetLinkFramework(bsfMetalGpuBackend QuartzCore)
B3DTargetLinkFramework(bsfMetalGpuBackend CoreGraphics)
B3DTargetLinkFramework(bsfMetalGpuBackend CoreVideo)
B3DTargetLinkFramework(bsfMetalGpuBackend IOKit)
B3DTargetLinkFramework(bsfMetalGpuBackend AppKit)
B3DTargetLinkFramework(bsfMetalGpuBackend IOSurface)

# IDE specific
set_property(TARGET bsfMetalGpuBackend PROPERTY FOLDER Plugins)

# Install
if(B3D_GPU_BACKEND MATCHES "Metal" OR B3D_BUILD_ALL_PLUGINS)
	B3DSetUpPostBuildAndInstallSteps(bsfMetalGpuBackend)
endif()

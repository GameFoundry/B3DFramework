# Native-Metal shader backend: HLSLCrossCompiler emits MSL, then the bytecode compiler invokes Apple's tools to
# produce a metallib and reflects that library through Metal.
if(NOT APPLE)
	return()
endif()

# Source files and their filters
B3DGlobSourceFiles(${CMAKE_CURRENT_SOURCE_DIR} "" "" B3D_SHADERBACKENDMSL_SOURCE_FILES)

add_library(bsfShaderBackendMSL STATIC ${B3D_SHADERBACKENDMSL_SOURCE_FILES})

B3DSetDefaultLinkAndCompileFlags(bsfShaderBackendMSL)

target_include_directories(bsfShaderBackendMSL PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

# The vksl bytecode compiler provides the wrapped GLSLToSPIRV class and brings the engine headers, glslang and SPIRV-Cross.
target_link_libraries(bsfShaderBackendMSL PUBLIC bsfShaderBackendVKSL)

set_property(TARGET bsfShaderBackendMSL PROPERTY FOLDER Plugins)

# Register shader compiler target as "<language>|<targetLanguage>|<define>". Used to determine supported targets.
set_property(GLOBAL APPEND PROPERTY B3D_SHADER_CROSS_COMPILE_TARGETS "msl|MSL|METAL|KeepEntryNames")

B3DTargetLinkFramework(bsfShaderBackendMSL Foundation)
B3DTargetLinkFramework(bsfShaderBackendMSL Metal)

# Let anything using this shader backend know which libraries it needs to link
set_property(GLOBAL APPEND PROPERTY B3D_SHADER_BACKEND_LIBRARIES bsfShaderBackendMSL)

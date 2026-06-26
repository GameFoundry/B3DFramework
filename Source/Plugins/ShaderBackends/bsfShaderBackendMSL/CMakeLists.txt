# Device-independent native-Metal bytecode compiler: engine VKSL source -> argument-buffer Metal Shading Language
# (GLSL/VKSL -> SPIR-V via bsfShaderBackendVKSL's GLSLToSPIRV, then SPIR-V -> MSL via SPIRV-Cross).
if(NOT APPLE)
	return()
endif()

add_library(bsfShaderBackendMSL STATIC
	B3DBytecodeCompilerMSL.cpp)

B3DSetDefaultLinkAndCompileFlags(bsfShaderBackendMSL)

target_include_directories(bsfShaderBackendMSL PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

# The vksl bytecode compiler provides the wrapped GLSLToSPIRV class and brings the engine headers, glslang and SPIRV-Cross.
target_link_libraries(bsfShaderBackendMSL PUBLIC bsfShaderBackendVKSL)

set_property(TARGET bsfShaderBackendMSL PROPERTY FOLDER Plugins)

# Register shader compiler target as "<language>|<targetLanguage>|<define>". Used by bsfSL and cook tool to determine supported targets.
set_property(GLOBAL APPEND PROPERTY B3D_SHADER_CROSS_COMPILE_TARGETS "msl|VKSL450|METAL")

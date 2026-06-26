# Device-independent mvksl bytecode compiler: engine MVKSL / VKSL source -> MoltenVK MSL (GLSL/VKSL -> SPIR-V via
# bsfBytecodeVKSL's GLSLToSPIRV, then SPIR-V -> MSL via SPIRV-Cross).
# The "MVKSL" suffix matches the engine shading-language id "mvksl". MoltenVK / Vulkan-on-macOS only.
if(NOT APPLE)
	return()
endif()

add_library(bsfBytecodeMVKSL STATIC
	B3DBytecodeCompilerMVKSL.cpp)

B3DSetDefaultLinkAndCompileFlags(bsfBytecodeMVKSL)

# Public so consumers inherit the factory header
target_include_directories(bsfBytecodeMVKSL PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

# The vksl bytecode compiler provides the wrapped GLSLToSPIRV class and brings the engine headers, glslang and
# SPIRV-Cross (used directly here for the SPIR-V -> MSL step) transitively.
target_link_libraries(bsfBytecodeMVKSL PUBLIC bsfBytecodeVKSL)

set_property(TARGET bsfBytecodeMVKSL PROPERTY FOLDER Plugins)

# Register shader compiler target as "<language>|<targetLanguage>|<define>". Used by bsfSL and cook tool to determine supported targets.
set_property(GLOBAL APPEND PROPERTY B3D_SHADER_CROSS_COMPILE_TARGETS "mvksl|VKSL450|METAL")

# Device-independent hlsl bytecode compiler: engine HLSL source -> DXBC (via the OS-provided D3D shader compiler)
# with D3D reflection. Built as a static library so any consumer (the Direct3D GPU backends, the offline cook tool)
# can link it and inherit the CreateBytecodeCompilerhlsl() factory, with no DLL boundary. The "HLSL" suffix matches
# the engine shading-language id "hlsl" (first field of B3D_SHADER_CROSS_COMPILE_TARGETS).

# Source files and their filters
B3DGlobSourceFiles(${CMAKE_CURRENT_SOURCE_DIR} "" "" B3D_SHADERBACKENDHLSL_SOURCE_FILES)

add_library(bsfShaderBackendHLSL STATIC ${B3D_SHADERBACKENDHLSL_SOURCE_FILES})

B3DSetDefaultLinkAndCompileFlags(bsfShaderBackendHLSL)

# Public so consumers inherit the headers
target_include_directories(bsfShaderBackendHLSL PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

# Engine headers
if(B3D_MONOLITHIC_BUILD)
	target_link_libraries(bsfShaderBackendHLSL PUBLIC bsfHeaders)
else()
	target_link_libraries(bsfShaderBackendHLSL PUBLIC bsf)
endif()

# The D3D shader compiler (D3DCompile/D3DReflect), shipped with the OS
target_link_libraries(bsfShaderBackendHLSL PUBLIC d3dcompiler.lib)

set_property(TARGET bsfShaderBackendHLSL PROPERTY FOLDER Plugins)

# Register shader compiler target as "<language>|<targetLanguage>|<define>|KeepEntryNames". Used to determine supported targets.
set_property(GLOBAL APPEND PROPERTY B3D_SHADER_CROSS_COMPILE_TARGETS "hlsl|HLSL5|DIRECTX|KeepEntryNames")

# Anything that needs shader compilation will use this property to find libraries it needs to link
set_property(GLOBAL APPEND PROPERTY B3D_SHADER_BACKEND_LIBRARIES bsfShaderBackendHLSL)

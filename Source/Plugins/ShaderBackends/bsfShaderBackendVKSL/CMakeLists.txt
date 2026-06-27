# Device-independent vksl bytecode compiler: engine VKSL / GLSL source -> SPIR-V (via glslang) with SPIRV-Cross
# reflection. Built as a static library so any consumer (the Vulkan / Metal GPU backends, the offline cook tool) can
# link it and inherit its third-party dependencies plus the CreateBytecodeCompilervksl() factory, with no DLL boundary.
# The "VKSL" suffix matches the engine shading-language id "vksl" (first field of B3D_SHADER_CROSS_COMPILE_TARGETS).

# Ensure pre-built binary dependencies are up to date
if(B3D_USE_BUNDLED_LIBRARIES)
	B3DCheckAndUpdatePrebuiltDependency(glslang)
	B3DCheckAndUpdatePrebuiltDependency(SPIRVCross)
endif()

find_package(glslang)
find_package(SPIRVCross)

# Source files and their filters
B3DGlobSourceFiles(${CMAKE_CURRENT_SOURCE_DIR} "" "" B3D_SHADERBACKENDVKSL_SOURCE_FILES)

add_library(bsfShaderBackendVKSL STATIC ${B3D_SHADERBACKENDVKSL_SOURCE_FILES})

B3DSetDefaultLinkAndCompileFlags(bsfShaderBackendVKSL)

# Public so consumers inherit the headers
target_include_directories(bsfShaderBackendVKSL PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

# Engine headers
if(B3D_MONOLITHIC_BUILD)
	target_link_libraries(bsfShaderBackendVKSL PUBLIC bsfHeaders)
else()
	target_link_libraries(bsfShaderBackendVKSL PUBLIC bsf)
endif()

# Third-party dependencies (glslang, SPIR-V Cross)
target_link_libraries(bsfShaderBackendVKSL PUBLIC ${glslang_LIBRARIES} ${SPIRVCross_LIBRARIES})

set_property(TARGET bsfShaderBackendVKSL PROPERTY FOLDER Plugins)

# Register shader compiler target as "<language>|<targetLanguage>|<define>". Used to determine supported targets.
set_property(GLOBAL APPEND PROPERTY B3D_SHADER_CROSS_COMPILE_TARGETS "vksl|VKSL450|VULKAN")

# Let anything using this shader backend know which libraries it needs to link
set_property(GLOBAL APPEND PROPERTY B3D_SHADER_BACKEND_LIBRARIES bsfShaderBackendVKSL)

# Offline asset cook tool. Cooks shaders and likely to do other access processing in the future.

add_executable(BansheeCookTool
	B3DBansheeCookTool.cpp
	B3DShaderCooker.cpp
	B3DShaderCooker.h
	B3DBuiltinShaderCookerSource.cpp
	B3DBuiltinShaderCookerSource.h)
B3DSetDefaultLinkAndCompileFlags(BansheeCookTool)

target_link_libraries(BansheeCookTool bsf)

B3DAddRuntimeDependencies(BansheeCookTool)

# The cook runs deviceless on the Null GPU + Null renderer, but loads bsfRenderBeast (without activating it) so we can query its renderer materials.
add_dependencies(BansheeCookTool bsfSL bsfNullGpuBackend bsfNullRenderer bsfRenderBeast)

set_property(TARGET BansheeCookTool PROPERTY FOLDER Tools)

if(NOT B3D_IS_ENGINE)
	install(TARGETS BansheeCookTool RUNTIME DESTINATION bin)
endif()

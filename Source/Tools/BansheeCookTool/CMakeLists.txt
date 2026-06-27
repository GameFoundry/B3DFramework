# Offline asset cook tool. Cooks shaders and likely to do other access processing in the future.

add_executable(BansheeCookTool
	B3DBansheeCookTool.cpp)
B3DSetDefaultLinkAndCompileFlags(BansheeCookTool)

target_link_libraries(BansheeCookTool bsf)

B3DAddRuntimeDependencies(BansheeCookTool)
add_dependencies(BansheeCookTool bsfSL bsfNullGpuBackend bsfNullRenderer)

set_property(TARGET BansheeCookTool PROPERTY FOLDER Tools)

if(NOT B3D_IS_ENGINE)
	install(TARGETS BansheeCookTool RUNTIME DESTINATION bin)
endif()

# Source files and their filters
B3DGlobSourceFiles(${CMAKE_CURRENT_SOURCE_DIR} "" "" BS_MONO_SRC)

# Find packages
find_package(DotNETCoreMono)

# Target
add_library(bsfMono STATIC ${BS_MONO_SRC})

# Common flags
B3DSetDefaultLinkAndCompileFlags(bsfMono)

# Includes
target_include_directories(bsfMono PUBLIC "./")

# Defines
target_compile_definitions(bsfMono PUBLIC -DBS_MONO_STATIC_LIB)

# We use Debug coreclr in Debug configuration only
target_compile_definitions(bsfMono PRIVATE $<$<CONFIG:Debug>:B3D_MONO_CHECKED_RUNTIME=1>)

# Libraries
## External lib: Mono
target_link_libraries(bsfMono PRIVATE ${DotNETCoreMono_LIBRARIES})

## Local libs
target_link_libraries(bsfMono PUBLIC bsf)
B3DSetUpPostBuildAndInstallSteps(bsfMono)

# Runtime executable
if(B3D_IS_ENGINE)
	# Target for running generic Mono code
	add_executable(MonoExec B3DMonoExec.cpp)
	target_link_libraries(MonoExec PRIVATE bsfMono)

	target_link_libraries(MonoExec PRIVATE ${DotNETCoreMono_LIBRARIES})

	B3DSetUpPostBuildAndInstallSteps(MonoExec)
endif()

# IDE specific
set_property(TARGET bsfMono PROPERTY FOLDER Script)

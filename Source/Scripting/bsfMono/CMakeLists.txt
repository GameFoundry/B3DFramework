# Source files and their filters
B3DGlobSourceFiles(${CMAKE_CURRENT_SOURCE_DIR} "" "" BS_MONO_SRC)

# Ensure pre-built binary dependencies are up to date
if(B3D_USE_BUNDLED_LIBRARIES AND NOT B3D_USE_DOTNETCORE)
	set(BS_MONO_PREBUILT_DEPENDENCY_VERSION 2) # Tick this when you change the binaries
	check_and_update_binary_deps(bsfDep_mono mono ${BSF_SOURCE_DIR}/../Dependencies/mono ${BS_MONO_PREBUILT_DEPENDENCY_VERSION})
endif()

# Find packages
if(B3D_USE_DOTNETCORE)
	find_package(DotNETCoreMono)
else()
	find_package(mono)
endif()

# Target
add_library(bsfMono STATIC ${BS_MONO_SRC})

# Common flags
add_common_flags(bsfMono)

# Includes
target_include_directories(bsfMono PUBLIC "./")

# Defines
target_compile_definitions(bsfMono PUBLIC -DBS_MONO_STATIC_LIB)

# Libraries
## External lib: Mono
if(B3D_USE_DOTNETCORE)
	target_link_libraries(bsfMono PUBLIC ${DotNETCoreMono_LIBRARIES})
else()
	target_link_libraries(bsfMono PUBLIC ${mono_LIBRARIES})
endif()

## Local libs
target_link_libraries(bsfMono PUBLIC bsf)
B3DSetUpPostBuildAndInstallSteps(bsfMono)

# Runtime executable
if(B3D_IS_ENGINE)
	# Target for running generic Mono code
	add_executable(MonoExec BsMonoExec.cpp)

	if(B3D_USE_DOTNETCORE)
		target_link_libraries(MonoExec ${DotNETCoreMono_LIBRARIES})
	else()
		target_link_libraries(MonoExec ${mono_LIBRARIES})
	endif()

	B3DSetUpPostBuildAndInstallSteps(MonoExec)
endif()

# IDE specific
set_property(TARGET bsfMono PROPERTY FOLDER Script)

conditional_cotire(bsfMono)

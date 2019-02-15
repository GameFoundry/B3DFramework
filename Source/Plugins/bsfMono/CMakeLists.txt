# Source files and their filters
include(CMakeSources.cmake)

# Ensure pre-built binary dependencies are up to date
if(USE_BUNDLED_LIBRARIES)
	set(BS_MONO_PREBUILT_DEPENDENCY_VERSION 1) # Tick this when you change the binaries
	check_and_update_binary_deps(bsfDep_mono mono ${BSF_SOURCE_DIR}/../Dependencies/mono ${BS_MONO_PREBUILT_DEPENDENCY_VERSION})
endif()

# Find packages
find_package(mono)

# Target
add_library(bsfMono OBJECT ${BS_MONO_SRC})

# Common flags
add_common_flags(bsfMono)

# Includes
target_include_directories(bsfMono PUBLIC "./")

# Defines
target_compile_definitions(bsfMono PUBLIC -DBS_MONO_STATIC_LIB)

# Libraries
## External lib: Mono
target_link_libraries(bsfMono PUBLIC ${mono_LIBRARIES})

## Local libs
target_link_libraries(bsfMono PUBLIC bsf)

# Runtime executable
if(BS_IS_BANSHEE3D)
	# Target for running generic Mono code
	add_executable(MonoExec BsMonoGenericExec.cpp)
	target_link_libraries(MonoExec ${mono_LIBRARIES})
endif()

# IDE specific
set_property(TARGET bsfMono PROPERTY FOLDER Script)

conditional_cotire(bsfMono)

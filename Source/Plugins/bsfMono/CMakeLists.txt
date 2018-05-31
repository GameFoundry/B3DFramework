# Source files and their filters
include(CMakeSources.cmake)

# Find packages
find_package(mono)

# Target
add_library(bsfMono SHARED ${BS_MONO_SRC})

# Includes
target_include_directories(bsfMono PUBLIC "./")

# Defines
target_compile_definitions(bsfMono PRIVATE -DBS_MONO_EXPORTS)

# Libraries
## External lib: Mono
target_link_libraries(bsfMono PRIVATE ${mono_LIBRARIES})

## Local libs
target_link_libraries(bsfMono PUBLIC bsf)

# Runtime executable
add_executable(MonoExec BsMonoExec.cpp)
target_link_libraries(MonoExec ${mono_LIBRARIES})

# IDE specific
set_property(TARGET bsfMono PROPERTY FOLDER Plugins)

conditional_cotire(bsfMono)

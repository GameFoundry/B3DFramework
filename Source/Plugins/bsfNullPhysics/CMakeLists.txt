# Source files and their filters
include(CMakeSources.cmake)

# Target
add_library(bsfNullPhysics SHARED ${BS_NULL_PHYSICS_SRC})

# Common flags
add_common_flags(bsfNullPhysics)

# Includes
target_include_directories(bsfNullPhysics PRIVATE "./")

# Libraries
## Local libs
target_link_libraries(bsfNullPhysics PRIVATE bsf)

# IDE specific
set_property(TARGET bsfNullPhysics PROPERTY FOLDER Plugins)

# Install
if(B3D_PHYSICS_BACKEND MATCHES "Null")
	install_bsf_target(bsfNullPhysics)
endif()

conditional_cotire(bsfNullPhysics)

# Source files and their filters
B3DGlobSourceFiles(${CMAKE_CURRENT_SOURCE_DIR} "" "" BS_NULL_PHYSICS_SRC)

# Target
add_library(bsfNullPhysics SHARED ${BS_NULL_PHYSICS_SRC})

# Common flags
B3DSetDefaultLinkAndCompileFlags(bsfNullPhysics)

# Includes
target_include_directories(bsfNullPhysics PRIVATE "./")

# Libraries
## Local libs
target_link_libraries(bsfNullPhysics PRIVATE bsf)

# IDE specific
set_property(TARGET bsfNullPhysics PROPERTY FOLDER Plugins)

# Install
if(B3D_PHYSICS_BACKEND MATCHES "Null" OR B3D_BUILD_ALL_PLUGINS)
	B3DSetUpPostBuildAndInstallSteps(bsfNullPhysics)
endif()

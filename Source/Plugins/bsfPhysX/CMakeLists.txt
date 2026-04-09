# Source files and their filters
B3DGlobSourceFiles(${CMAKE_CURRENT_SOURCE_DIR} "" "" BS_PHYSX_SRC)

# Packages
if(B3D_PHYSICS_BACKEND MATCHES "PhysX" OR B3D_BUILD_ALL_PLUGINS)
	# Ensure pre-built binary dependencies are up to date
	if(B3D_USE_BUNDLED_LIBRARIES)
		B3DCheckAndUpdatePrebuiltDependency(PhysX)
	endif()

	find_package(PhysX REQUIRED)
endif()
	
# Target
add_library(bsfPhysX SHARED ${BS_PHYSX_SRC})

# Common flags
B3DSetDefaultLinkAndCompileFlags(bsfPhysX)

# Includes
target_include_directories(bsfPhysX PRIVATE "./")

# Defines
target_compile_definitions(bsfPhysX PRIVATE $<$<CONFIG:Debug>:_DEBUG> $<$<CONFIG:RelWithDebInfo>:NDEBUG> $<$<CONFIG:MINSIZEREL>:NDEBUG>
		$<$<CONFIG:Release>:NDEBUG>)

# Libraries
## External lib: PhysX
target_link_libraries(bsfPhysX PRIVATE ${PhysX_LIBRARIES})

## Local libs
target_link_libraries(bsfPhysX PRIVATE bsf)

# IDE specific
set_property(TARGET bsfPhysX PROPERTY FOLDER Plugins)

# Install
if(B3D_PHYSICS_BACKEND MATCHES "PhysX" OR B3D_BUILD_ALL_PLUGINS)
	B3DSetUpPostBuildAndInstallSteps(bsfPhysX)
endif()

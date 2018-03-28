# Source files and their filters
include(CMakeSources.cmake)

# Packages
find_package(PhysX REQUIRED)
	
# Target
add_library(bsfPhysX SHARED ${BS_PHYSX_SRC})

# Includes
target_include_directories(bsfPhysX PRIVATE "./")

# Defines
target_compile_definitions(bsfPhysX PRIVATE -DBS_PHYSX_EXPORTS)
target_compile_definitions(bsfPhysX PRIVATE $<$<CONFIG:Debug>:_DEBUG> $<$<CONFIG:OptimizedDebug>:NDEBUG>
		$<$<CONFIG:Release>:NDEBUG>)

# Libraries
## External lib: PhysX
target_link_libraries(bsfPhysX PRIVATE ${PhysX_LIBRARIES})

## Local libs
target_link_libraries(bsfPhysX PRIVATE bsf)

# IDE specific
set_property(TARGET bsfPhysX PROPERTY FOLDER Plugins)

# Install
install(
	TARGETS bsfPhysX
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)

if(WIN32)
	install(
		FILES $<TARGET_PDB_FILE:bsfPhysX> 
		DESTINATION bin 
		OPTIONAL
	)
endif()
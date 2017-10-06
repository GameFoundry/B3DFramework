# Source files and their filters
include(CMakeSources.cmake)

# Packages
find_package(PhysX REQUIRED)

# Includes
set(BansheePhysX_INC 
	"./"
	"../BansheeUtility"
	"../BansheeCore")

include_directories(${BansheePhysX_INC})
	
# Target
add_library(BansheePhysX SHARED ${BS_BANSHEEPHYSX_SRC})

# Defines
target_compile_definitions(BansheePhysX PRIVATE -DBS_PHYSX_EXPORTS)
target_compile_definitions(BansheePhysX PRIVATE $<$<CONFIG:Debug>:_DEBUG> $<$<CONFIG:OptimizedDebug>:NDEBUG>
		$<$<CONFIG:Release>:NDEBUG>)

# Libraries
## External lib: PhysX
target_link_libraries(BansheePhysX PRIVATE ${PhysX_LIBRARIES})

## Local libs
target_link_libraries(BansheePhysX PRIVATE BansheeUtility BansheeCore)

# IDE specific
set_property(TARGET BansheePhysX PROPERTY FOLDER Plugins)
# Source files and their filters
include(CMakeSources.cmake)

# Packages
find_package(FMOD)

# Includes
set(BansheeFMOD_INC 
	"Include" 
	"../BansheeUtility/Include" 
	"../BansheeCore/Include"
	${FMOD_INCLUDE_DIRS})

include_directories(${BansheeFMOD_INC})	
	
# Target
add_library(BansheeFMOD SHARED ${BS_BANSHEEFMOD_SRC})

# Defines
target_compile_definitions(BansheeFMOD PRIVATE -DBS_FMOD_EXPORTS)

# Libraries
## External libs: FMOD
add_library(FMOD STATIC IMPORTED)

set_target_properties(FMOD PROPERTIES IMPORTED_LOCATION_DEBUG "${FMOD_LIBRARIES_DEBUG}")
set_target_properties(FMOD PROPERTIES IMPORTED_LOCATION_OPTIMIZEDDEBUG "${FMOD_LIBRARIES_DEBUG}")
set_target_properties(FMOD PROPERTIES IMPORTED_LOCATION_RELEASE "${FMOD_LIBRARIES_OPTIMIZED}")	

target_link_libraries(BansheeFMOD PRIVATE FMOD)	

## Local libs
target_link_libraries(BansheeFMOD PUBLIC BansheeUtility BansheeCore)

# IDE specific
set_property(TARGET BansheeFMOD PROPERTY FOLDER Plugins)
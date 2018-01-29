# Source files and their filters
include(CMakeSources.cmake)

# Packages
find_package(FBXSDK REQUIRED)

# Includes
set(BansheeFBXImporter_INC 
	"./" 
	"../BansheeUtility" 
	"../BansheeCore")

include_directories(${BansheeFBXImporter_INC})	
	
# Target
add_library(BansheeFBXImporter SHARED ${BS_BANSHEEFBXIMPORTER_SRC})

# Defines
target_compile_definitions(BansheeFBXImporter PRIVATE -DBS_FBX_EXPORTS)

# Libraries
## External lib: FBX
target_link_libraries(BansheeFBXImporter ${FBXSDK_LIBRARIES})

## Local libs
target_link_libraries(BansheeFBXImporter BansheeUtility BansheeCore)

# IDE specific
set_property(TARGET BansheeFBXImporter PROPERTY FOLDER Plugins)

# Install
install(
	TARGETS BansheeFBXImporter
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)
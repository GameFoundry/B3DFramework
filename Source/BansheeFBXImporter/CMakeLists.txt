# Source files and their filters
include(CMakeSources.cmake)

# Includes
set(BansheeFBXImporter_INC 
	"Include" 
	"../BansheeUtility/Include" 
	"../BansheeCore/Include"
	"../../Dependencies/FBXSDK/include")

include_directories(${BansheeFBXImporter_INC})	
	
# Target
add_library(BansheeFBXImporter SHARED ${BS_BANSHEEFBXIMPORTER_SRC})

# Defines
target_compile_definitions(BansheeFBXImporter PRIVATE -DBS_FBX_EXPORTS)

# Libraries
## External lib: FBX
add_library_per_config(BansheeFBXImporter FBXSDK Release/libfbxsdk-md Debug/libfbxsdk-md)	

## Local libs
target_link_libraries(BansheeFBXImporter PUBLIC BansheeUtility BansheeCore)

# IDE specific
set_property(TARGET BansheeFBXImporter PROPERTY FOLDER Plugins)
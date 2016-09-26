# Source files and their filters
include(CMakeSources.cmake)

# Includes
set(BansheeFreeImgImporter_INC 
	"Include" 
	"../BansheeUtility/Include" 
	"../BansheeCore/Include"
	"../../Dependencies/freeimg/include")

include_directories(${BansheeFreeImgImporter_INC})	
	
# Target
add_library(BansheeFreeImgImporter SHARED ${BS_BANSHEEFREEIMGIMPORTER_SRC})

# Defines
target_compile_definitions(BansheeFreeImgImporter PRIVATE -DBS_FREEIMG_EXPORTS -D_MBCS -DFREEIMAGE_LIB)

# Libraries
## External lib: FreeImage
add_library_per_config(BansheeFreeImgImporter freeimg Release/FreeImage Debug/FreeImage)

## Local libs
target_link_libraries(BansheeFreeImgImporter PUBLIC BansheeUtility BansheeCore)

# IDE specific
set_property(TARGET BansheeFreeImgImporter PROPERTY FOLDER Plugins)
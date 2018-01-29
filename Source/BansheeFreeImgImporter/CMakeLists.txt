# Source files and their filters
include(CMakeSources.cmake)

# Packages
find_package(freeimg REQUIRED)

# Includes
set(BansheeFreeImgImporter_INC 
	"./" 
	"../BansheeUtility" 
	"../BansheeCore"
	"../BansheeEngine")

include_directories(${BansheeFreeImgImporter_INC})	
	
# Target
add_library(BansheeFreeImgImporter SHARED ${BS_BANSHEEFREEIMGIMPORTER_SRC})

# Defines
target_compile_definitions(BansheeFreeImgImporter PRIVATE -DBS_FREEIMG_EXPORTS -D_MBCS -DFREEIMAGE_LIB)

# Libraries
## External lib: FreeImage
target_link_libraries(BansheeFreeImgImporter ${freeimg_LIBRARIES})

## Local libs
target_link_libraries(BansheeFreeImgImporter BansheeUtility BansheeCore BansheeEngine)

# IDE specific
set_property(TARGET BansheeFreeImgImporter PROPERTY FOLDER Plugins)

# Install
install(
	TARGETS BansheeFreeImgImporter
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)
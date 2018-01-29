# Source files and their filters
include(CMakeSources.cmake)

# Packages
find_package(freetype REQUIRED)

# Includes
set(BansheeFontImporter_INC 
	"./" 
	"../BansheeUtility" 
	"../BansheeCore")

include_directories(${BansheeFontImporter_INC})	
	
# Target
add_library(BansheeFontImporter SHARED ${BS_BANSHEEFONTIMPORTER_SRC})

# Defines
target_compile_definitions(BansheeFontImporter PRIVATE -DBS_FONT_EXPORTS -DUSE_FREETYPE2_STATIC)

# Libraries
## External lib: FreeType
target_link_libraries(BansheeFontImporter ${freetype_LIBRARIES})

## Local libs
target_link_libraries(BansheeFontImporter BansheeUtility BansheeCore)

# IDE specific
set_property(TARGET BansheeFontImporter PROPERTY FOLDER Plugins)

# Install
install(
	TARGETS BansheeFontImporter
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)
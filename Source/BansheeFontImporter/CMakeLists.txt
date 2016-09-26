# Source files and their filters
include(CMakeSources.cmake)

# Includes
set(BansheeFontImporter_INC 
	"Include" 
	"../BansheeUtility/Include" 
	"../BansheeCore/Include"
	"../../Dependencies/freetype/include")

include_directories(${BansheeFontImporter_INC})	
	
# Target
add_library(BansheeFontImporter SHARED ${BS_BANSHEEFONTIMPORTER_SRC})

# Defines
target_compile_definitions(BansheeFontImporter PRIVATE -DBS_FONT_EXPORTS -DUSE_FREETYPE2_STATIC)

# Libraries
## External lib: FreeType
add_library_per_config(BansheeFontImporter freetype Release/freetype Debug/freetype)

## Local libs
target_link_libraries(BansheeFontImporter PUBLIC BansheeUtility BansheeCore)

# IDE specific
set_property(TARGET BansheeFontImporter PROPERTY FOLDER Plugins)
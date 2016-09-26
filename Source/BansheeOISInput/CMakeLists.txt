# Source files and their filters
include(CMakeSources.cmake)

# Includes
set(BansheeOISInput_INC 
	"Include"
	"../BansheeUtility/Include" 
	"../BansheeCore/Include"
	"../../Dependencies/OIS/include")

include_directories(${BansheeOISInput_INC})	
	
# Target
add_library(BansheeOISInput SHARED ${BS_BANSHEEOISINPUT_SRC})

# Defines
target_compile_definitions(BansheeOISInput PRIVATE -DBS_OIS_EXPORTS -DOIS_DYNAMIC_LIB -DOIS_NONCLIENT_BUILD)

# Libraries
## External lib: OIS
add_library_per_config(BansheeOISInput OIS Release/BansheeOIS Debug/BansheeOIS)

## Local libs
target_link_libraries(BansheeOISInput PUBLIC BansheeUtility BansheeCore)

# IDE specific
set_property(TARGET BansheeOISInput PROPERTY FOLDER Plugins)
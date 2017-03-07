# Source files and their filters
include(CMakeSources.cmake)

# Includes
set(BansheeEngine_INC 
	"Include" 
	"../BansheeUtility/Include" 
	"../BansheeCore/Include")

include_directories(${BansheeEngine_INC})	

# Target
add_library(BansheeEngine SHARED ${BS_BANSHEEENGINE_SRC})

# Defines
target_compile_definitions(BansheeEngine PRIVATE 
	-DBS_EXPORTS
	$<$<CONFIG:Debug>:BS_CONFIG=BS_CONFIG_DEBUG>
	$<$<CONFIG:OptimizedDebug>:BS_CONFIG=BS_CONFIG_OPTIMIZEDDEBUG>
	$<$<CONFIG:Release>:BS_CONFIG=BS_CONFIG_RELEASE>)

# Libraries
## Local libs
target_link_libraries(BansheeEngine BansheeUtility BansheeCore)	

# IDE specific
set_property(TARGET BansheeEngine PROPERTY FOLDER Layers)
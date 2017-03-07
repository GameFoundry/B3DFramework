# Source files and their filters
include(CMakeSources.cmake)

# Includes
set(BansheeCore_INC 
	"Include" 
	"../BansheeUtility/Include" 
	"../../Dependencies/nvtt/include")

if(WIN32)
	set(BansheeCore_INC ${BansheeCore_INC} "Include/Win32")
endif()

include_directories(${BansheeCore_INC})	

# Target
add_library(BansheeCore SHARED ${BS_BANSHEECORE_SRC})

# Defines
target_compile_definitions(BansheeCore PRIVATE -DBS_CORE_EXPORTS)

# Libraries
## External lib: NVTT
add_library_per_config(BansheeCore nvtt Release/nvtt Debug/nvtt)

## Local libs
target_link_libraries(BansheeCore PUBLIC BansheeUtility)	
	
## OS libs
if(WIN32)
target_link_libraries(BansheeCore PRIVATE Winmm)
else()
	# TODO_OTHER_PLATFORMS_GO_HERE
endif()

# IDE specific
set_property(TARGET BansheeCore PROPERTY FOLDER Layers)
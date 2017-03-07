# Source files and their filters
include(CMakeSources.cmake)

# Includes
set(BansheeEditor_INC 
	"Include" 
	"../BansheeUtility/Include" 
	"../BansheeCore/Include"
	"../BansheeEngine/Include")

include_directories(${BansheeEditor_INC})	

# Target
add_library(BansheeEditor SHARED ${BS_BANSHEEEDITOR_SRC})

# Defines
target_compile_definitions(BansheeEditor PRIVATE -DBS_ED_EXPORTS)

# Libraries
## Local libs
target_link_libraries(BansheeEditor BansheeUtility BansheeCore BansheeEngine)	
	
## OS libs
if(WIN32)
target_link_libraries(BansheeEditor Advapi32)
else()
	# TODO_OTHER_PLATFORMS_GO_HERE
endif()

# IDE specific
set_property(TARGET BansheeEditor PROPERTY FOLDER Layers)
# Source files and their filters
include(CMakeSources.cmake)

# Includes
set(ExampleGettingStarted_INC 
	"Include"
	"../../BansheeUtility/Include" 
	"../../BansheeCore/Include"
	"../../BansheeEngine/Include")

include_directories(${ExampleGettingStarted_INC})	
	
# Target
if(WIN32)
	add_executable(ExampleGettingStarted WIN32 ${BS_EXAMPLEPROJECT_SRC})
else()
	add_executable(ExampleGettingStarted ${BS_EXAMPLEPROJECT_SRC})
endif()
	
# Libraries
## Local libs
target_link_libraries(ExampleGettingStarted BansheeEngine BansheeUtility BansheeCore)

# IDE specific
set_property(TARGET ExampleGettingStarted PROPERTY FOLDER Examples)

# Plugin dependencies
add_engine_dependencies(ExampleGettingStarted)
add_dependencies(ExampleGettingStarted BansheeFBXImporter BansheeFontImporter BansheeFreeImgImporter)

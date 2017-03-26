# Source files and their filters
include(CMakeSources.cmake)

# Includes
set(ExamplePhysicallyBasedShading_INC 
	"Include"
	"../../BansheeUtility/Include" 
	"../../BansheeCore/Include"
	"../../BansheeEngine/Include")

include_directories(${ExamplePhysicallyBasedShading_INC})	
	
# Target
if(WIN32)
	add_executable(ExamplePhysicallyBasedShading WIN32 ${BS_EXAMPLEPHYSICALLYBASEDSHADING_SRC})
else()
	add_executable(ExamplePhysicallyBasedShading ${BS_EXAMPLEPHYSICALLYBASEDSHADING_SRC})
endif()
	
# Libraries
## Local libs
target_link_libraries(ExamplePhysicallyBasedShading BansheeEngine BansheeUtility BansheeCore)

# IDE specific
set_property(TARGET ExamplePhysicallyBasedShading PROPERTY FOLDER Examples)

# Plugin dependencies
add_engine_dependencies(ExamplePhysicallyBasedShading)
add_dependencies(ExamplePhysicallyBasedShading BansheeFBXImporter BansheeFontImporter BansheeFreeImgImporter)

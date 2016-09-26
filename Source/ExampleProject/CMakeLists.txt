# Source files and their filters
include(CMakeSources.cmake)

# Includes
set(ExampleProject_INC 
	"Include"
	"../BansheeUtility/Include" 
	"../BansheeCore/Include"
	"../BansheeEngine/Include")

include_directories(${ExampleProject_INC})	
	
# Target
if(WIN32)
	add_executable(ExampleProject WIN32 ${BS_EXAMPLEPROJECT_SRC})
else()
	add_executable(ExampleProject ${BS_EXAMPLEPROJECT_SRC})
endif()
	
# Libraries
## Local libs
target_link_libraries(ExampleProject BansheeEngine BansheeUtility BansheeCore)

# IDE specific
set_property(TARGET ExampleProject PROPERTY FOLDER Executable)

# Plugin dependencies
add_engine_dependencies(ExampleProject)
add_dependencies(ExampleProject BansheeFBXImporter BansheeFontImporter BansheeFreeImgImporter)

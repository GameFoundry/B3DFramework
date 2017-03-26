# Source files and their filters
include(CMakeSources.cmake)

# Includes
set(ExampleLLR_INC 
	"Include"
	"../../BansheeUtility/Include" 
	"../../BansheeCore/Include"
	"../../BansheeEngine/Include")

include_directories(${ExampleLLR_INC})	
	
# Target
if(WIN32)
	add_executable(ExampleLowLevelRendering WIN32 ${BS_EXAMPLELLR_SRC})
else()
	add_executable(ExampleLowLevelRendering ${BS_EXAMPLELLR_SRC})
endif()
	
# Libraries
## Local libs
target_link_libraries(ExampleLowLevelRendering BansheeEngine BansheeUtility BansheeCore)

# IDE specific
set_property(TARGET ExampleLowLevelRendering PROPERTY FOLDER Examples)

# Dependencies
add_engine_dependencies(ExampleLowLevelRendering)
add_dependencies(ExampleLowLevelRendering BansheeFBXImporter BansheeFontImporter BansheeFreeImgImporter)

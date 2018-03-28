# Source files and their filters
include(CMakeSources.cmake)

# Packages
find_package(FBXSDK REQUIRED)
	
# Target
add_library(bsfFBXImporter SHARED ${BS_FBXIMPORTER_SRC})

# Includes
target_include_directories(bsfFBXImporter PRIVATE "./")

# Defines
target_compile_definitions(bsfFBXImporter PRIVATE -DBS_FBX_EXPORTS)

# Libraries
## External lib: FBX
target_link_libraries(bsfFBXImporter ${FBXSDK_LIBRARIES})

## Local libs
target_link_libraries(bsfFBXImporter bsf)

# IDE specific
set_property(TARGET bsfFBXImporter PROPERTY FOLDER Plugins)

# Install
install(
	TARGETS bsfFBXImporter
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)

if(WIN32)
	install(
		FILES $<TARGET_PDB_FILE:bsfFBXImporter> 
		DESTINATION bin 
		OPTIONAL
	)
endif()
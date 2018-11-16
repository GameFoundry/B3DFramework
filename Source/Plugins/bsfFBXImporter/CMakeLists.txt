# Source files and their filters
include(CMakeSources.cmake)

# Packages
find_package(FBXSDK REQUIRED)
	
# Target
add_library(bsfFBXImporter SHARED ${BS_FBXIMPORTER_SRC})

# Common flags
add_common_flags(bsfFBXImporter)

# Includes
target_include_directories(bsfFBXImporter PRIVATE "./")

# Defines
target_compile_definitions(bsfFBXImporter PRIVATE -DBS_FBX_EXPORTS)

# Libraries
## External lib: FBX
target_link_libraries(bsfFBXImporter PRIVATE ${FBXSDK_LIBRARIES})

## Local libs
target_link_libraries(bsfFBXImporter PRIVATE bsf)

## OS libs
if(APPLE) # MacOS
	target_link_framework(bsfFBXImporter CoreFoundation)
endif()

# IDE specific
set_property(TARGET bsfFBXImporter PROPERTY FOLDER Plugins)

# Install
install_bsf_target(bsfFBXImporter)

conditional_cotire(bsfFBXImporter)

# Source files and their filters
B3DGlobSourceFiles(${CMAKE_CURRENT_SOURCE_DIR} "" "" BS_FBXIMPORTER_SRC)

# Packages
find_package(FBXSDK REQUIRED)
	
# Target
add_library(bsfFBXImporter SHARED ${BS_FBXIMPORTER_SRC})

# Common flags
B3DSetDefaultLinkAndCompileFlags(bsfFBXImporter)

# Includes
target_include_directories(bsfFBXImporter PRIVATE "./")

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
B3DSetUpPostBuildAndInstallSteps(bsfFBXImporter)

conditional_cotire(bsfFBXImporter)

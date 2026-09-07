# Source files and their filters
B3DGlobSourceFiles(${CMAKE_CURRENT_SOURCE_DIR} "" "" BS_FBXIMPORTER_SRC)

# Packages
find_package(FBXSDK REQUIRED)
	
# Target
B3DAddPlugin(bsfFBXImporter IMPORTER ${BS_FBXIMPORTER_SRC})

# Common flags
B3DSetDefaultLinkAndCompileFlags(bsfFBXImporter)

# Includes
target_include_directories(bsfFBXImporter PRIVATE "./")

# Libraries
## External lib: FBX
target_link_libraries(bsfFBXImporter PRIVATE ${FBXSDK_LIBRARIES})

## OS libs
if(APPLE) # MacOS
	B3DTargetLinkFramework(bsfFBXImporter CoreFoundation)
endif()

# IDE specific
set_property(TARGET bsfFBXImporter PROPERTY FOLDER Plugins)

# Install
B3DSetUpPostBuildAndInstallSteps(bsfFBXImporter)

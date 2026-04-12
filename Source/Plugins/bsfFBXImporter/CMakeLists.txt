# Source files and their filters
B3DGlobSourceFiles(${CMAKE_CURRENT_SOURCE_DIR} "" "" BS_FBXIMPORTER_SRC)

# Packages
# Ensure pre-built binary dependencies are up to date
if(B3D_USE_BUNDLED_LIBRARIES)
	B3DCheckAndUpdatePrebuiltDependency(FBXSDK)
endif()

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

## Local libs
target_link_libraries(bsfFBXImporter PRIVATE bsf)

## OS libs
if(APPLE) # MacOS
	B3DTargetLinkFramework(bsfFBXImporter CoreFoundation)
endif()

# IDE specific
set_property(TARGET bsfFBXImporter PROPERTY FOLDER Plugins)

# Install
B3DSetUpPostBuildAndInstallSteps(bsfFBXImporter)

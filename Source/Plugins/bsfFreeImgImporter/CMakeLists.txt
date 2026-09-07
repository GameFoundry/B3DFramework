# Source files and their filters
B3DGlobSourceFiles(${CMAKE_CURRENT_SOURCE_DIR} "" "" BS_FREEIMGIMPORTER_SRC)

# Packages
find_package(freeimg REQUIRED)
	
# Target
B3DAddPlugin(bsfFreeImgImporter IMPORTER ${BS_FREEIMGIMPORTER_SRC})

# Common flags
B3DSetDefaultLinkAndCompileFlags(bsfFreeImgImporter)

# Includes
target_include_directories(bsfFreeImgImporter PRIVATE "./")

# Defines
target_compile_definitions(bsfFreeImgImporter PRIVATE -D_MBCS)

# Libraries
## External lib: FreeImage
target_link_libraries(bsfFreeImgImporter PRIVATE ${freeimg_LIBRARIES})

# IDE specific
set_property(TARGET bsfFreeImgImporter PROPERTY FOLDER Plugins)

# Install
B3DSetUpPostBuildAndInstallSteps(bsfFreeImgImporter)

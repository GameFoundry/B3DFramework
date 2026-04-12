# Source files and their filters
B3DGlobSourceFiles(${CMAKE_CURRENT_SOURCE_DIR} "" "" BS_FONTIMPORTER_SRC)

# Packages
if(B3D_USE_BUNDLED_LIBRARIES)
    B3DCheckAndUpdatePrebuiltDependency(freetype)
endif()

find_package(freetype REQUIRED)
	
# Target
B3DAddPlugin(bsfFontImporter IMPORTER ${BS_FONTIMPORTER_SRC})

# Common flags
B3DSetDefaultLinkAndCompileFlags(bsfFontImporter)

# Includes
target_include_directories(bsfFontImporter PRIVATE "./")

# Defines
target_compile_definitions(bsfFontImporter PRIVATE -DUSE_FREETYPE2_STATIC)

# Libraries
## External lib: FreeType
target_link_libraries(bsfFontImporter ${freetype_LIBRARIES})

## Local libs
target_link_libraries(bsfFontImporter bsf)

# IDE specific
set_property(TARGET bsfFontImporter PROPERTY FOLDER Plugins)

# Install
B3DSetUpPostBuildAndInstallSteps(bsfFontImporter)

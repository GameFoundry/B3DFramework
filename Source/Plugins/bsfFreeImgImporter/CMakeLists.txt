# Source files and their filters
B3DGlobSourceFiles(${CMAKE_CURRENT_SOURCE_DIR} "" "" BS_FREEIMGIMPORTER_SRC)

# Packages
find_package(freeimg REQUIRED)
	
# Target
add_library(bsfFreeImgImporter SHARED ${BS_FREEIMGIMPORTER_SRC})

# Common flags
add_common_flags(bsfFreeImgImporter)

# Includes
target_include_directories(bsfFreeImgImporter PRIVATE "./")

# Defines
target_compile_definitions(bsfFreeImgImporter PRIVATE -D_MBCS -DFREEIMAGE_LIB)

# Libraries
## External lib: FreeImage
target_link_libraries(bsfFreeImgImporter ${freeimg_LIBRARIES})

## Local libs
target_link_libraries(bsfFreeImgImporter bsf)

# IDE specific
set_property(TARGET bsfFreeImgImporter PROPERTY FOLDER Plugins)

# Install
install_bsf_target(bsfFreeImgImporter)

conditional_cotire(bsfFreeImgImporter)

# Find SPIRVCross
#
# This module defines
#  SPIRVCross_INCLUDE_DIRS
#  SPIRVCross_LIBRARIES
#  SPIRVCross_FOUND

B3DStartFindPackage(SPIRVCross)

set(SPIRVCross_BUNDLED_INSTALL_DIR ${B3D_FRAMEWORK_SOURCE_FOLDER}/../Dependencies/SPIRVCross)
if(B3D_USE_BUNDLED_LIBRARIES OR NOT SPIRVCross_INSTALL_DIR)
	set(SPIRVCross_INSTALL_DIR ${SPIRVCross_BUNDLED_INSTALL_DIR} CACHE PATH "Path to SPIRVCross dependency" FORCE)
endif()

B3DPopulateDefaultPackageSearchPaths(SPIRVCross)

B3DFindImportedIncludes(SPIRVCross spirv_cross/spirv_cross.hpp)
B3DFindImportedLibraryWithConfigurationNames(SPIRVCross spirv-cross-core STATIC spirv-cross-core spirv-cross-cored)
B3DFindImportedLibraryWithConfigurationNames(SPIRVCross spirv-cross-c STATIC spirv-cross-c spirv-cross-cd)
B3DFindImportedLibraryWithConfigurationNames(SPIRVCross spirv-cross-cpp STATIC spirv-cross-cpp spirv-cross-cppd)
B3DFindImportedLibraryWithConfigurationNames(SPIRVCross spirv-cross-glsl STATIC spirv-cross-glsl spirv-cross-glsld)
B3DFindImportedLibraryWithConfigurationNames(SPIRVCross spirv-cross-msl STATIC spirv-cross-msl spirv-cross-msld)
B3DFindImportedLibraryWithConfigurationNames(SPIRVCross spirv-cross-reflect STATIC spirv-cross-reflect spirv-cross-reflectd)
B3DFindImportedLibraryWithConfigurationNames(SPIRVCross spirv-cross-util STATIC spirv-cross-util spirv-cross-utild)

B3DEndFindPackage(SPIRVCross spirv-cross-core)

# The macOS libraries are built with exceptions converted to assertions, since bsf builds with -fno-exceptions
if(APPLE AND TARGET SPIRVCross::spirv-cross-core)
	set_property(TARGET SPIRVCross::spirv-cross-core APPEND PROPERTY INTERFACE_COMPILE_DEFINITIONS SPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS)
endif()

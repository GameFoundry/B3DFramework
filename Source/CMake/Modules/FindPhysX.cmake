# Find PhysX
#
# This module defines
#  PhysX_INCLUDE_DIRS
#  PhysX_LIBRARIES
#  PhysX_FOUND

B3DStartFindPackage(PhysX)

set(PhysX_BUNDLED_INSTALL_DIR ${B3D_FRAMEWORK_SOURCE_FOLDER}/../Dependencies/PhysX)
if(B3D_USE_BUNDLED_LIBRARIES OR NOT PhysX_INSTALL_DIR)
	set(PhysX_INSTALL_DIR ${PhysX_BUNDLED_INSTALL_DIR} CACHE PATH "Path to PhysX dependency" FORCE)
endif()

# Ensure the bundled copy is up to date, building it from source if no prebuilt package is available
B3DEnsureBundledDependency(PhysX BUILD_SCRIPT B3DBuildPhysX.sh)

B3DPopulateDefaultPackageSearchPaths(PhysX)

B3DFindImportedIncludes(PhysX PxPhysicsAPI.h)

# All libraries are static. Debug links PhysX's "checked" configuration, which the build script installs as the Debug
# libraries under the same file names. Dependents are listed before their dependencies for the benefit of single-pass
# linkers.
foreach(PhysXLibrary PhysXCharacterKinematic PhysXExtensions PhysXCooking PhysX PhysXPvdSDK PhysXCommon PhysXFoundation)
	B3DFindImportedLibraryWithConfigurationNames(PhysX ${PhysXLibrary} STATIC ${PhysXLibrary}_static_64 ${PhysXLibrary}_static_64)
endforeach()

B3DEndFindPackage(PhysX PhysX)

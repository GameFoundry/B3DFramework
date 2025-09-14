# Find PhysX
#
# This module defines
#  PhysX_INCLUDE_DIRS
#  PhysX_LIBRARIES
#  PhysX_FOUND

B3DStartFindPackage(PhysX)

# Always use bundled library as it is not commonly available
set(PhysX_INSTALL_DIR ${B3D_FRAMEWORK_SOURCE_FOLDER}/../Dependencies/PhysX CACHE PATH "")
B3DPopulateDefaultPackageSearchPaths(PhysX)

if(NOT APPLE)
	if(B3D_IS_64BIT)
		set(BS_PHYSX_SUFFIX _x64)
	else()
		set(BS_PHYSX_SUFFIX _x86)
	endif()
endif()

B3DFindImportedIncludes(PhysX PxPhysics.h)
if(NOT APPLE)
	B3DFindImportedLibraryWithConfigurationNames(PhysX PhysX3${BS_PHYSX_SUFFIX} SHARED PhysX3${BS_PHYSX_SUFFIX} PhysX3CHECKED${BS_PHYSX_SUFFIX})
	B3DFindImportedLibraryWithConfigurationNames(PhysX PhysX3Common${BS_PHYSX_SUFFIX} SHARED PhysX3Common${BS_PHYSX_SUFFIX} PhysX3CommonCHECKED${BS_PHYSX_SUFFIX})
	B3DFindImportedLibraryWithConfigurationNames(PhysX PhysX3Cooking${BS_PHYSX_SUFFIX} SHARED PhysX3Cooking${BS_PHYSX_SUFFIX} PhysX3CookingCHECKED${BS_PHYSX_SUFFIX})
	B3DFindImportedLibraryWithConfigurationNames(PhysX PhysX3CharacterKinematic${BS_PHYSX_SUFFIX} SHARED PhysX3CharacterKinematic${BS_PHYSX_SUFFIX} PhysX3CharacterKinematicCHECKED${BS_PHYSX_SUFFIX})
	B3DFindImportedLibraryWithConfigurationNames(PhysX PhysX3Extensions STATIC PhysX3Extensions PhysX3ExtensionsCHECKED)
else()
	B3DFindImportedLibrary(PhysX LowLevel STATIC)
	B3DFindImportedLibrary(PhysX LowLevelCloth STATIC)
	B3DFindImportedLibrary(PhysX PhysX3 STATIC)
	B3DFindImportedLibrary(PhysX PhysX3Common STATIC)
	B3DFindImportedLibrary(PhysX PhysX3Cooking STATIC)
	B3DFindImportedLibrary(PhysX PhysX3CharacterKinematic STATIC)
	B3DFindImportedLibrary(PhysX PhysX3Extensions STATIC)
	B3DFindImportedLibrary(PhysX PhysXProfileSDK STATIC)
	B3DFindImportedLibrary(PhysX PvdRuntime STATIC)
	B3DFindImportedLibrary(PhysX PxTask STATIC)
	B3DFindImportedLibrary(PhysX SceneQuery STATIC)
	B3DFindImportedLibrary(PhysX SimulationController STATIC)
endif()

B3DEndFindPackage(PhysX PhysX3${BS_PHYSX_SUFFIX})

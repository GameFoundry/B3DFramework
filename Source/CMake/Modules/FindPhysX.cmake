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
B3DPopulateDefaultPackageSearchPaths(PhysX)

if(NOT APPLE)
	if(B3D_IS_64BIT)
		set(B3D_PHYSX_SUFFIX _x64)
	else()
		set(B3D_PHYSX_SUFFIX _x86)
	endif()
endif()

B3DFindImportedIncludes(PhysX PxPhysics.h)
if(NOT APPLE)
	B3DFindImportedLibraryWithConfigurationNames(PhysX PhysX3${B3D_PHYSX_SUFFIX} SHARED PhysX3${B3D_PHYSX_SUFFIX} PhysX3CHECKED${B3D_PHYSX_SUFFIX})
	B3DFindImportedLibraryWithConfigurationNames(PhysX PhysX3Common${B3D_PHYSX_SUFFIX} SHARED PhysX3Common${B3D_PHYSX_SUFFIX} PhysX3CommonCHECKED${B3D_PHYSX_SUFFIX})
	B3DFindImportedLibraryWithConfigurationNames(PhysX PhysX3Cooking${B3D_PHYSX_SUFFIX} SHARED PhysX3Cooking${B3D_PHYSX_SUFFIX} PhysX3CookingCHECKED${B3D_PHYSX_SUFFIX})
	B3DFindImportedLibraryWithConfigurationNames(PhysX PhysX3CharacterKinematic${B3D_PHYSX_SUFFIX} SHARED PhysX3CharacterKinematic${B3D_PHYSX_SUFFIX} PhysX3CharacterKinematicCHECKED${B3D_PHYSX_SUFFIX})
	B3DFindImportedLibraryWithConfigurationNames(PhysX PhysX3Extensions STATIC PhysX3Extensions PhysX3ExtensionsCHECKED)
	B3DFindImportedLibraryWithConfigurationNames(PhysX PxFoundation${B3D_PHYSX_SUFFIX} SHARED PxFoundation${B3D_PHYSX_SUFFIX} PxFoundationCHECKED${B3D_PHYSX_SUFFIX})
	B3DFindImportedLibraryWithConfigurationNames(PhysX PxPvdSDK${B3D_PHYSX_SUFFIX} SHARED PxPvdSDK${B3D_PHYSX_SUFFIX} PxPvdSDKCHECKED${B3D_PHYSX_SUFFIX})
else()
	# Everything is static on macOS. Debug maps to PhysX's "checked" build, and dependents are
	# listed before their dependencies for the benefit of single-pass linkers.
	B3DFindImportedLibraryWithConfigurationNames(PhysX PhysX3CharacterKinematic STATIC PhysX3CharacterKinematic PhysX3CharacterKinematicCHECKED)
	B3DFindImportedLibraryWithConfigurationNames(PhysX PhysX3Cooking STATIC PhysX3Cooking PhysX3CookingCHECKED)
	B3DFindImportedLibraryWithConfigurationNames(PhysX PhysX3Extensions STATIC PhysX3Extensions PhysX3ExtensionsCHECKED)
	B3DFindImportedLibraryWithConfigurationNames(PhysX PhysX3 STATIC PhysX3 PhysX3CHECKED)
	B3DFindImportedLibraryWithConfigurationNames(PhysX SimulationController STATIC SimulationController SimulationControllerCHECKED)
	B3DFindImportedLibraryWithConfigurationNames(PhysX SceneQuery STATIC SceneQuery SceneQueryCHECKED)
	B3DFindImportedLibraryWithConfigurationNames(PhysX LowLevel STATIC LowLevel LowLevelCHECKED)
	B3DFindImportedLibraryWithConfigurationNames(PhysX LowLevelAABB STATIC LowLevelAABB LowLevelAABBCHECKED)
	B3DFindImportedLibraryWithConfigurationNames(PhysX LowLevelCloth STATIC LowLevelCloth LowLevelClothCHECKED)
	B3DFindImportedLibraryWithConfigurationNames(PhysX LowLevelDynamics STATIC LowLevelDynamics LowLevelDynamicsCHECKED)
	B3DFindImportedLibraryWithConfigurationNames(PhysX LowLevelParticles STATIC LowLevelParticles LowLevelParticlesCHECKED)
	B3DFindImportedLibraryWithConfigurationNames(PhysX PhysX3Common STATIC PhysX3Common PhysX3CommonCHECKED)
	B3DFindImportedLibraryWithConfigurationNames(PhysX PsFastXml STATIC PsFastXml PsFastXmlCHECKED)
	B3DFindImportedLibraryWithConfigurationNames(PhysX PxPvdSDK STATIC PxPvdSDK PxPvdSDKCHECKED)
	B3DFindImportedLibraryWithConfigurationNames(PhysX PxTask STATIC PxTask PxTaskCHECKED)
	B3DFindImportedLibraryWithConfigurationNames(PhysX PxFoundation STATIC PxFoundation PxFoundationCHECKED)
endif()

B3DEndFindPackage(PhysX PhysX3${B3D_PHYSX_SUFFIX})

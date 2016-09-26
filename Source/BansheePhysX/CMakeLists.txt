# Source files and their filters
include(CMakeSources.cmake)

# Includes
set(BansheePhysX_INC 
	"Include"
	"../../Dependencies/PhysX/include"
	"../BansheeUtility/Include" 
	"../BansheeCore/Include")

include_directories(${BansheePhysX_INC})	
	
# Target
add_library(BansheePhysX SHARED ${BS_BANSHEEPHYSX_SRC})

# Defines
target_compile_definitions(BansheePhysX PRIVATE -DBS_PHYSX_EXPORTS)
target_compile_definitions(BansheePhysX PRIVATE $<$<CONFIG:OptimizedDebug>:NDEBUG> $<$<CONFIG:Release>:NDEBUG>)

# Libraries
if(BS_64BIT)
	set(BS_PHYSX_SUFFIX _x64)
else()
	set(BS_PHYSX_SUFFIX _x86)
endif()

## External lib: PhysX
if(BS_64BIT)
	add_library_per_config_multi(BansheePhysX PhysX PhysX Release/PhysX3_x64 Debug/PhysX3CHECKED_x64)
	add_library_per_config_multi(BansheePhysX PhysX3Common PhysX Release/PhysX3Common_x64 Debug/PhysX3CommonCHECKED_x64)
	add_library_per_config_multi(BansheePhysX PhysX3Cooking PhysX Release/PhysX3Cooking_x64 Debug/PhysX3CookingCHECKED_x64)	
	add_library_per_config_multi(BansheePhysX PhysX3CharacterKinematic PhysX Release/PhysX3CharacterKinematic_x64 Debug/PhysX3CharacterKinematicCHECKED_x64)
	add_library_per_config_multi(BansheePhysX PhysX3Extensions PhysX Release/PhysX3Extensions Debug/PhysX3ExtensionsCHECKED)
else()
	add_library_per_config_multi(BansheePhysX PhysX3 PhysX Release/PhysX3_x86 Debug/PhysX3CHECKED_x86)
	add_library_per_config_multi(BansheePhysX PhysX3Common PhysX Release/PhysX3Common_x86 Debug/PhysX3CommonCHECKED_x86)
	add_library_per_config_multi(BansheePhysX PhysX3Cooking PhysX Release/PhysX3Cooking_x86 Debug/PhysX3CookingCHECKED_x86)	
	add_library_per_config_multi(BansheePhysX PhysX3CharacterKinematic PhysX Release/PhysX3CharacterKinematic_x86 Debug/PhysX3CharacterKinematicCHECKED_x86)
	add_library_per_config_multi(BansheePhysX PhysX3Extensions PhysX Release/PhysX3Extensions Debug/PhysX3ExtensionsCHECKED)
endif()

## Local libs
target_link_libraries(BansheePhysX PUBLIC BansheeUtility BansheeCore)

# IDE specific
set_property(TARGET BansheePhysX PROPERTY FOLDER Plugins)
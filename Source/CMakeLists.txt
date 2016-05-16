cmake_minimum_required (VERSION 3.5.1)
project (Banshee)

# Version
set (Banshee_VERSION_MAJOR 0)
set (Banshee_VERSION_MINOR 3)

# Configuration types
if(CMAKE_CONFIGURATION_TYPES) # Multiconfig generator?
	set(CMAKE_CONFIGURATION_TYPES "Debug;OptimizedDebug;Release;" CACHE STRING "" FORCE) 
else()
	if(NOT CMAKE_BUILD_TYPE)
		message("Defaulting to release build.")
		set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
	endif()
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY HELPSTRING "Choose the type of build")
	# Set the valid options for cmake-gui drop-down list
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug;OptimizedDebug;Release")
endif()

if(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
	set(BS_64BIT true)
endif()

# Global compile & linker flags
## Compiler-agnostic settings
### Target at least C++11
set(CMAKE_CXX_STANDARD 11)
## Compiler-specific settings
if(MSVC)
	# Linker
	set(BS_LINKER_FLAGS_COMMON "/DYNAMICBASE /NOLOGO")

	set(BS_LINKER_FLAGS_DEBUG "${BS_LINKER_FLAGS_COMMON} /DEBUG")
	set(BS_LINKER_FLAGS_OPTIMIZEDDEBUG "${BS_LINKER_FLAGS_COMMON} /LTCG:incremental /OPT:REF /DEBUG")
	set(BS_LINKER_FLAGS_RELEASE "${BS_LINKER_FLAGS_COMMON} /LTCG /INCREMENTAL:NO /OPT:REF")
	
	if(BS_64BIT)
		set(BS_LINKER_FLAGS_OPTIMIZEDDEBUG "${BS_LINKER_FLAGS_OPTIMIZEDDEBUG} /OPT:ICF")
		set(BS_LINKER_FLAGS_RELEASE "${BS_LINKER_FLAGS_RELEASE} /OPT:ICF")
	endif()
	
	set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "/DLL ${BS_LINKER_FLAGS_DEBUG}")
	set(CMAKE_MODULE_LINKER_FLAGS_DEBUG "/DLL ${BS_LINKER_FLAGS_DEBUG}")
	set(CMAKE_EXE_LINKER_FLAGS_DEBUG ${BS_LINKER_FLAGS_DEBUG})
	
	set(CMAKE_SHARED_LINKER_FLAGS_OPTIMIZEDDEBUG "/DLL ${BS_LINKER_FLAGS_OPTIMIZEDDEBUG}")
	set(CMAKE_MODULE_LINKER_FLAGS_OPTIMIZEDDEBUG "/DLL ${BS_LINKER_FLAGS_OPTIMIZEDDEBUG}")
	set(CMAKE_EXE_LINKER_FLAGS_OPTIMIZEDDEBUG ${BS_LINKER_FLAGS_OPTIMIZEDDEBUG})
	
	set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "/DLL ${BS_LINKER_FLAGS_RELEASE}")
	set(CMAKE_MODULE_LINKER_FLAGS_RELEASE "/DLL ${BS_LINKER_FLAGS_RELEASE}")
	set(CMAKE_EXE_LINKER_FLAGS_RELEASE ${BS_LINKER_FLAGS_RELEASE})
	
	# Compiler
	set(BS_COMPILER_FLAGS_COMMON "/GS- /W3 /GR- /WX- /nologo /bigobj /wd\"4577\"")
	set(CMAKE_CXX_FLAGS "/DWIN32 /D_WINDOWS")
	
	set(CMAKE_CXX_FLAGS_DEBUG "${BS_COMPILER_FLAGS_COMMON} /ZI /Gm /Od /RTC1 /MDd")
	set(CMAKE_CXX_FLAGS_OPTIMIZEDDEBUG "${BS_COMPILER_FLAGS_COMMON} /GL /Gy /Zi /Gm /O2 /Oi /MD")
	set(CMAKE_CXX_FLAGS_RELEASE "${BS_COMPILER_FLAGS_COMMON} /GL /Gy /O2 /Oi /MD /MP")
	
	# Global defines
	add_definitions(-D_HAS_EXCEPTIONS=0)
	
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
	# Note: Optionally add -ffunction-sections, -fdata-sections, but with linker option --gc-sections
	set(BS_COMPILER_FLAGS_COMMON "-fpic -fno-exceptions -fno-strict-aliasing -fno-rtti -fno-ms-compatibility -fms-extensions")
	
	set(CMAKE_CXX_FLAGS_DEBUG "${BS_COMPILER_FLAGS_COMMON} -g -O0")
	set(CMAKE_CXX_FLAGS_OPTIMIZEDDEBUG "${BS_COMPILER_FLAGS_COMMON} -gline-tables-only -O2")
	set(CMAKE_CXX_FLAGS_RELEASE "${BS_COMPILER_FLAGS_COMMON} -g0 -O2")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	set(BS_COMPILER_FLAGS_COMMON "-fpic -fno-exceptions -fno-strict-aliasing -fno-rtti")
	
	set(CMAKE_CXX_FLAGS_DEBUG "${BS_COMPILER_FLAGS_COMMON} -g -O0")
	set(CMAKE_CXX_FLAGS_OPTIMIZEDDEBUG "${BS_COMPILER_FLAGS_COMMON} -gline-tables-only -O2")
	set(CMAKE_CXX_FLAGS_RELEASE "${BS_COMPILER_FLAGS_COMMON} -g0 -O2")

else()
# TODO_OTHER_COMPILERS_GO_HERE
endif()

# Output
set(CMAKE_BINARY_DIR "${PROJECT_SOURCE_DIR}/../Build/${CMAKE_GENERATOR}/")

if(BS_64BIT)
	set(BS_OUTPUT_DIR_PREFIX x64)
else()
	set(BS_OUTPUT_DIR_PREFIX x86)
endif()

set(BS_BINARY_OUTPUT_DIR ${PROJECT_SOURCE_DIR}/../bin/${BS_OUTPUT_DIR_PREFIX})
set(BS_LIBRARY_OUTPUT_DIR ${PROJECT_SOURCE_DIR}/../lib/${BS_OUTPUT_DIR_PREFIX})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${BS_BINARY_OUTPUT_DIR}/Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_OPTIMIZEDDEBUG ${BS_BINARY_OUTPUT_DIR}/OptimizedDebug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${BS_BINARY_OUTPUT_DIR}/Release)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${BS_BINARY_OUTPUT_DIR}/Debug)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_OPTIMIZEDDEBUG ${BS_BINARY_OUTPUT_DIR}/OptimizedDebug)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${BS_BINARY_OUTPUT_DIR}/Release)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${BS_LIBRARY_OUTPUT_DIR}/Debug)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_OPTIMIZEDDEBUG ${BS_LIBRARY_OUTPUT_DIR}/OptimizedDebug)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${BS_LIBRARY_OUTPUT_DIR}/Release)

if(WIN32)
	set(BS_LIBRARY_EXTENSION ".lib")
else()
	set(BS_LIBRARY_EXTENSION ".a")
endif()

function(add_library_per_config target_name lib_name)
	add_library(${lib_name} STATIC IMPORTED)
	set_target_properties(${lib_name} PROPERTIES IMPORTED_LOCATION_DEBUG ${PROJECT_SOURCE_DIR}/../Dependencies/${target_name}/lib/${BS_OUTPUT_DIR_PREFIX}/Debug/${lib_name}${BS_LIBRARY_EXTENSION})
	set_target_properties(${lib_name} PROPERTIES IMPORTED_LOCATION_OPTIMIZEDDEBUG ${PROJECT_SOURCE_DIR}/../Dependencies/${target_name}/lib/${BS_OUTPUT_DIR_PREFIX}/OptimizedDebug/${lib_name}${BS_LIBRARY_EXTENSION})
	set_target_properties(${lib_name} PROPERTIES IMPORTED_LOCATION_RELEASE ${PROJECT_SOURCE_DIR}/../Dependencies/${target_name}/lib/${BS_OUTPUT_DIR_PREFIX}/Release/${lib_name}${BS_LIBRARY_EXTENSION})	
	
	target_link_libraries(${target_name} PRIVATE ${lib_name})	
endfunction()

function(add_library_per_config_suffix target_name lib_name rls_suffix debug_suffix)
	add_library(${lib_name} STATIC IMPORTED)
	set_target_properties(${lib_name} PROPERTIES IMPORTED_LOCATION_DEBUG ${PROJECT_SOURCE_DIR}/../Dependencies/${target_name}/lib/${BS_OUTPUT_DIR_PREFIX}/Debug/${lib_name}${debug_suffix}${BS_LIBRARY_EXTENSION})
	set_target_properties(${lib_name} PROPERTIES IMPORTED_LOCATION_OPTIMIZEDDEBUG ${PROJECT_SOURCE_DIR}/../Dependencies/${target_name}/lib/${BS_OUTPUT_DIR_PREFIX}/OptimizedDebug/${lib_name}${rls_suffix}${BS_LIBRARY_EXTENSION})
	set_target_properties(${lib_name} PROPERTIES IMPORTED_LOCATION_RELEASE ${PROJECT_SOURCE_DIR}/../Dependencies/${target_name}/lib/${BS_OUTPUT_DIR_PREFIX}/Release/${lib_name}${rls_suffix}${BS_LIBRARY_EXTENSION})	
	
	target_link_libraries(${target_name} PRIVATE ${lib_name})	
endfunction()

set_property(GLOBAL PROPERTY USE_FOLDERS TRUE)

# Sub-directories
## Layers
add_subdirectory(BansheeUtility)
add_subdirectory(BansheeCore)
add_subdirectory(BansheeEngine)
add_subdirectory(BansheeEditor)

## Plugins
if(WIN32)
	add_subdirectory(BansheeD3D9RenderAPI)
	add_subdirectory(BansheeD3D11RenderAPI)
endif()

add_subdirectory(BansheeGLRenderAPI)
add_subdirectory(BansheeFBXImporter)
add_subdirectory(BansheeFontImporter)
add_subdirectory(BansheeFreeImgImporter)
add_subdirectory(BansheeMono)
add_subdirectory(BansheeOISInput)
add_subdirectory(BansheePhysX)
add_subdirectory(BansheeSL)
add_subdirectory(BansheeOpenAudio)
add_subdirectory(RenderBeast)

## Script interop
add_subdirectory(SBansheeEngine)
add_subdirectory(SBansheeEditor)

## Executables
add_subdirectory(BansheeEditorExec)
add_subdirectory(Game)
add_subdirectory(ExampleProject)

## Managed projects
if(MSVC)
	include_external_msproject(MBansheeEngine ${PROJECT_SOURCE_DIR}/MBansheeEngine/MBansheeEngine.csproj)
	include_external_msproject(MBansheeEditor ${PROJECT_SOURCE_DIR}/MBansheeEditor/MBansheeEditor.csproj)
	
	add_dependencies(BansheeEditorExec MBansheeEngine MBansheeEditor)
	add_dependencies(Game MBansheeEngine)
	
	set_property(TARGET MBansheeEngine PROPERTY FOLDER Script)
	set_property(TARGET MBansheeEditor PROPERTY FOLDER Script)
else()
# TODO - Use Mono compiler to build the managed code as a pre-build step
endif()

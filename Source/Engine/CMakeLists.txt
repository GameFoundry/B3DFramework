# Source files
set(coreSourceFoldersToIgnore "")
if(NOT B3D_ENABLE_NETWORKING)
	list(APPEND coreSourceFoldersToIgnore "Network")
endif()
if(NOT B3D_BUILD_TESTS)
	list(APPEND coreSourceFoldersToIgnore "Testing")
endif()

B3DGlobSourceFilesAcrossPlatforms("Utility/" "" B3D_UTILITY_SOURCE_FILES)
B3DGlobSourceFilesAcrossPlatforms("Core/" "${coreSourceFoldersToIgnore}" B3D_CORE_SOURCE_FILES)

if(MSVC)
	list(APPEND B3D_CORE_SOURCE_FILES "Core/VSVisualizations.natvis")
endif()

# Packages
if(B3D_USE_BUNDLED_LIBRARIES)
	B3DCheckAndUpdatePrebuiltDependency(snappy)
	if(B3D_USE_NVTT)
		B3DCheckAndUpdatePrebuiltDependency(nvtt)
	endif()
	B3DCheckAndUpdatePrebuiltDependency(freetype)
endif()

find_package(snappy REQUIRED)
if(B3D_USE_NVTT)
	find_package(nvtt REQUIRED)
endif()
find_package(freetype REQUIRED)

if(B3D_ENABLE_NETWORKING)
	if(B3D_USE_BUNDLED_LIBRARIES)
		B3DCheckAndUpdatePrebuiltDependency(GameNetworkingSockets)
	endif()

	find_package(GameNetworkingSockets REQUIRED)
endif()


# Third party (non-package) libraries
add_library(ThirdParty INTERFACE)
target_include_directories(ThirdParty INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Utility/ThirdParty>
	# CityHash's city.cc includes <citycrc.h>, which in turn includes <city.h>, so its own directory must be on the include path (as in the library's upstream build).
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Utility/ThirdParty/CityHash>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include/Utility/ThirdParty>)

# Public usage requirements for bsf (include dirs + compile flags). Exposed as a
# separate INTERFACE target so static plugins can inherit them in monolithic
# builds without creating a link cycle with bsf itself.
add_library(bsfHeaders INTERFACE)

# Add global framework includes
target_include_directories(bsfHeaders INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Utility>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Core>
	$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/Generated/Utility>
	$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/Generated/Core>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include/Utility>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include/Core>)

# Add platform-specific includes
B3DGetActivePlatformFolders(activePlatformFolders)

foreach(platformFolder ${activePlatformFolders})
	set(platformEngineFolder ${B3D_PLATFORM_${platformFolder}_SOURCE_FOLDER}/Engine)

	if(EXISTS ${platformEngineFolder}/Utility)
		target_include_directories(bsfHeaders INTERFACE $<BUILD_INTERFACE:${platformEngineFolder}/Utility>)
	endif()

	if(EXISTS ${platformEngineFolder}/Core)
		target_include_directories(bsfHeaders INTERFACE $<BUILD_INTERFACE:${platformEngineFolder}/Core>)
	endif()
endforeach()

if(MSVC)
	if(NOT B3D_ENABLE_RTTI)
		target_compile_options(bsfHeaders INTERFACE
			$<$<COMPILE_LANGUAGE:CXX>:/GR->)
	endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "AppleClang")
	if(NOT B3D_ENABLE_EXCEPTIONS)
		target_compile_options(bsfHeaders INTERFACE
			-fno-exceptions)
	endif()
	if(NOT B3D_ENABLE_RTTI)
		target_compile_options(bsfHeaders INTERFACE
			$<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>)
	endif()
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	if(NOT B3D_ENABLE_EXCEPTIONS)
		target_compile_options(bsfHeaders INTERFACE
			-fno-exceptions)
	endif()
	if(NOT B3D_ENABLE_RTTI)
		target_compile_options(bsfHeaders INTERFACE
			$<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>)
	endif()
else()
# TODO_OTHER_COMPILERS_GO_HERE
endif()

# Target
add_library(bsf SHARED ${B3D_UTILITY_SOURCE_FILES} ${B3D_CORE_SOURCE_FILES})

B3DSetDefaultLinkAndCompileFlags(bsf)

# Inherit public include dirs and compile flags from the bsfHeaders interface.
target_link_libraries(bsf PUBLIC bsfHeaders)

# Defines
target_compile_definitions(bsf PRIVATE
	-DB3D_EXPORTS
	$<$<CONFIG:Debug>:B3D_CONFIG=B3D_CONFIG_ID_DEBUG>
	$<$<CONFIG:RelWithDebInfo>:B3D_CONFIG=B3D_CONFIG_ID_RELWITHDEBINFO>
	$<$<CONFIG:MinSizeRel>:B3D_CONFIG=B3D_CONFIG_ID_MINSIZEREL>
	$<$<CONFIG:Release>:B3D_CONFIG=B3D_CONFIG_ID_RELEASE>)

# Libraries
## External lib: NVTT
if(B3D_USE_NVTT)
	target_link_libraries(bsf PRIVATE ${nvtt_LIBRARIES})
endif()

## External lib: Snappy
target_link_libraries(bsf PRIVATE ${snappy_LIBRARIES})

## External lib: Freetype
target_link_libraries(bsf PRIVATE ${freetype_LIBRARIES})

## External lib: GameNetworkingSockets
if(B3D_ENABLE_NETWORKING)
	target_link_libraries(bsf PRIVATE ${GameNetworkingSockets_LIBRARIES})
endif()

## External libs: Header only libraries
target_link_libraries(bsf PUBLIC ThirdParty)


# IDE specific
set_property(TARGET bsf PROPERTY FOLDER Engine)

# Install
if(NOT B3D_IS_ENGINE)
	install(
		DIRECTORY Utility
		DESTINATION include
		FILES_MATCHING
		PATTERN "*.h"
		PATTERN "*.hpp"
		PATTERN "Private" EXCLUDE
	)

	install(
		DIRECTORY Core
		DESTINATION include
		FILES_MATCHING
		PATTERN "*.h"
		PATTERN "*.hpp"
		PATTERN "Private" EXCLUDE
	)

	install(
		DIRECTORY ${PROJECT_BINARY_DIR}/Generated/Core
		DESTINATION include
		FILES_MATCHING
		PATTERN "*.h"
	)

	install(
		DIRECTORY ${PROJECT_BINARY_DIR}/Generated/Utility
		DESTINATION include
		FILES_MATCHING
		PATTERN "*.h"
	)
endif()

B3DSetUpPostBuildAndInstallSteps(bsf)

if(NOT B3D_IS_ENGINE)
	# This doesn't actually install anything as it is an imported library
	# but it is required because it is a dependency of the bsf target
	install(TARGETS ThirdParty EXPORT bsf)

	# Public usage requirements interface. Installs no artifacts, but must be in
	# the export set because bsf links it PUBLIC and thus references it in its
	# exported INTERFACE_LINK_LIBRARIES.
	install(TARGETS bsfHeaders EXPORT bsf)

	install(
		EXPORT bsf
		NAMESPACE b3d::
		FILE bsf-config.cmake
		DESTINATION lib/bsf
	)
endif()

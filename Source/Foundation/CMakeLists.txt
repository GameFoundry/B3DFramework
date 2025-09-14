# Source files
set(coreSourceFoldersToIgnore "")
if(NOT B3D_EXPERIMENTAL_ENABLE_NETWORKING)
	list(APPEND coreSourceFoldersToIgnore "Network")
endif()

B3DGlobSourceFiles(${CMAKE_CURRENT_SOURCE_DIR} "bsfUtility/" "" BS_UTILITY_SRC)
B3DGlobSourceFiles(${CMAKE_CURRENT_SOURCE_DIR} "bsfCore/" ${coreSourceFoldersToIgnore} BS_CORE_SRC)
B3DGlobSourceFiles(${CMAKE_CURRENT_SOURCE_DIR} "bsfEngine/" "" BS_ENGINE_SRC)

if(MSVC)
	list(APPEND BS_CORE_SRC "bsfCore/VSVisualizations.natvis")
endif()

# Ensure pre-built binary dependencies are up to date
if(B3D_USE_BUNDLED_LIBRARIES)
	if(B3D_EXPERIMENTAL_ENABLE_NETWORKING)
		set(BS_RAKNET_PREBUILT_DEPENDENCY_VERSION 1) # Tick this when you change the binaries
		check_and_update_binary_deps(bsfDep_RakNet RakNet ${B3D_FRAMEWORK_SOURCE_FOLDER}/../Dependencies/RakNet ${BS_RAKNET_PREBUILT_DEPENDENCY_VERSION})
	endif()
endif()

# Packages
find_package(snappy REQUIRED)
find_package(nvtt REQUIRED)
find_package(freetype REQUIRED)

if(B3D_EXPERIMENTAL_ENABLE_NETWORKING)
	find_package(RakNet REQUIRED)
endif()

if(LINUX)
	find_package(X11 REQUIRED)
	find_package(LibUUID REQUIRED)
	find_package(LibICU REQUIRED)

	if(NOT X11_Xcursor_FOUND)
		message(FATAL_ERROR "Could not find XCursor library.")
	endif()

	if(NOT X11_Xrandr_FOUND)
		message(FATAL_ERROR "Could not find XRandR library.")
	endif()

	if(NOT X11_Xi_FOUND)
		message(FATAL_ERROR "Could not find Xi (XInput) library.")
	endif()
elseif(APPLE)
	find_package(LibUUID REQUIRED)
endif()

# Third party (non-package) libraries
add_library(ThirdParty INTERFACE)
target_include_directories(ThirdParty INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/bsfUtility/ThirdParty>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include/bsfUtility/ThirdParty>)

# Target
add_library(bsf SHARED ${BS_UTILITY_SRC} ${BS_CORE_SRC} ${BS_ENGINE_SRC})

B3DSetDefaultLinkAndCompileFlags(bsf)

# Includes
target_include_directories(bsf PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/bsfUtility>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/bsfCore>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/bsfEngine>
	$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/Generated/bsfUtility>
	$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/Generated/bsfEngine>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include/bsfUtility>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include/bsfCore>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include/bsfEngine>)

# Defines
target_compile_definitions(bsf PRIVATE
	-DB3D_EXPORTS
	-DB3D_CORE_EXPORTS
	-DB3D_UTILITY_EXPORTS
	$<$<CONFIG:Debug>:B3D_CONFIG=B3D_CONFIG_ID_DEBUG>
	$<$<CONFIG:RelWithDebInfo>:B3D_CONFIG=B3D_CONFIG_ID_RELWITHDEBINFO>
	$<$<CONFIG:MinSizeRel>:B3D_CONFIG=B3D_CONFIG_ID_MINSIZEREL>
	$<$<CONFIG:Release>:B3D_CONFIG=B3D_CONFIG_ID_RELEASE>)

if(MSVC)
	if(NOT BSF_ENABLE_RTTI)
		target_compile_options(bsf PUBLIC
			$<$<COMPILE_LANGUAGE:CXX>:/GR->)
	endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "AppleClang")
	if(NOT BSF_ENABLE_EXCEPTIONS)
		target_compile_options(bsf PUBLIC
			-fno-exceptions)
	endif()
	if(NOT BSF_ENABLE_RTTI)
		target_compile_options(bsf PUBLIC
			$<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>)
	endif()
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	if(NOT BSF_ENABLE_EXCEPTIONS)
		target_compile_options(bsf PUBLIC
			-fno-exceptions)
	endif()
	if(NOT BSF_ENABLE_RTTI)
		target_compile_options(bsf PUBLIC
			$<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>)
	endif()
else()
# TODO_OTHER_COMPILERS_GO_HERE
endif()

# Libraries
## External lib: NVTT
target_link_libraries(bsf PRIVATE ${nvtt_LIBRARIES})	

## External lib: Snappy
target_link_libraries(bsf PRIVATE ${snappy_LIBRARIES})

## External lib: Freetype
target_link_libraries(bsf PRIVATE ${freetype_LIBRARIES})

## External lib: RakNet
if(B3D_EXPERIMENTAL_ENABLE_NETWORKING)
	target_link_libraries(bsf PRIVATE ${RakNet_LIBRARIES})
endif()

## External libs: Header only libraries
target_link_libraries(bsf PUBLIC ThirdParty)

if(LINUX)
	## External lib: X11, LibUUID, LibICU
	target_link_libraries(bsf PUBLIC ${X11_LIBRARIES} ${X11_Xcursor_LIB} ${X11_Xrandr_LIB} ${X11_Xi_LIB})
	target_link_libraries(bsf PRIVATE ${LibUUID_LIBRARIES})
	target_link_libraries(bsf PRIVATE ${LibICU_LIBRARIES})
endif()

if(APPLE)
	## External lib: LibUUID
	target_link_libraries(bsf PRIVATE ${LibUUID_LIBRARIES})
endif()

## OS libs
if(WIN32)
	target_link_libraries(bsf PRIVATE Winmm dinput8 xinput9_1_0 dxguid.lib)
	target_link_libraries(bsf PRIVATE DbgHelp IPHLPAPI Rpcrt4 User32 Shcore)
elseif(LINUX)
	target_link_libraries(bsf PRIVATE dl pthread)
elseif(APPLE) # MacOS
	target_link_framework(bsf CoreServices)
	target_link_framework(bsf IOKit)
	target_link_framework(bsf AppKit)
	target_link_framework(bsf Carbon)
endif()

# IDE specific
set_property(TARGET bsf PROPERTY FOLDER Foundation)

# Install
if(NOT B3D_IS_ENGINE)
	install(
		DIRECTORY bsfUtility
		DESTINATION include
		FILES_MATCHING
		PATTERN "*.h"
		PATTERN "*.hpp"
		PATTERN "Private" EXCLUDE
	)

	install(
		DIRECTORY bsfCore
		DESTINATION include
		FILES_MATCHING
		PATTERN "*.h"
		PATTERN "*.hpp"
		PATTERN "Private" EXCLUDE
	)

	install(
		DIRECTORY bsfEngine
		DESTINATION include
		FILES_MATCHING
		PATTERN "*.h"
		PATTERN "*.hpp"
		PATTERN "Private" EXCLUDE
	)

	install(
		DIRECTORY ${PROJECT_BINARY_DIR}/Generated/bsfEngine
		DESTINATION include
		FILES_MATCHING
		PATTERN "*.h"
	)

	install(
		DIRECTORY ${PROJECT_BINARY_DIR}/Generated/bsfUtility
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

	install(
		EXPORT bsf
		namespace b3df::
		FILE bsf-config.cmake
		DESTINATION lib/bsf
	)
endif()

# Precompiled headers (Cotire)
if(MSVC)
	find_windows_system_headers(windows_headers "Windows.h")
	get_filename_component(windows_system_header_dir "${windows_headers}" DIRECTORY)
	set(ignore_paths "${CMAKE_CURRENT_SOURCE_DIR};" "${windows_system_header_dir}/..")
else()
	find_clang_invalid_libc_pch_headers(clang_bad_headers)
	set(ignore_paths "${CMAKE_CURRENT_SOURCE_DIR};${X11_INCLUDE_DIR}/X11;${clang_bad_headers}")
endif()

set_directory_properties(PROPERTIES
	COTIRE_PREFIX_HEADER_IGNORE_PATH "${ignore_paths}")

conditional_cotire(bsf)

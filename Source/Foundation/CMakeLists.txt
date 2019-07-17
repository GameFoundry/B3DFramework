# Source files and their filters
include(bsfUtility/CMakeSources.cmake)
include(bsfCore/CMakeSources.cmake)
include(bsfEngine/CMakeSources.cmake)

# Ensure pre-built binary dependencies are up to date
if(USE_BUNDLED_LIBRARIES)
	if(EXPERIMENTAL_ENABLE_NETWORKING)
		set(BS_RAKNET_PREBUILT_DEPENDENCY_VERSION 1) # Tick this when you change the binaries
		check_and_update_binary_deps(bsfDep_RakNet RakNet ${BSF_SOURCE_DIR}/../Dependencies/RakNet ${BS_RAKNET_PREBUILT_DEPENDENCY_VERSION})
	endif()
endif()

# Packages
find_package(snappy REQUIRED)
find_package(nvtt REQUIRED)

if(EXPERIMENTAL_ENABLE_NETWORKING)
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
install_dll_on_build(bsf ${PROJECT_SOURCE_DIR})

add_common_flags(bsf)

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
	-DBS_EXPORTS
	-DBS_CORE_EXPORTS
	-DBS_UTILITY_EXPORTS
	$<$<CONFIG:Debug>:BS_CONFIG=BS_CONFIG_DEBUG>
	$<$<CONFIG:RelWithDebInfo>:BS_CONFIG=BS_CONFIG_RELWITHDEBINFO>
	$<$<CONFIG:MinSizeRel>:BS_CONFIG=BS_CONFIG_MINSIZEREL>
	$<$<CONFIG:Release>:BS_CONFIG=BS_CONFIG_RELEASE>)

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

## External lib: RakNet
if(EXPERIMENTAL_ENABLE_NETWORKING)
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
	target_link_libraries(bsf PRIVATE DbgHelp IPHLPAPI Rpcrt4)
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
if(NOT BS_IS_BANSHEE3D)
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

install_bsf_target(bsf)

if(NOT BS_IS_BANSHEE3D)
	# This doesn't actually install anything as it is an imported library
	# but it is required because it is a dependency of the bsf target
	install(TARGETS ThirdParty EXPORT bsf)

	install(
		EXPORT bsf
		NAMESPACE bsf::
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

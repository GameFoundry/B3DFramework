# Source files
set(coreSourceFoldersToIgnore "")
if(NOT B3D_ENABLE_NETWORKING)
	list(APPEND coreSourceFoldersToIgnore "Network")
endif()
if(NOT B3D_BUILD_TESTS)
	list(APPEND coreSourceFoldersToIgnore "Testing")
endif()

B3DGlobSourceFiles(${CMAKE_CURRENT_SOURCE_DIR} "Utility/" "" B3D_UTILITY_SOURCE_FILES)
B3DGlobSourceFiles(${CMAKE_CURRENT_SOURCE_DIR} "Core/" coreSourceFoldersToIgnore B3D_CORE_SOURCE_FILES)

if(MSVC)
	list(APPEND B3D_CORE_SOURCE_FILES "Core/VSVisualizations.natvis")
endif()

# Packages
find_package(snappy REQUIRED)
find_package(nvtt REQUIRED)
find_package(freetype REQUIRED)

if(B3D_ENABLE_NETWORKING)
	find_package(GameNetworkingSockets REQUIRED)
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
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Utility/ThirdParty>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include/Utility/ThirdParty>)

# Target
add_library(bsf SHARED ${B3D_UTILITY_SOURCE_FILES} ${B3D_CORE_SOURCE_FILES})

B3DSetDefaultLinkAndCompileFlags(bsf)

# Includes
target_include_directories(bsf PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Utility>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Core>
	$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/Generated/Utility>
	$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/Generated/Core>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include/Utility>
	$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include/Core>)

# Defines
target_compile_definitions(bsf PRIVATE
	-DB3D_EXPORTS
	$<$<CONFIG:Debug>:B3D_CONFIG=B3D_CONFIG_ID_DEBUG>
	$<$<CONFIG:RelWithDebInfo>:B3D_CONFIG=B3D_CONFIG_ID_RELWITHDEBINFO>
	$<$<CONFIG:MinSizeRel>:B3D_CONFIG=B3D_CONFIG_ID_MINSIZEREL>
	$<$<CONFIG:Release>:B3D_CONFIG=B3D_CONFIG_ID_RELEASE>)

if(MSVC)
	if(NOT B3D_ENABLE_RTTI)
		target_compile_options(bsf PUBLIC
			$<$<COMPILE_LANGUAGE:CXX>:/GR->)
	endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "AppleClang")
	if(NOT B3D_ENABLE_EXCEPTIONS)
		target_compile_options(bsf PUBLIC
			-fno-exceptions)
	endif()
	if(NOT B3D_ENABLE_RTTI)
		target_compile_options(bsf PUBLIC
			$<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>)
	endif()
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	if(NOT B3D_ENABLE_EXCEPTIONS)
		target_compile_options(bsf PUBLIC
			-fno-exceptions)
	endif()
	if(NOT B3D_ENABLE_RTTI)
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

## External lib: GameNetworkingSockets
if(B3D_ENABLE_NETWORKING)
	target_link_libraries(bsf PRIVATE ${GameNetworkingSockets_LIBRARIES})
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
	B3DTargetLinkFramework(bsf CoreServices)
	B3DTargetLinkFramework(bsf IOKit)
	B3DTargetLinkFramework(bsf AppKit)
	B3DTargetLinkFramework(bsf Carbon)
endif()

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

	install(
		EXPORT bsf
		NAMESPACE b3d::
		FILE bsf-config.cmake
		DESTINATION lib/bsf
	)
endif()

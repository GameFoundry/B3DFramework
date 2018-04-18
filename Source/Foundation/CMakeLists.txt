# Source files and their filters
include(bsfUtility/CMakeSources.cmake)
include(bsfCore/CMakeSources.cmake)
include(bsfEngine/CMakeSources.cmake)

# Packages
find_package(snappy REQUIRED)
find_package(nvtt REQUIRED)

if(LINUX)
	find_package(X11 REQUIRED)
	find_package(LibUUID REQUIRED)
	find_package(ICU REQUIRED COMPONENTS data uc)

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
target_include_directories(ThirdParty INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/bsfUtility/ThirdParty")

# Target
add_library(bsf SHARED ${BS_UTILITY_SRC} ${BS_CORE_SRC} ${BS_ENGINE_SRC})

# Includes
target_include_directories(bsf PUBLIC "bsfUtility" "bsfCore" "bsfEngine")

# Defines
target_compile_definitions(bsf PRIVATE 
	-DBS_EXPORTS
	-DBS_CORE_EXPORTS
	-DBS_UTILITY_EXPORTS
	$<$<CONFIG:Debug>:BS_CONFIG=BS_CONFIG_DEBUG>
	$<$<CONFIG:OptimizedDebug>:BS_CONFIG=BS_CONFIG_OPTIMIZEDDEBUG>
	$<$<CONFIG:Release>:BS_CONFIG=BS_CONFIG_RELEASE>)

# Libraries
## External lib: NVTT
target_link_libraries(bsf PRIVATE ${nvtt_LIBRARIES})	

## External lib: Snappy
target_link_libraries(bsf PRIVATE ${snappy_LIBRARIES})

## External libs: Header only libraries
target_link_libraries(bsf PUBLIC ThirdParty)

if(LINUX)
	## External lib: X11, LibUUID
	target_link_libraries(bsf PUBLIC ${X11_LIBRARIES} ${X11_Xcursor_LIB} ${X11_Xrandr_LIB} ${X11_Xi_LIB})
	target_link_libraries(bsf PRIVATE ${LibUUID_LIBRARIES})
	target_link_libraries(bsf PRIVATE ${ICU_LIBRARIES})
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
install(
	DIRECTORY bsfUtility
	DESTINATION include
	FILES_MATCHING PATTERN "*.h"
	PATTERN "Private" EXCLUDE
)

install(
	DIRECTORY bsfCore 
	DESTINATION include
	FILES_MATCHING PATTERN "*.h"
	PATTERN "Private" EXCLUDE
)

install(
	DIRECTORY bsfEngine
	DESTINATION include
	FILES_MATCHING PATTERN "*.h"
	PATTERN "Private" EXCLUDE
)

if(WIN32)
	install(
		FILES $<TARGET_PDB_FILE:bsf> 
		DESTINATION bin 
		OPTIONAL
	)
endif()

install(
	TARGETS bsf
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)

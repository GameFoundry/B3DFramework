# Source files and their filters
include(CMakeSources.cmake)

# Packages
find_package(snappy REQUIRED)

if(LINUX)
	find_package(X11 REQUIRED)
	find_package(LibUUID REQUIRED)

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
target_include_directories(ThirdParty INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/ThirdParty")

# Includes
set(BansheeUtility_INC 
	"./")

if(WIN32)
	set(BansheeUtility_INC ${BansheeUtility_INC} "Win32")
endif()

include_directories(${BansheeUtility_INC})	

# Target
add_library(BansheeUtility SHARED ${BS_BANSHEEUTILITY_SRC})

# Defines
target_compile_definitions(BansheeUtility PRIVATE -DBS_UTILITY_EXPORTS)

# Libraries
## External lib: Snappy
target_link_libraries(BansheeUtility PRIVATE ${snappy_LIBRARIES})	

## External libs: Header only libraries
target_link_libraries(BansheeUtility PUBLIC ThirdParty)

if(WIN32)
	## OS libs
	target_link_libraries(BansheeUtility PRIVATE DbgHelp IPHLPAPI Rpcrt4)
elseif(LINUX)
	## OS libs
	target_link_libraries(BansheeUtility PRIVATE dl pthread)

	## External lib: X11, LibUUID
	target_link_libraries(BansheeUtility PUBLIC ${X11_LIBRARIES} ${X11_Xcursor_LIB} ${X11_Xrandr_LIB} ${X11_Xi_LIB})
	target_link_libraries(BansheeUtility PRIVATE ${LibUUID_LIBRARIES})
elseif(APPLE)
	## External lib: LibUUID
	target_link_libraries(BansheeUtility PRIVATE ${LibUUID_LIBRARIES})
endif()

# IDE specific
set_property(TARGET BansheeUtility PROPERTY FOLDER Layers)

# Install
install(
	DIRECTORY .
	DESTINATION include
	FILES_MATCHING PATTERN "*.h"
	PATTERN "Private" EXCLUDE
)

install(
	TARGETS BansheeUtility 
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)
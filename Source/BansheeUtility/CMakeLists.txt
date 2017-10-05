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
endif()

# Includes
set(BansheeUtility_INC 
	"./" 
	"ThirdParty")

if(WIN32)
	set(BansheeUtility_INC ${BansheeUtility_INC} "Win32")
endif()

include_directories(${BansheeUtility_INC})	

# Target
add_library(BansheeUtility SHARED ${BS_BANSHEEUTILITY_SRC})

add_executable(BansheeUtilityTest BsUtilityTest.cpp)
target_link_libraries(BansheeUtilityTest BansheeUtility)

# Defines
target_compile_definitions(BansheeUtility PRIVATE -DBS_UTILITY_EXPORTS)

# Libraries
## External lib: Snappy
target_link_libraries(BansheeUtility PRIVATE ${snappy_LIBRARIES})	

if(WIN32)
	## OS libs
	target_link_libraries(BansheeUtility PRIVATE DbgHelp IPHLPAPI Rpcrt4)
else()
	## OS libs
	target_link_libraries(BansheeUtility PRIVATE dl pthread)

	## External lib: X11, LibUUID
	target_link_libraries(BansheeUtility PUBLIC ${X11_LIBRARIES} ${X11_Xcursor_LIB} ${X11_Xrandr_LIB})
	target_link_libraries(BansheeUtility PRIVATE ${LibUUID_LIBRARIES})
endif()

# IDE specific
set_property(TARGET BansheeUtility PROPERTY FOLDER Layers)

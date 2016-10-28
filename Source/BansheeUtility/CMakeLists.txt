# Source files and their filters
include(CMakeSources.cmake)

# Includes
set(BansheeUtility_INC 
	"Include" 
	"Include/ThirdParty")

if(WIN32)
	set(BansheeUtility_INC ${BansheeUtility_INC} "Include/Win32")
else()
	# TODO_OTHER_PLATFORMS_GO_HERE
endif()

include_directories(${BansheeUtility_INC})	
	
# Target
add_library(BansheeUtility SHARED ${BS_BANSHEEUTILITY_SRC})

add_executable(BansheeUtilityTest Source/BansheeUtilityTest.cpp)
target_link_libraries(BansheeUtilityTest BansheeUtility)

# Defines
target_compile_definitions(BansheeUtility PRIVATE -DBS_UTILITY_EXPORTS)

# Libraries
## OS libs
if(WIN32)
target_link_libraries(BansheeUtility 
	DbgHelp
	IPHLPAPI
	Rpcrt4
	)
else()
	target_link_libraries(BansheeUtility dl)
endif()

# IDE specific
set_property(TARGET BansheeUtility PROPERTY FOLDER Layers)

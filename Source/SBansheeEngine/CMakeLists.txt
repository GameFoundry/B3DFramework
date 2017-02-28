# Source files and their filters
include(CMakeSources.cmake)

# Includes
set(SBansheeEngine_INC 
	"Include"
	"../BansheeUtility/Include" 
	"../BansheeCore/Include"
	"../BansheeEngine/Include"
	"../BansheeEditor/Include"
	"../BansheeMono/Include")

include_directories(${SBansheeEngine_INC})	
set(BS_SCRIPT_PARSER_INCLUDE_DIRS ${BS_SCRIPT_PARSER_INCLUDE_DIRS} "SBansheeEngine/Include" PARENT_SCOPE)

# Target
add_library(SBansheeEngine SHARED ${BS_SBANSHEEENGINE_SRC})
set(BS_SCRIPT_PARSER_SOURCE_FILES ${BS_SCRIPT_PARSER_SOURCE_FILES} ${BS_SBANSHEEENGINE_SRC} PARENT_SCOPE)

# Defines
target_compile_definitions(SBansheeEngine PRIVATE -DBS_SCR_BE_EXPORTS)

# Libraries
## Local libs
target_link_libraries(SBansheeEngine BansheeMono BansheeEditor BansheeEngine BansheeUtility BansheeCore)

# IDE specific
set_property(TARGET SBansheeEngine PROPERTY FOLDER Script)
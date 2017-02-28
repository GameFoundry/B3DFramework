# Source files and their filters
include(CMakeSources.cmake)

# Includes
set(SBansheeEditor_INC 
	"Include"
	"../BansheeUtility/Include" 
	"../BansheeCore/Include"
	"../BansheeEngine/Include"
	"../BansheeEditor/Include"
	"../BansheeMono/Include"
	"../SBansheeEngine/Include")

include_directories(${SBansheeEditor_INC})	
set(BS_SCRIPT_PARSER_INCLUDE_DIRS ${BS_SCRIPT_PARSER_INCLUDE_DIRS} "SBansheeEditor/Include" PARENT_SCOPE)

# Target
add_library(SBansheeEditor SHARED ${BS_SBANSHEEEDITOR_SRC})
set(BS_SCRIPT_PARSER_SOURCE_FILES ${BS_SCRIPT_PARSER_SOURCE_FILES} ${BS_SBANSHEEEDITOR_SRC} PARENT_SCOPE)

# Defines
target_compile_definitions(SBansheeEditor PRIVATE -DBS_SCR_BED_EXPORTS)

# Libraries
## Local libs
target_link_libraries(SBansheeEditor SBansheeEngine BansheeMono BansheeEditor BansheeEngine BansheeUtility BansheeCore)

# IDE specific
set_property(TARGET SBansheeEditor PROPERTY FOLDER Script)
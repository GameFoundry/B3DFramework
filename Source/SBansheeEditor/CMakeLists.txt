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
	"../SBansheeEngine/Include"
	"${PROJECT_BINARY_DIR}/Generated/Cpp/Editor/Include")

include_directories(${SBansheeEditor_INC})	

# Target
add_library(SBansheeEditor SHARED 
	${BS_SBANSHEEEDITOR_SRC} 
	${BS_GENERATED_EDITOR_H_FILES} 
	${BS_GENERATED_EDITOR_CPP_FILES})

# Defines
target_compile_definitions(SBansheeEditor PRIVATE -DBS_SCR_BED_EXPORTS)

# Libraries
## Local libs
target_link_libraries(SBansheeEditor SBansheeEngine BansheeMono BansheeEditor BansheeEngine BansheeUtility BansheeCore)

# IDE specific
set_property(TARGET SBansheeEditor PROPERTY FOLDER Script)
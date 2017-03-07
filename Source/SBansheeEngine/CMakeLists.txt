# Source files and their filters
include(CMakeSources.cmake)

# Includes
set(SBansheeEngine_INC 
	"Include"
	"../BansheeUtility/Include" 
	"../BansheeCore/Include"
	"../BansheeEngine/Include"
	"../BansheeEditor/Include"
	"../BansheeMono/Include"
	"${PROJECT_BINARY_DIR}/Generated/Cpp/Engine/Include")

include_directories(${SBansheeEngine_INC})	

# Target
add_library(SBansheeEngine SHARED 
	${BS_SBANSHEEENGINE_SRC}
	${BS_GENERATED_ENGINE_H_FILES} 
	${BS_GENERATED_ENGINE_CPP_FILES})

# Defines
target_compile_definitions(SBansheeEngine PRIVATE -DBS_SCR_BE_EXPORTS)

# Libraries
## Local libs
target_link_libraries(SBansheeEngine BansheeMono BansheeEditor BansheeEngine BansheeUtility BansheeCore)

# IDE specific
set_property(TARGET SBansheeEngine PROPERTY FOLDER Script)
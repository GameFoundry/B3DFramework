# Source files and their filters
include(CMakeSources.cmake)

# Includes
set(BansheeMono_INC 
	"Include" 
	"../BansheeUtility/Include" 
	"../BansheeCore/Include"
	"../BansheeEngine/Include"
	"../../Dependencies/Mono/include")

include_directories(${BansheeMono_INC})	
	
# Target
add_library(BansheeMono SHARED ${BS_BANSHEEMONO_SRC})

# Defines
target_compile_definitions(BansheeMono PRIVATE -DBS_MONO_EXPORTS)

# Libraries
## External lib: Mono
add_library_per_config(BansheeMono mono Release/mono-2.0 Debug/mono-2.0)

## Local libs
target_link_libraries(BansheeMono PUBLIC BansheeEngine BansheeUtility BansheeCore)

# IDE specific
set_property(TARGET BansheeMono PROPERTY FOLDER Plugins)
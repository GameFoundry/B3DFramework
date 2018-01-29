# Source files and their filters
include(CMakeSources.cmake)

# Packages
if(AUDIO_MODULE MATCHES "FMOD")
	find_package(FMOD)
	find_package(ogg)
	find_package(vorbis)
endif()

# Includes
set(BansheeFMOD_INC 
	"./" 
	"../BansheeUtility" 
	"../BansheeCore"
	"../BansheeOpenAudio")

include_directories(${BansheeFMOD_INC})	
	
# Target
add_library(BansheeFMOD SHARED ${BS_BANSHEEFMOD_SRC})

# Defines
target_compile_definitions(BansheeFMOD PRIVATE -DBS_FMOD_EXPORTS)

# Libraries
## External libs: FMOD, Vorbis, Ogg
target_link_libraries(BansheeFMOD PRIVATE ${FMOD_LIBRARIES})	
target_link_libraries(BansheeFMOD PRIVATE ${ogg_LIBRARIES})
target_link_libraries(BansheeFMOD PRIVATE ${vorbis_LIBRARIES})

## Local libs
target_link_libraries(BansheeFMOD PUBLIC BansheeUtility BansheeCore)

# IDE specific
set_property(TARGET BansheeFMOD PROPERTY FOLDER Plugins)

# Install
if(AUDIO_MODULE MATCHES "FMOD")
install(
	TARGETS BansheeFMOD
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)
endif()
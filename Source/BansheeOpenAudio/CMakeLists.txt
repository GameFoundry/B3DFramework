# Source files and their filters
include(CMakeSources.cmake)

# Find packages
if(AUDIO_MODULE MATCHES "OpenAudio")
	find_package(OpenAL REQUIRED)
	find_package(ogg REQUIRED)
	find_package(vorbis REQUIRED)
	find_package(FLAC REQUIRED)
endif()

# Includes
set(BansheeOpenAudio_INC 
	"./" 
	"../BansheeUtility" 
	"../BansheeCore")

include_directories(${BansheeOpenAudio_INC})	
	
# Target
add_library(BansheeOpenAudio SHARED ${BS_BANSHEEOPENAUDIO_SRC})

# Defines
target_compile_definitions(BansheeOpenAudio PRIVATE -DBS_OA_EXPORTS)

# Libraries
## External libs: FLAC, Vorbis, Ogg, OpenAL
target_link_libraries(BansheeOpenAudio PRIVATE ${FLAC_LIBRARIES})
target_link_libraries(BansheeOpenAudio PRIVATE ${ogg_LIBRARIES})
target_link_libraries(BansheeOpenAudio PRIVATE ${vorbis_LIBRARIES})
target_link_libraries(BansheeOpenAudio PRIVATE ${OpenAL_LIBRARIES})

## Local libs
target_link_libraries(BansheeOpenAudio PRIVATE BansheeUtility BansheeCore)

# IDE specific
set_property(TARGET BansheeOpenAudio PROPERTY FOLDER Plugins)

# Install
if(AUDIO_MODULE MATCHES "OpenAudio")
	install(
		TARGETS BansheeOpenAudio
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib
	)
endif()
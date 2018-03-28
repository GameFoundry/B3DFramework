# Source files and their filters
include(CMakeSources.cmake)

# Find packages
if(AUDIO_MODULE MATCHES "OpenAudio")
	find_package(OpenAL REQUIRED)
	find_package(ogg REQUIRED)
	find_package(vorbis REQUIRED)
	find_package(FLAC REQUIRED)
endif()
	
# Target
add_library(bsfOpenAudio SHARED ${BS_OPENAUDIO_SRC})

# Includes
target_include_directories(bsfOpenAudio PRIVATE "./")

# Defines
target_compile_definitions(bsfOpenAudio PRIVATE -DBS_OA_EXPORTS)

# Libraries
## External libs: FLAC, Vorbis, Ogg, OpenAL
target_link_libraries(bsfOpenAudio PRIVATE ${FLAC_LIBRARIES})
target_link_libraries(bsfOpenAudio PRIVATE ${ogg_LIBRARIES})
target_link_libraries(bsfOpenAudio PRIVATE ${vorbis_LIBRARIES})
target_link_libraries(bsfOpenAudio PRIVATE ${OpenAL_LIBRARIES})

## Local libs
target_link_libraries(bsfOpenAudio PRIVATE bsf)

# IDE specific
set_property(TARGET bsfOpenAudio PROPERTY FOLDER Plugins)

# Install
if(AUDIO_MODULE MATCHES "OpenAudio")
	install(
		TARGETS bsfOpenAudio
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib
	)
	
	if(WIN32)
		install(
			FILES $<TARGET_PDB_FILE:bsfOpenAudio> 
			DESTINATION bin 
			OPTIONAL
		)
	endif()
endif()
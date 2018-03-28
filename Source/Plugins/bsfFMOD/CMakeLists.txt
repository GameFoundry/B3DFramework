# Source files and their filters
include(CMakeSources.cmake)

# Packages
if(AUDIO_MODULE MATCHES "FMOD")
	find_package(FMOD)
	find_package(ogg)
	find_package(vorbis)
endif()
	
# Target
add_library(bsfFMOD SHARED ${BS_FMOD_SRC})

# Includes
target_include_directories(bsfFMOD PRIVATE "./")

# Defines
target_compile_definitions(bsfFMOD PRIVATE -DBS_FMOD_EXPORTS)

# Libraries
## External libs: FMOD, Vorbis, Ogg
target_link_libraries(bsfFMOD PRIVATE ${FMOD_LIBRARIES})	
target_link_libraries(bsfFMOD PRIVATE ${ogg_LIBRARIES})
target_link_libraries(bsfFMOD PRIVATE ${vorbis_LIBRARIES})

## Local libs
target_link_libraries(bsfFMOD PUBLIC bsf)

# IDE specific
set_property(TARGET bsfFMOD PROPERTY FOLDER Plugins)

# Install
if(AUDIO_MODULE MATCHES "FMOD")
	install(
		TARGETS bsfFMOD
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib
	)

	if(WIN32)
		install(
			FILES $<TARGET_PDB_FILE:bsfFMOD> 
			DESTINATION bin 
			OPTIONAL
		)
	endif()
endif()
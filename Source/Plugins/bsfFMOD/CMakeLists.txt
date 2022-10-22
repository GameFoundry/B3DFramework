# Source files and their filters
include(CMakeSources.cmake)

# Packages
if(B3D_AUDIO_BACKEND MATCHES "FMOD")
	find_package(FMOD)
	find_package(ogg)
	find_package(vorbis)
endif()
	
# Target
add_library(bsfFMOD SHARED ${BS_FMOD_SRC})

# Common flags
add_common_flags(bsfFMOD)

# Includes
target_include_directories(bsfFMOD PRIVATE
	"./"
	"../bsfOpenAudio/")

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
if(B3D_AUDIO_BACKEND MATCHES "FMOD")
	install_bsf_target(bsfFMOD)
endif()

conditional_cotire(bsfFMOD)

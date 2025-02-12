# Source files and their filters
B3DGlobSourceFiles(${CMAKE_CURRENT_SOURCE_DIR} "" "" BS_OPENAUDIO_SRC)

# Find packages
if(B3D_AUDIO_BACKEND MATCHES "OpenAudio")
	find_package(OpenAL REQUIRED)
	find_package(ogg REQUIRED)
	find_package(vorbis REQUIRED)
	find_package(FLAC REQUIRED)
endif()
	
# Target
add_library(bsfOpenAudio SHARED ${BS_OPENAUDIO_SRC})

# Common flags
B3DSetDefaultLinkAndCompileFlags(bsfOpenAudio)

# Includes
target_include_directories(bsfOpenAudio PRIVATE "./")

# Libraries
## External libs: FLAC, Vorbis, Ogg, OpenAL
target_link_libraries(bsfOpenAudio PRIVATE ${OpenAL_LIBRARIES})
target_link_libraries(bsfOpenAudio PRIVATE ${vorbis_LIBRARIES})
target_link_libraries(bsfOpenAudio PRIVATE ${FLAC_LIBRARIES})
target_link_libraries(bsfOpenAudio PRIVATE ${ogg_LIBRARIES})

## OS libs
if(APPLE) # MacOS
	target_link_framework(bsfOpenAudio CoreAudio)
	target_link_framework(bsfOpenAudio AudioUnit)
	target_link_framework(bsfOpenAudio AudioToolbox)
endif()

## Local libs
target_link_libraries(bsfOpenAudio PRIVATE bsf)

# IDE specific
set_property(TARGET bsfOpenAudio PROPERTY FOLDER Plugins)

# Install
if(B3D_AUDIO_BACKEND MATCHES "OpenAudio")
	B3DSetUpPostBuildAndInstallSteps(bsfOpenAudio)
endif()

conditional_cotire(bsfOpenAudio)

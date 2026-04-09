# Source files and their filters
B3DGlobSourceFiles(${CMAKE_CURRENT_SOURCE_DIR} "" "" BS_OPENAUDIO_SRC)

# Find packages
if(B3D_AUDIO_BACKEND MATCHES "OpenAudio" OR B3D_BUILD_ALL_PLUGINS)
	# Ensure pre-built binary dependencies are up to date
	if(B3D_USE_BUNDLED_LIBRARIES)
		B3DCheckAndUpdatePrebuiltDependency(OpenAL)
		B3DCheckAndUpdatePrebuiltDependency(libFLAC)
		B3DCheckAndUpdatePrebuiltDependency(libvorbis)
		B3DCheckAndUpdatePrebuiltDependency(libogg)
	endif()

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
	B3DTargetLinkFramework(bsfOpenAudio CoreAudio)
	B3DTargetLinkFramework(bsfOpenAudio AudioUnit)
	B3DTargetLinkFramework(bsfOpenAudio AudioToolbox)
endif()

## Local libs
target_link_libraries(bsfOpenAudio PRIVATE bsf)

# IDE specific
set_property(TARGET bsfOpenAudio PROPERTY FOLDER Plugins)

# Install
if(B3D_AUDIO_BACKEND MATCHES "OpenAudio" OR B3D_BUILD_ALL_PLUGINS)
	B3DSetUpPostBuildAndInstallSteps(bsfOpenAudio)
endif()

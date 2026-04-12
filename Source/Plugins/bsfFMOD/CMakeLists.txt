# Source files and their filters
B3DGlobSourceFiles(${CMAKE_CURRENT_SOURCE_DIR} "" "" BS_FMOD_SRC)

# Packages
if(B3D_AUDIO_BACKEND MATCHES "FMOD")
	# Ensure pre-built binary dependencies are up to date
	if(B3D_USE_BUNDLED_LIBRARIES)
		B3DCheckAndUpdatePrebuiltDependency(libvorbis)
		B3DCheckAndUpdatePrebuiltDependency(libogg)
	endif()

	find_package(FMOD)
	find_package(ogg)
	find_package(vorbis)
endif()
	
# Target
B3DAddPlugin(bsfFMOD ENGINE ${BS_FMOD_SRC})

# Common flags
B3DSetDefaultLinkAndCompileFlags(bsfFMOD)

# Includes
target_include_directories(bsfFMOD PRIVATE
	"./"
	"../bsfOpenAudio/")

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
	B3DSetUpPostBuildAndInstallSteps(bsfFMOD)
endif()

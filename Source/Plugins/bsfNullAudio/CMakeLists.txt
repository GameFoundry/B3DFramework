# Source files and their filters
B3DGlobSourceFiles(${CMAKE_CURRENT_SOURCE_DIR} "" "" BS_NULLAUDIO_SRC)

# Target
B3DAddPlugin(bsfNullAudio ENGINE ${BS_NULLAUDIO_SRC})

# Common flags
B3DSetDefaultLinkAndCompileFlags(bsfNullAudio)

# Includes
target_include_directories(bsfNullAudio PRIVATE "./")

# Libraries
## Local libs
target_link_libraries(bsfNullAudio PRIVATE bsf)

# IDE specific
set_property(TARGET bsfNullAudio PROPERTY FOLDER Plugins)

# Install
if(B3D_AUDIO_BACKEND MATCHES "Null" OR B3D_BUILD_ALL_PLUGINS)
	B3DSetUpPostBuildAndInstallSteps(bsfNullAudio)
endif()

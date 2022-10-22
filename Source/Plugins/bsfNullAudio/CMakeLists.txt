# Source files and their filters
include(CMakeSources.cmake)
	
# Target
add_library(bsfNullAudio SHARED ${BS_NULLAUDIO_SRC})

# Common flags
add_common_flags(bsfNullAudio)

# Includes
target_include_directories(bsfNullAudio PRIVATE "./")

# Libraries
## Local libs
target_link_libraries(bsfNullAudio PRIVATE bsf)

# IDE specific
set_property(TARGET bsfNullAudio PROPERTY FOLDER Plugins)

# Install
if(B3D_AUDIO_BACKEND MATCHES "Null")
	install_bsf_target(bsfNullAudio)
endif()

conditional_cotire(bsfNullAudio)

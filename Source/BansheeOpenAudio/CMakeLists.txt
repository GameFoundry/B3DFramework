# Source files and their filters
include(CMakeSources.cmake)

# Includes
set(BansheeOpenAudio_INC 
	"Include" 
	"../BansheeUtility/Include" 
	"../BansheeCore/Include"
	"../../Dependencies/libogg/include"
	"../../Dependencies/libvorbis/include"
	"../../Dependencies/libFLAC/include"
	"../../Dependencies/OpenAL/include")

include_directories(${BansheeOpenAudio_INC})	
	
# Target
add_library(BansheeOpenAudio SHARED ${BS_BANSHEEOPENAUDIO_SRC})

# Libraries
## External libs: FLAC, Vorbis, Ogg, OpenAL
add_library_per_config(BansheeOpenAudio libFLAC libFLAC libFLAC)
add_library_per_config_multi(BansheeOpenAudio libvorbis libvorbis libvorbis libvorbis)
add_library_per_config_multi(BansheeOpenAudio libvorbisfile libvorbis libvorbisfile libvorbisfile)
add_library_per_config(BansheeOpenAudio libogg Release/libogg Debug/libogg)
add_library_per_config(BansheeOpenAudio OpenAL OpenAL32 OpenAL32)

## Local libs
target_link_libraries(BansheeOpenAudio PUBLIC BansheeUtility BansheeCore)

# IDE specific
set_property(TARGET BansheeOpenAudio PROPERTY FOLDER Plugins)
set(BS_BANSHEEFMOD_INC_NOFILTER
	"Include/BsFMODPrerequisites.h"
	"Include/BsFMODAudio.h"
	"Include/BsFMODImporter.h"
	"Include/BsFMODAudioSource.h"
	"Include/BsFMODAudioListener.h"
	"Include/BsFMODAudioClip.h"
	"../BansheeOpenAudio/Include/BsOggVorbisWriter.h"
)

set(BS_BANSHEEFMOD_SRC_NOFILTER
	"Source/BsFMODPlugin.cpp"
	"Source/BsFMODAudio.cpp"
	"Source/BsFMODImporter.cpp"
	"Source/BsFMODAudioSource.cpp"
	"Source/BsFMODAudioListener.cpp"
	"Source/BsFMODAudioClip.cpp"
	"../BansheeOpenAudio/Source/BsOggVorbisWriter.cpp"
)

source_group("Header Files" FILES ${BS_BANSHEEFMOD_INC_NOFILTER})
source_group("Source Files" FILES ${BS_BANSHEEFMOD_SRC_NOFILTER})

set(BS_BANSHEEFMOD_SRC
	${BS_BANSHEEFMOD_INC_NOFILTER}
	${BS_BANSHEEFMOD_SRC_NOFILTER}
)
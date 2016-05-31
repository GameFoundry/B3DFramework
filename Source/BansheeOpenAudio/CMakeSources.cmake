set(BS_BANSHEEOPENAUDIO_INC_NOFILTER
	"Include/BsOAPrerequisites.h"
	"Include/BsOAImporter.h"
	"Include/BsOAWaveReader.h"
	"Include/BsOAOggVorbisReader.h"
	"Include/BsOAFLACReader.h"
	"Include/BsOAFileReader.h"
	"Include/BsOggVorbisWriter.h"
	"Include/BsOAAudioClip.h"
	"Include/BsOAAudio.h"
	"Include/BsOAAudioSource.h"
	"Include/BsOAAudioListener.h"
)

set(BS_BANSHEEOPENAUDIO_SRC_NOFILTER
	"Source/BsOAPlugin.cpp"
	"Source/BsOAImporter.cpp"
	"Source/BsOAWaveReader.cpp"
	"Source/BsOAOggVorbisReader.cpp"
	"Source/BsOAFLACReader.cpp"
	"Source/BsOggVorbisWriter.cpp"
	"Source/BsOAAudioClip.cpp"
	"Source/BsOAAudio.cpp"
	"Source/BsOAAudioSource.cpp"
	"Source/BsOAAudioListener.cpp"
)

source_group("Header Files" FILES ${BS_BANSHEEOPENAUDIO_INC_NOFILTER})
source_group("Source Files" FILES ${BS_BANSHEEOPENAUDIO_SRC_NOFILTER})

set(BS_BANSHEEOPENAUDIO_SRC
	${BS_BANSHEEOPENAUDIO_INC_NOFILTER}
	${BS_BANSHEEOPENAUDIO_SRC_NOFILTER}
)
//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsOAPrerequisites.h"
#include "BsOAFileReader.h"
#include "vorbis\vorbisfile.h"

namespace BansheeEngine
{
	/** @addtogroup OpenAudio
	 *  @{
	 */

	/** Used for reading Ogg Vorbis audio files. */
	class OAOggVorbisReader : public OAFileReader
	{
	public:
		OAOggVorbisReader();
		~OAOggVorbisReader();

		/** @copydoc OAFileReader::open */
		bool open(const SPtr<DataStream>& stream, AudioFileInfo& info) override;

		/** @copydoc OAFileReader::read */
		UINT32 read(UINT8* samples, UINT32 numSamples) override;

		/** @copydoc OAFileReader::seek */
		void seek(UINT32 offset) override;

		/** @copydoc OAFileReader::isValid */
		bool isValid(const SPtr<DataStream>& stream) override;
	private:
		SPtr<DataStream> mStream;
		OggVorbis_File mOggVorbisFile;
		UINT32 mChannelCount;
	};

	/** @} */
}
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

	/** Information used by the active decoder. */
	struct OggDecoderData
	{
		OggDecoderData()
			: offset(0)
		{ }

		SPtr<DataStream> stream;
		UINT32 offset;
	};

	/** Used for reading Ogg Vorbis audio data. */
	class OAOggVorbisReader : public OAFileReader
	{
	public:
		OAOggVorbisReader();
		~OAOggVorbisReader();

		/** @copydoc OAFileReader::open */
		bool open(const SPtr<DataStream>& stream, AudioDataInfo& info, UINT32 offset = 0) override;

		/** @copydoc OAFileReader::read */
		UINT32 read(UINT8* samples, UINT32 numSamples) override;

		/** @copydoc OAFileReader::seek */
		void seek(UINT32 offset) override;

		/** @copydoc OAFileReader::isValid */
		bool isValid(const SPtr<DataStream>& stream, UINT32 offset = 0) override;
	private:
		OggDecoderData mDecoderData;
		OggVorbis_File mOggVorbisFile;
		UINT32 mChannelCount;
	};

	/** @} */
}
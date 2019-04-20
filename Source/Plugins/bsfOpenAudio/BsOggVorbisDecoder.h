//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsOAPrerequisites.h"
#include "BsAudioDecoder.h"
#include "vorbis/vorbisfile.h"

namespace bs
{
	/** @addtogroup OpenAudio
	 *  @{
	 */

	/** Information used by the active decoder. */
	struct OggDecoderData
	{
		OggDecoderData() = default;

		SPtr<DataStream> stream;
		UINT32 offset = 0;
	};

	/** Used for reading Ogg Vorbis audio data. */
	class OggVorbisDecoder : public AudioDecoder
	{
	public:
		OggVorbisDecoder();
		~OggVorbisDecoder();

		/** @copydoc AudioDecoder::open */
		bool open(const SPtr<DataStream>& stream, AudioDataInfo& info, UINT32 offset = 0) override;

		/** @copydoc AudioDecoder::read */
		UINT32 read(UINT8* samples, UINT32 numSamples) override;

		/** @copydoc AudioDecoder::seek */
		void seek(UINT32 offset) override;

		/** @copydoc AudioDecoder::isValid */
		bool isValid(const SPtr<DataStream>& stream, UINT32 offset = 0) override;
	private:
		OggDecoderData mDecoderData;
		OggVorbis_File mOggVorbisFile;
		UINT32 mChannelCount = 0;
	};

	/** @} */
}

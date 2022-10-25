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

		SPtr<DataStream> Stream;
		u32 Offset = 0;
	};

	/** Used for reading Ogg Vorbis audio data. */
	class OggVorbisDecoder : public AudioDecoder
	{
	public:
		OggVorbisDecoder();
		~OggVorbisDecoder();

		/** @copydoc AudioDecoder::open */
		bool Open(const SPtr<DataStream>& stream, AudioDataInfo& info, u32 offset = 0);

		/** @copydoc AudioDecoder::read */
		u32 Read(u8* samples, u32 numSamples) override;

		/** @copydoc AudioDecoder::seek */
		void Seek(u32 offset) override;

		/** @copydoc AudioDecoder::isValid */
		bool IsValid(const SPtr<DataStream>& stream, u32 offset = 0);

	private:
		OggDecoderData mDecoderData;
		OggVorbis_File mOggVorbisFile;
		u32 mChannelCount = 0;
	};

	/** @} */
} // namespace bs

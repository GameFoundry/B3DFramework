//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsOAPrerequisites.h"
#include "BsAudioDecoder.h"

namespace bs
{
	/** @addtogroup OpenAudio
	 *  @{
	 */

	/** Decodes .WAV audio data into raw PCM format. */
	class WaveDecoder : public AudioDecoder
	{
	public:
		WaveDecoder() = default;

		/** @copydoc AudioDecoder::open */
		bool Open(const SPtr<DataStream>& stream, AudioDataInfo& info, u32 offset = 0);

		/** @copydoc AudioDecoder::read */
		u32 Read(u8* samples, u32 numSamples);

		/** @copydoc AudioDecoder::seek */
		void Seek(u32 offset);

		/** @copydoc AudioDecoder::isValid */
		bool IsValid(const SPtr<DataStream>& stream, u32 offset = 0);

	private:
		/** Parses the WAVE header and output audio file meta-data. Returns false if the header is not valid. */
		bool ParseHeader(AudioDataInfo& info);

		SPtr<DataStream> mStream;
		u32 mDataOffset = 0;
		u32 mBytesPerSample = 0;

		static const u32 MAIN_CHUNK_SIZE = 12;
	};

	/** @} */
} // namespace bs

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
		bool Open(const SPtr<DataStream>& stream, AudioDataInfo& info, UINT32 offset = 0) override;

		/** @copydoc AudioDecoder::read */
		UINT32 Read(UINT8* samples, UINT32 numSamples) override;

		/** @copydoc AudioDecoder::seek */
		void Seek(UINT32 offset) override;

		/** @copydoc AudioDecoder::isValid */
		bool IsValid(const SPtr<DataStream>& stream, UINT32 offset = 0) override;
	private:
		/** Parses the WAVE header and output audio file meta-Data. Returns false if the header is not valid. */
		bool ParseHeader(AudioDataInfo& info);

		SPtr<DataStream> mStream;
		UINT32 mDataOffset = 0;
		UINT32 mBytesPerSample = 0;

		static const UINT32 MAIN_CHUNK_SIZE = 12;
	};

	/** @} */
}

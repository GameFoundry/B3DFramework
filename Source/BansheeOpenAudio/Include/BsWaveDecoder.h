//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsOAPrerequisites.h"
#include "BsAudioDecoder.h"

namespace BansheeEngine
{
	/** @addtogroup OpenAudio
	 *  @{
	 */

	/** Decodes .WAV audio data into raw PCM format. */
	class WaveDecoder : public AudioDecoder
	{
	public:
		WaveDecoder();

		/** @copydoc AudioDecoder::open */
		bool open(const SPtr<DataStream>& stream, AudioDataInfo& info, UINT32 offset = 0) override;

		/** @copydoc AudioDecoder::read */
		UINT32 read(UINT8* samples, UINT32 numSamples) override;

		/** @copydoc AudioDecoder::seek */
		void seek(UINT32 offset) override;

		/** @copydoc AudioDecoder::isValid */
		bool isValid(const SPtr<DataStream>& stream, UINT32 offset = 0) override;
	private:
		/** Parses the WAVE header and output audio file meta-data. Returns false if the header is not valid. */
		bool parseHeader(AudioDataInfo& info);

		SPtr<DataStream> mStream;
		UINT32 mDataOffset;
		UINT32 mBytesPerSample;

		static const UINT32 MAIN_CHUNK_SIZE = 12;
	};

	/** @} */
}
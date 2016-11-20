//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsOAPrerequisites.h"
#include "BsAudioDecoder.h"
#include "FLAC\stream_decoder.h"

namespace bs
{
	/** @addtogroup OpenAudio
	 *  @{
	 */

	/** Data used by the FLAC decoder. */
	struct FLACDecoderData
	{
		SPtr<DataStream> stream;
		UINT32 streamOffset = 0;
		AudioDataInfo info;
		UINT8* output = nullptr;
		Vector<UINT8> overflow;
		UINT32 samplesToRead = 0;
		bool error = false;
	};

	/** Decodes FLAC audio data into raw PCM samples. */
	class FLACDecoder : public AudioDecoder
	{
	public:
		FLACDecoder();
		~FLACDecoder();

		/** @copydoc AudioDecoder::open */
		bool open(const SPtr<DataStream>& stream, AudioDataInfo& info, UINT32 offset = 0) override;

		/** @copydoc AudioDecoder::seek */
		void seek(UINT32 offset) override;

		/** @copydoc AudioDecoder::read */
		UINT32 read(UINT8* samples, UINT32 numSamples) override;

		/** @copydoc AudioDecoder::isValid */
		bool isValid(const SPtr<DataStream>& stream, UINT32 offset = 0) override;
	private:
		/** Cleans up the FLAC decoder. */
		void close();

		FLAC__StreamDecoder* mDecoder;
		FLACDecoderData mData;
	};

	/** @} */
}
//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsOAPrerequisites.h"
#include "BsOAFileReader.h"
#include "FLAC\stream_decoder.h"

namespace BansheeEngine
{
	/** @addtogroup OpenAudio
	 *  @{
	 */

	/** Data used by the FLAC decoder. */
	struct FLACDecoderData
	{
		SPtr<DataStream> stream;
		UINT32 streamOffset = 0;
		AudioFileInfo info;
		UINT8* output = nullptr;
		Vector<UINT8> overflow;
		UINT32 samplesToRead = 0;
		bool error = false;
	};

	/** Used for reading FLAC audio data. */
	class OAFLACReader : public OAFileReader
	{
	public:
		OAFLACReader();
		~OAFLACReader();

		/** @copydoc OAFileReader::open */
		bool open(const SPtr<DataStream>& stream, AudioFileInfo& info, UINT32 offset = 0) override;

		/** @copydoc OAFileReader::seek */
		void seek(UINT32 offset) override;

		/** @copydoc OAFileReader::read */
		UINT32 read(UINT8* samples, UINT32 numSamples) override;

		/** @copydoc OAFileReader::isValid */
		bool isValid(const SPtr<DataStream>& stream, UINT32 offset = 0) override;
	private:
		/** Cleans up the FLAC decoder. */
		void close();

		FLAC__StreamDecoder* mDecoder;
		FLACDecoderData mData;
	};

	/** @} */
}
//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DOAPrerequisites.h"
#include "B3DAudioDecoder.h"
#include "FLAC/stream_decoder.h"

namespace b3d
{
	/** @addtogroup OpenAudio
	 *  @{
	 */

	/** Data used by the FLAC decoder. */
	struct FLACDecoderData
	{
		SPtr<DataStream> Stream;
		u32 StreamOffset = 0;
		AudioDataInfo Info;
		u8* Output = nullptr;
		Vector<u8> Overflow;
		u32 SamplesToRead = 0;
		bool Error = false;
	};

	/** Decodes FLAC audio data into raw PCM samples. */
	class FLACDecoder : public AudioDecoder
	{
	public:
		FLACDecoder() = default;
		~FLACDecoder();

		bool Open(const SPtr<DataStream>& stream, AudioDataInfo& info, u32 offset = 0) override;
		void Seek(u32 offset) override;
		u32 Read(u8* samples, u32 numSamples) override;
		bool IsValid(const SPtr<DataStream>& stream, u32 offset = 0) override;

	private:
		/** Cleans up the FLAC decoder. */
		void Close();

		FLAC__StreamDecoder* mDecoder = nullptr;
		FLACDecoderData mData;
	};

	/** @} */
} // namespace b3d

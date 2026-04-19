//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DOAPrerequisites.h"
#include "B3DAudioDecoder.h"

namespace b3d
{
	/** @addtogroup OpenAudio
	 *  @{
	 */

	/** Decodes .WAV audio data into raw PCM format. */
	class WaveDecoder : public AudioDecoder
	{
	public:
		WaveDecoder() = default;

		bool Open(const SPtr<DataStream>& stream, AudioDataInfo& info, u32 offset = 0) override;
		u32 Read(u8* samples, u32 numSamples) override;
		void Seek(u32 offset) override;
		bool IsValid(const SPtr<DataStream>& stream, u32 offset = 0) override;

	private:
		/** Parses the WAVE header and output audio file meta-data. Returns false if the header is not valid. */
		bool ParseHeader(AudioDataInfo& info);

		SPtr<DataStream> mStream;
		u32 mDataOffset = 0;
		u32 mBytesPerSample = 0;

		static const u32 kMainChunkSize = 12;
	};

	/** @} */
} // namespace b3d

//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DOAPrerequisites.h"
#include "B3DAudioDecoder.h"
#include "vorbis/vorbisfile.h"

namespace b3d
{
	/** @addtogroup OpenAudio
	 *  @{
	 */

	/** Information used by the active decoder. */
	struct OggDecoderData
	{
		OggDecoderData() = default;

		TShared<DataStream> Stream;
		u32 Offset = 0;
	};

	/** Used for reading Ogg Vorbis audio data. */
	class OggVorbisDecoder : public AudioDecoder
	{
	public:
		OggVorbisDecoder();
		~OggVorbisDecoder();

		bool Open(const TShared<DataStream>& stream, AudioDataInfo& info, u32 offset = 0) override;
		u32 Read(u8* samples, u32 numSamples) override;
		void Seek(u32 offset) override;
		bool IsValid(const TShared<DataStream>& stream, u32 offset = 0) override;

	private:
		OggDecoderData mDecoderData;
		OggVorbis_File mOggVorbisFile;
		u32 mChannelCount = 0;
	};

	/** @} */
} // namespace b3d

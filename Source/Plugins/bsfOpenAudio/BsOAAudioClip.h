//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsOAPrerequisites.h"
#include "Audio/BsAudioClip.h"
#include "BsOggVorbisDecoder.h"

namespace bs
{
	/** @addtogroup OpenAudio
	 *  @{
	 */

	/** OpenAudio implementation of an AudioClip. */
	class OAAudioClip : public AudioClip
	{
	public:
		OAAudioClip(const SPtr<DataStream>& samples, u32 streamSize, u32 numSamples, const AUDIO_CLIP_DESC& desc);
		virtual ~OAAudioClip();

		/**
		 * Returns audio samples in PCM format, channel data interleaved. Only available if the audio data has been created
		 * with AudioReadMode::Stream, AudioReadMode::LoadCompressed (and the format is compressed), or if @p keepSourceData
		 * was enabled on creation.
		 *
		 * @param[in]	samples		Previously allocated buffer to contain the samples.
		 * @param[in]	offset		Offset in number of samples at which to start reading (should be a multiple of number
		 *							of channels).
		 * @param[in]	count		Number of samples to read (should be a multiple of number of channels).
		 *
		 * @note	Implementation must be thread safe as this will get called from audio streaming thread.
		 */
		void GetSamples(u8* samples, u32 offset, u32 count) const;

		/** @name Internal
		 *  @{
		 */

		/** Returns the internal OpenAL buffer. Only valid if the audio clip was created without AudioReadMode::Stream. */
		u32 GetOpenALBufferInternal() const { return mBufferId; }

		/** @} */
	protected:
		/** @copydoc Resource::initialize */
		void Initialize();

		/** @copydoc AudioClip::getSourceStream */
		SPtr<DataStream> GetSourceStream(u32& size);

	private:
		mutable Mutex mMutex;
		mutable OggVorbisDecoder mVorbisReader;
		bool mNeedsDecompression = false;
		u32 mBufferId = (u32)-1;

		// These streams exist to save original audio data in case it's needed later (usually for saving with the editor, or
		// manual data manipulation). In normal usage (in-game) these will be null so no memory is wasted.
		SPtr<DataStream> mSourceStreamData;
		u32 mSourceStreamSize = 0;
	};

	/** @} */
} // namespace bs

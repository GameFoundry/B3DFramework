//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsOAPrerequisites.h"

namespace bs
{
	/** @addtogroup OpenAudio
	 *  @{
	 */

	/** Interface used for implementations that parse audio formats into a set of PCM samples. */
	class AudioDecoder
	{
	public:
		virtual ~AudioDecoder() = default;

		/** 
		 * Attempts to open audio data from the provided stream. Must be called before any reads or seeks. 
		 *
		 * @param[in]	stream	Data stream audio data is stored in.
		 * @param[out]	info	Output information describing meta-data of the audio in the stream.
		 * @param[in]	offset	Offset at which audio data in the stream begins, in bytes.
		 * @return				True if the data is valid, false otherwise.
		 */
		virtual bool open(const SPtr<DataStream>& stream, AudioDataInfo& info, UINT32 offset = 0) = 0;

		/** 
		 * Moves the read pointer to the specified offset. Any further read() calls will read from this location. User must
		 * ensure not to seek past the end of the data.
		 *
		 * @param[in]	offset	Offset to move the pointer in. In number of samples.
		 */
		virtual void seek(UINT32 offset) = 0;

		/**
		 * Reads a set of samples from the audio data.
		 *
		 * @param[out]	samples		Pre-allocated buffer to store the samples in.
		 * @param[in]	numSamples	Number of samples to read.
		 * @return					Number of samples that were actually read (can be less than requested if the more data
		 *							in the stream).
		 *
		 * @note All values are returned as signed values.
		 */
		virtual UINT32 read(UINT8* samples, UINT32 numSamples) = 0;

		/** 
		 * Checks if the data in the provided stream valid audio data for the current format. You should check this before
		 * calling open().
		 *
		 * @param[in]	stream	Stream to check.
		 * @param[in]	offset	Offset at which audio data in the stream begins, in bytes.
		 * @return				True if the data is valid, false otherwise.
		 */
		virtual bool isValid(const SPtr<DataStream>& stream, UINT32 offset = 0) = 0;
	};

	/** @} */
}
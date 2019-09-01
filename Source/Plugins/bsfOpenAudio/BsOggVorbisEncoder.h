//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "vorbis/vorbisenc.h"

namespace bs
{
	/** @addtogroup OpenAudio
	 *  @{
	 */

	/** Used for encoding PCM to Ogg Vorbis audio data. */
	class OggVorbisEncoder
	{
	public:
		OggVorbisEncoder() = default;
		~OggVorbisEncoder();

		/**
		 * Sets up the writer. Should be called before calling write().
		 *
		 * @param[in]	writeCallback	Callback that will be triggered when the writer is ready to output some data.
		 *								The callback should copy the provided data into its own buffer.
		 * @param[in]	sampleRate		Determines how many samples per second the written data will have.
		 * @param[in]	bitDepth		Determines the size of a single sample, in bits.
		 * @param[in]	numChannels		Determines the number of audio channels. Channel data will be output interleaved
		 *								in the output buffer.
		 */
		bool open(std::function<void(UINT8*, UINT32)> writeCallback, UINT32 sampleRate, UINT32 bitDepth, UINT32 numChannels);

		/**
		 * Writes a new set of samples and converts them to Ogg Vorbis.
		 *
		 * @param[in]	samples		Samples in PCM format. 8-bit samples should be unsigned, but higher bit depths signed.
		 *							Each sample is assumed to be the bit depth that was provided to the open() method.
		 * @param[in]	numSamples	Number of samples to encode.
		 */
		void write(UINT8* samples, UINT32 numSamples);

		/**
		 * Flushes the last of the data into the write buffer (triggers the write callback). This is called automatically
		 * when the writer is closed or goes out of scope.
		 */
		void flush();

		/**
		 * Closes the encoder and flushes the last of the data into the write buffer (triggers the write callback). This is
		 * called automatically when the writer goes out of scope.
		 */
		void close();

		/**
		 * Helper method that allows you to quickly convert PCM to Ogg Vorbis data.
		 *
		 * @param[in]	samples		Buffer containing samples in PCM format. All samples should be in signed integer format.
		 * @param[in]	info		Meta-data describing the provided samples.
		 * @param[out]	size		Number of bytes written to the output buffer.
		 * @return					Memory data stream containing the encoded samples.
		 */
		static SPtr<MemoryDataStream> PCMToOggVorbis(UINT8* samples, const AudioDataInfo& info, UINT32& size);
	private:
		/** Writes Vorbis blocks into Ogg packets. */
		void writeBlocks();

		static const UINT32 BUFFER_SIZE = 4096;

		std::function<void(UINT8*, UINT32)> mWriteCallback;
		UINT8 mBuffer[BUFFER_SIZE];
		UINT32 mBufferOffset = 0;
		UINT32 mNumChannels = 0;
		UINT32 mBitDepth = 0;
		bool mClosed = true;

		ogg_stream_state mOggState;
		vorbis_info mVorbisInfo;
		vorbis_dsp_state mVorbisState;
		vorbis_block mVorbisBlock;
	};

	/** @} */
}

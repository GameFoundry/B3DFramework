//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsOAPrerequisites.h"
#include "vorbis\vorbisenc.h"

namespace BansheeEngine
{
	/** @addtogroup OpenAudio
	 *  @{
	 */

	/** Used for writing Ogg Vorbis audio data. */
	class OAOggVorbisWriter
	{
	public:
		OAOggVorbisWriter();
		~OAOggVorbisWriter();

		bool open(std::function<void(UINT8*, UINT32)> writeCallback, UINT32 sampleRate, UINT32 bitDepth, UINT32 numChannels);

		void write(UINT8* samples, UINT32 numSamples); // Assumes 16-bit samples
		void flush();
		void close();
	private:
		void writeBlocks();

		static const UINT32 BUFFER_SIZE = 4096;

		std::function<void(UINT8*, UINT32)> mWriteCallback;
		UINT8 mBuffer[BUFFER_SIZE];
		UINT32 mBufferOffset;
		UINT32 mNumChannels;
		UINT32 mBitDepth;

		ogg_stream_state mOggState;
		vorbis_info mVorbisInfo;
		vorbis_dsp_state mVorbisState;
	};

	/** @} */
}
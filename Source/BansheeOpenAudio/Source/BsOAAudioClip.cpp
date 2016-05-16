//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsOAAudioClip.h"
#include "BsOAOggVorbisWriter.h"
#include "BsDataStream.h"

namespace BansheeEngine
{
	OAAudioClip::OAAudioClip(const SPtr<DataStream>& samples, UINT32 streamSize, UINT32 numSamples, const AUDIO_CLIP_DESC& desc)
		:AudioClip(samples, streamSize, numSamples, desc)
	{ }

	void OAAudioClip::initialize()
	{
		if (mDesc.readMode != AudioReadMode::Stream)
		{
			// If not streaming, but input stream is a file, read all of it into an internal memory stream
			if (mStreamData != nullptr)
			{
				// Decompress if needed
				if (mDesc.format == AudioFormat::VORBIS && mDesc.readMode == AudioReadMode::LoadDecompressed)
				{
					// TODO - Read entire stream and decompress into a new stream
				}
				else
				{
					if(mStreamData->isFile()) // If reading from file, make a copy of data in memory
						mStreamData = bs_shared_ptr_new<MemoryDataStream>(mStreamData);
				}
			}
		}

		AudioClip::initialize();
	}

	void OAAudioClip::getSamples(UINT8* samples, UINT32 count, UINT32 offset) const
	{
		// TODO - Read from stream, and decompress as needed

		// TODO - This must be thread safe as it will be called from other threads. Note this in the docs.
	}

	SPtr<DataStream> OAAudioClip::getSourceFormatData(UINT32& size)
	{
		if(mDesc.format == AudioFormat::PCM || mDesc.readMode == AudioReadMode::LoadCompressed || 
			mDesc.readMode == AudioReadMode::Stream)
		{
			size = mStreamSize;
			return mStreamData;
		}
		else // Data has been decompressed, we need to re-compress it to get the original stream
		{
			if (mStreamData == nullptr)
			{
				size = 0;
				return nullptr;
			}

			assert(!mStreamData->isFile());
			SPtr<MemoryDataStream> memStream = std::static_pointer_cast<MemoryDataStream>(mStreamData);

			AudioFileInfo info;
			info.bitDepth = mDesc.bitDepth;
			info.numChannels = mDesc.numChannels;
			info.numSamples = mNumSamples;
			info.sampleRate = mDesc.frequency;

			UINT32 bufferSize = 0;
			UINT8* encodedSamples = OAOggVorbisWriter::PCMToOggVorbis(memStream->getPtr(), info, bufferSize);

			return bs_shared_ptr_new<MemoryDataStream>(encodedSamples, bufferSize);
		}
	}
}
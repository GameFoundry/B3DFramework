//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsOAAudioClip.h"
#include "BsOggVorbisEncoder.h"
#include "BsOggVorbisDecoder.h"
#include "FileSystem/BsDataStream.h"
#include "BsOAAudio.h"
#include "AL/al.h"

namespace bs
{
	OAAudioClip::OAAudioClip(const SPtr<DataStream>& samples, UINT32 streamSize, UINT32 numSamples, const AUDIO_CLIP_DESC& desc)
		:AudioClip(samples, streamSize, numSamples, desc)
	{ }

	OAAudioClip::~OAAudioClip()
	{
		if (mBufferId != (UINT32)-1)
			alDeleteBuffers(1, &mBufferId);
	}

	void OAAudioClip::initialize()
	{
		{
			Lock lock(mMutex); // Needs to be called even if stream data is null, to ensure memory fence is added so the
							   // other thread sees properly initialized AudioClip members

			AudioDataInfo info;
			info.bitDepth = mDesc.bitDepth;
			info.numChannels = mDesc.numChannels;
			info.numSamples = mNumSamples;
			info.sampleRate = mDesc.frequency;

			// If we need to keep source data, read everything into memory and keep a copy
			if (mKeepSourceData)
			{
				mStreamData->seek(mStreamOffset);

				UINT8* sampleBuffer = (UINT8*)bs_alloc(mStreamSize);
				mStreamData->read(sampleBuffer, mStreamSize);

				mSourceStreamData = bs_shared_ptr_new<MemoryDataStream>(sampleBuffer, mStreamSize);
				mSourceStreamSize = mStreamSize;
			}

			// Load decompressed data into a sound buffer
			bool loadDecompressed = 
				mDesc.readMode == AudioReadMode::LoadDecompressed || 
				(mDesc.readMode == AudioReadMode::LoadCompressed && mDesc.format == AudioFormat::PCM);

			if(loadDecompressed)
			{
				// Read all data into memory
				SPtr<DataStream> stream;
				UINT32 offset = 0;
				if (mSourceStreamData != nullptr) // If it's already loaded in memory, use it directly
					stream = mSourceStreamData;
				else
				{
					stream = mStreamData;
					offset = mStreamOffset;
				}

				UINT32 bufferSize = info.numSamples * (info.bitDepth / 8);
				UINT8* sampleBuffer = (UINT8*)bs_stack_alloc(bufferSize);

				// Decompress from Ogg
				if (mDesc.format == AudioFormat::VORBIS)
				{
					OggVorbisDecoder reader;
					if (reader.open(stream, info, offset))
						reader.read(sampleBuffer, info.numSamples);
					else
						LOGERR("Failed decompressing AudioClip stream.");
				}
				// Load directly
				else
				{
					stream->seek(offset);
					stream->read(sampleBuffer, bufferSize);
				}

				alGenBuffers(1, &mBufferId);
				gOAAudio()._writeToOpenALBuffer(mBufferId, sampleBuffer, info);

				mStreamData = nullptr;
				mStreamOffset = 0;
				mStreamSize = 0;

				bs_stack_free(sampleBuffer);
			}
			// Load compressed data for streaming from memory
			else if(mDesc.readMode == AudioReadMode::LoadCompressed)
			{
				// If reading from file, make a copy of data in memory, otherwise just take ownership of the existing buffer
				if (mStreamData->isFile()) 
				{
					if (mSourceStreamData != nullptr) // If it's already loaded in memory, use it directly
						mStreamData = mSourceStreamData;
					else
					{
						UINT8* data = (UINT8*)bs_alloc(mStreamSize);

						mStreamData->seek(mStreamOffset);
						mStreamData->read(data, mStreamSize);

						mStreamData = bs_shared_ptr_new<MemoryDataStream>(data, mStreamSize);
					}

					mStreamOffset = 0;
				}
			}
			// Keep original stream for streaming from file
			else
			{
				// Do nothing
			}

			if (mDesc.format == AudioFormat::VORBIS && mDesc.readMode != AudioReadMode::LoadDecompressed)
			{
				mNeedsDecompression = true;

				if (mStreamData != nullptr)
				{
					if (!mVorbisReader.open(mStreamData, info, mStreamOffset))
						LOGERR("Failed decompressing AudioClip stream.");
				}
			}
		}

		AudioClip::initialize();
	}

	void OAAudioClip::getSamples(UINT8* samples, UINT32 offset, UINT32 count) const
	{
		Lock lock(mMutex);

		// Try to read from normal stream, and if that fails read from in-memory stream if it exists
		if (mStreamData != nullptr)
		{
			if (mNeedsDecompression)
			{
				mVorbisReader.seek(offset);
				mVorbisReader.read(samples, count);
			}
			else
			{
				UINT32 bytesPerSample = mDesc.bitDepth / 8;
				UINT32 size = count * bytesPerSample;
				UINT32 streamOffset = mStreamOffset + offset * bytesPerSample;

				mStreamData->seek(streamOffset);
				mStreamData->read(samples, size);
			}

			return;
		}

		if (mSourceStreamData != nullptr)
		{
			assert(!mNeedsDecompression); // Normal stream must exist if decompressing

			const UINT32 bytesPerSample = mDesc.bitDepth / 8;
			UINT32 size = count * bytesPerSample;
			UINT32 streamOffset = offset * bytesPerSample;

			mSourceStreamData->seek(streamOffset);
			mSourceStreamData->read(samples, size);
			return;
		}

		LOGWRN("Attempting to read samples while sample data is not available.");
	}

	SPtr<DataStream> OAAudioClip::getSourceStream(UINT32& size)
	{
		Lock lock(mMutex);

		size = mSourceStreamSize;
		mSourceStreamData->seek(0);

		return mSourceStreamData;
	}
}
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
	OAAudioClip::OAAudioClip(const SPtr<DataStream>& samples, u32 streamSize, u32 numSamples, const AUDIO_CLIP_DESC& desc)
		: AudioClip(samples, streamSize, numSamples, desc)
	{}

	OAAudioClip::~OAAudioClip()
	{
		if(mBufferId != (u32)-1)
			alDeleteBuffers(1, &mBufferId);
	}

	void OAAudioClip::Initialize()
	{
		{
			Lock lock(mMutex); // Needs to be called even if stream data is null, to ensure memory fence is added so the
							   // other thread sees properly initialized AudioClip members

			AudioDataInfo info;
			info.BitDepth = mDesc.BitDepth;
			info.NumChannels = mDesc.NumChannels;
			info.NumSamples = mNumSamples;
			info.SampleRate = mDesc.Frequency;

			// If we need to keep source data, read everything into memory and keep a copy
			if(mKeepSourceData)
			{
				mStreamData->Seek(mStreamOffset);

				auto memStream = bs_shared_ptr_new<MemoryDataStream>(mStreamSize);
				mSourceStreamData = memStream;

				mStreamData->Read(memStream->Data(), mStreamSize);
				mSourceStreamSize = mStreamSize;
			}

			// Load decompressed data into a sound buffer
			bool loadDecompressed =
				mDesc.ReadMode == AudioReadMode::LoadDecompressed ||
				(mDesc.ReadMode == AudioReadMode::LoadCompressed && mDesc.Format == AudioFormat::PCM);

			if(loadDecompressed)
			{
				// Read all data into memory
				SPtr<DataStream> stream;
				u32 offset = 0;
				if(mSourceStreamData != nullptr) // If it's already loaded in memory, use it directly
					stream = mSourceStreamData;
				else
				{
					stream = mStreamData;
					offset = mStreamOffset;
				}

				u32 bufferSize = info.NumSamples * (info.BitDepth / 8);
				u8* sampleBuffer = (u8*)bs_stack_alloc(bufferSize);

				// Decompress from Ogg
				if(mDesc.Format == AudioFormat::VORBIS)
				{
					OggVorbisDecoder reader;
					if(reader.Open(stream, info, offset))
						reader.Read(sampleBuffer, info.NumSamples);
					else
						BS_LOG(Error, Audio, "Failed decompressing AudioClip stream.");
				}
				// Load directly
				else
				{
					stream->Seek(offset);
					stream->Read(sampleBuffer, bufferSize);
				}

				alGenBuffers(1, &mBufferId);
				gOAAudio().WriteToOpenALBufferInternal(mBufferId, sampleBuffer, info);

				mStreamData = nullptr;
				mStreamOffset = 0;
				mStreamSize = 0;

				bs_stack_free(sampleBuffer);
			}
			// Load compressed data for streaming from memory
			else if(mDesc.ReadMode == AudioReadMode::LoadCompressed)
			{
				// If reading from file, make a copy of data in memory, otherwise just take ownership of the existing buffer
				if(mStreamData->IsFile())
				{
					if(mSourceStreamData != nullptr) // If it's already loaded in memory, use it directly
						mStreamData = mSourceStreamData;
					else
					{
						auto memStream = bs_shared_ptr_new<MemoryDataStream>(mStreamSize);

						mStreamData->Seek(mStreamOffset);
						mStreamData->Read(memStream->Data(), mStreamSize);

						mStreamData = memStream;
					}

					mStreamOffset = 0;
				}
			}
			// Keep original stream for streaming from file
			else
			{
				// Do nothing
			}

			if(mDesc.Format == AudioFormat::VORBIS && mDesc.ReadMode != AudioReadMode::LoadDecompressed)
			{
				mNeedsDecompression = true;

				if(mStreamData != nullptr)
				{
					if(!mVorbisReader.Open(mStreamData, info, mStreamOffset))
						BS_LOG(Error, Audio, "Failed decompressing AudioClip stream.");
				}
			}
		}

		AudioClip::Initialize();
	}

	void OAAudioClip::GetSamples(u8* samples, u32 offset, u32 count) const
	{
		Lock lock(mMutex);

		// Try to read from normal stream, and if that fails read from in-memory stream if it exists
		if(mStreamData != nullptr)
		{
			if(mNeedsDecompression)
			{
				mVorbisReader.Seek(offset);
				mVorbisReader.Read(samples, count);
			}
			else
			{
				u32 bytesPerSample = mDesc.BitDepth / 8;
				u32 size = count * bytesPerSample;
				u32 streamOffset = mStreamOffset + offset * bytesPerSample;

				mStreamData->Seek(streamOffset);
				mStreamData->Read(samples, size);
			}

			return;
		}

		if(mSourceStreamData != nullptr)
		{
			assert(!mNeedsDecompression); // Normal stream must exist if decompressing

			const u32 bytesPerSample = mDesc.BitDepth / 8;
			u32 size = count * bytesPerSample;
			u32 streamOffset = offset * bytesPerSample;

			mSourceStreamData->Seek(streamOffset);
			mSourceStreamData->Read(samples, size);
			return;
		}

		BS_LOG(Warning, RenderBackend, "Attempting to read samples while sample data is not available.");
	}

	SPtr<DataStream> OAAudioClip::GetSourceStream(u32& size)
	{
		Lock lock(mMutex);

		size = mSourceStreamSize;
		mSourceStreamData->Seek(0);

		return mSourceStreamData;
	}
} // namespace bs

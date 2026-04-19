//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DOAAudioClip.h"
#include "B3DOggVorbisEncoder.h"
#include "B3DOggVorbisDecoder.h"
#include "FileSystem/B3DDataStream.h"
#include "B3DOAAudio.h"
#include "AL/al.h"

using namespace b3d;

OAAudioClip::OAAudioClip(const SPtr<DataStream>& samples, u32 streamSize, u32 sampleCount, const AudioClipCreateInformation& createInformation)
	: AudioClip(samples, streamSize, sampleCount, createInformation)
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
		info.BitDepth = mInformation.BitDepth;
		info.ChannelCount = mInformation.ChannelCount;
		info.SampleCount = mSampleCount;
		info.SampleRate = mInformation.Frequency;

		// If we need to keep source data, read everything into memory and keep a copy
		if(mKeepSourceData)
		{
			mStreamData->Seek(mStreamOffset);

			auto memStream = B3DMakeShared<MemoryDataStream>(mStreamSize);
			mSourceStreamData = memStream;

			mStreamData->Read(memStream->Data(), mStreamSize);
			mSourceStreamSize = mStreamSize;
		}

		// Load decompressed data into a sound buffer
		bool loadDecompressed =
			mInformation.ReadMode == AudioReadMode::LoadDecompressed ||
			(mInformation.ReadMode == AudioReadMode::LoadCompressed && mInformation.Format == AudioFormat::PCM);

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

			u32 bufferSize = info.SampleCount * (info.BitDepth / 8);
			u8* sampleBuffer = (u8*)B3DStackAllocate(bufferSize);

			// Decompress from Ogg
			if(mInformation.Format == AudioFormat::VORBIS)
			{
				OggVorbisDecoder reader;
				if(reader.Open(stream, info, offset))
					reader.Read(sampleBuffer, info.SampleCount);
				else
					B3D_LOG(Error, LogAudio, "Failed decompressing AudioClip stream.");
			}
			// Load directly
			else
			{
				stream->Seek(offset);
				stream->Read(sampleBuffer, bufferSize);
			}

			alGenBuffers(1, &mBufferId);
			GetOAAudio().WriteToOpenALBuffer(mBufferId, sampleBuffer, info);

			mStreamData = nullptr;
			mStreamOffset = 0;
			mStreamSize = 0;

			B3DStackFree(sampleBuffer);
		}
		// Load compressed data for streaming from memory
		else if(mInformation.ReadMode == AudioReadMode::LoadCompressed)
		{
			// If reading from file, make a copy of data in memory, otherwise just take ownership of the existing buffer
			if(mStreamData->IsFile())
			{
				if(mSourceStreamData != nullptr) // If it's already loaded in memory, use it directly
					mStreamData = mSourceStreamData;
				else
				{
					auto memStream = B3DMakeShared<MemoryDataStream>(mStreamSize);

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

		if(mInformation.Format == AudioFormat::VORBIS && mInformation.ReadMode != AudioReadMode::LoadDecompressed)
		{
			mNeedsDecompression = true;

			if(mStreamData != nullptr)
			{
				if(!mVorbisReader.Open(mStreamData, info, mStreamOffset))
					B3D_LOG(Error, LogAudio, "Failed decompressing AudioClip stream.");
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
			u32 bytesPerSample = mInformation.BitDepth / 8;
			u32 size = count * bytesPerSample;
			u32 streamOffset = mStreamOffset + offset * bytesPerSample;

			mStreamData->Seek(streamOffset);
			mStreamData->Read(samples, size);
		}

		return;
	}

	if(mSourceStreamData != nullptr)
	{
		B3D_ASSERT(!mNeedsDecompression); // Normal stream must exist if decompressing

		const u32 bytesPerSample = mInformation.BitDepth / 8;
		u32 size = count * bytesPerSample;
		u32 streamOffset = offset * bytesPerSample;

		mSourceStreamData->Seek(streamOffset);
		mSourceStreamData->Read(samples, size);
		return;
	}

	B3D_LOG(Warning, LogRenderBackend, "Attempting to read samples while sample data is not available.");
}

SPtr<DataStream> OAAudioClip::GetSourceStream(u32& outSize)
{
	Lock lock(mMutex);

	outSize = mSourceStreamSize;
	mSourceStreamData->Seek(0);

	return mSourceStreamData;
}

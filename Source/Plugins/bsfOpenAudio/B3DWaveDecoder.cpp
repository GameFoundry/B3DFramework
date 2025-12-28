//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DWaveDecoder.h"
#include "FileSystem/B3DDataStream.h"

using namespace b3d;

#define WAVE_FORMAT_PCM 0x0001
#define WAVE_FORMAT_EXTENDED 0xFFFE

bool WaveDecoder::IsValid(const SPtr<DataStream>& stream, u32 offset)
{
	stream->Seek(offset);

	i8 header[kMainChunkSize];
	if(stream->Read(header, sizeof(header)) < (sizeof(header)))
		return false;

	return (header[0] == 'R') && (header[1] == 'I') && (header[2] == 'F') && (header[3] == 'F') && (header[8] == 'W') && (header[9] == 'A') && (header[10] == 'V') && (header[11] == 'E');
}

bool WaveDecoder::Open(const SPtr<DataStream>& stream, AudioDataInfo& info, u32 offset)
{
	if(stream == nullptr)
		return false;

	mStream = stream;
	mStream->Seek(offset + kMainChunkSize);

	if(!ParseHeader(info))
	{
		B3D_LOG(Error, LogAudio, "Provided file is not a valid WAVE file.");
		return false;
	}

	return true;
}

void WaveDecoder::Seek(u32 offset)
{
	mStream->Seek(mDataOffset + offset * mBytesPerSample);
}

u32 WaveDecoder::Read(u8* samples, u32 numSamples)
{
	u32 numRead = (u32)mStream->Read(samples, numSamples * mBytesPerSample);

	if(mBytesPerSample == 1) // 8-bit samples are stored as unsigned, but engine convention is to store all bit depths as signed
	{
		for(u32 i = 0; i < numRead; i++)
		{
			i8 val = samples[i] - 128;
			samples[i] = *((u8*)&val);
		}
	}

	return numRead;
}

bool WaveDecoder::ParseHeader(AudioDataInfo& info)
{
	bool foundData = false;
	while(!foundData)
	{
		// Get sub-chunk ID and size
		u8 subChunkId[4];
		if(mStream->Read(subChunkId, sizeof(subChunkId)) != sizeof(subChunkId))
			return false;

		u32 subChunkSize = 0;
		if(mStream->Read(&subChunkSize, sizeof(subChunkSize)) != sizeof(subChunkSize))
			return false;

		// FMT chunk
		if(subChunkId[0] == 'f' && subChunkId[1] == 'm' && subChunkId[2] == 't' && subChunkId[3] == ' ')
		{
			u16 format = 0;
			if(mStream->Read(&format, sizeof(format)) != sizeof(format))
				return false;

			if(format != WAVE_FORMAT_PCM && format != WAVE_FORMAT_EXTENDED)
			{
				B3D_LOG(Warning, LogAudio, "Wave file doesn't contain raw PCM data. Not supported.");
				return false;
			}

			u16 channelCount = 0;
			if(mStream->Read(&channelCount, sizeof(channelCount)) != sizeof(channelCount))
				return false;

			u32 sampleRate = 0;
			if(mStream->Read(&sampleRate, sizeof(sampleRate)) != sizeof(sampleRate))
				return false;

			u32 byteRate = 0;
			if(mStream->Read(&byteRate, sizeof(byteRate)) != sizeof(byteRate))
				return false;

			u16 blockAlign = 0;
			if(mStream->Read(&blockAlign, sizeof(blockAlign)) != sizeof(blockAlign))
				return false;

			u16 bitDepth = 0;
			if(mStream->Read(&bitDepth, sizeof(bitDepth)) != sizeof(bitDepth))
				return false;

			info.ChannelCount = channelCount;
			info.SampleRate = sampleRate;
			info.BitDepth = bitDepth;

			if(bitDepth != 8 && bitDepth != 16 && bitDepth != 24 && bitDepth != 32)
			{
				B3D_LOG(Error, LogAudio, "Unsupported number of bits per sample: {0}", bitDepth);
				return false;
			}

			// Read extension data, and get the actual format
			if(format == WAVE_FORMAT_EXTENDED)
			{
				u16 extensionSize = 0;
				if(mStream->Read(&extensionSize, sizeof(extensionSize)) != sizeof(extensionSize))
					return false;

				if(extensionSize != 22)
				{
					B3D_LOG(Warning, LogAudio, "Wave file doesn't contain raw PCM data. Not supported.");
					return false;
				}

				u16 validBitDepth = 0;
				if(mStream->Read(&validBitDepth, sizeof(validBitDepth)) != sizeof(validBitDepth))
					return false;

				u32 channelMask = 0;
				if(mStream->Read(&channelMask, sizeof(channelMask)) != sizeof(channelMask))
					return false;

				u8 subFormat[16];
				if(mStream->Read(subFormat, sizeof(subFormat)) != sizeof(subFormat))
					return false;

				memcpy(&format, subFormat, sizeof(format));
				if(format != WAVE_FORMAT_PCM)
				{
					B3D_LOG(Warning, LogAudio, "Wave file doesn't contain raw PCM data. Not supported.");
					return false;
				}
			}

			mBytesPerSample = bitDepth / 8;
		}
		// DATA chunk
		else if(subChunkId[0] == 'd' && subChunkId[1] == 'a' && subChunkId[2] == 't' && subChunkId[3] == 'a')
		{
			info.SampleCount = subChunkSize / mBytesPerSample;
			mDataOffset = (u32)mStream->Tell();

			foundData = true;
		}
		// Unsupported chunk type
		else
		{
			mStream->Skip(subChunkSize);
			if(mStream->Eof())
				return false;
		}
	}

	return true;
}

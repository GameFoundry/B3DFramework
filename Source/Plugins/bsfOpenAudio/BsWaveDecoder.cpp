//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsWaveDecoder.h"
#include "FileSystem/BsDataStream.h"

namespace bs
{
#define WAVE_FORMAT_PCM			0x0001
#define WAVE_FORMAT_EXTENDED	0xFFFE

	bool WaveDecoder::isValid(const SPtr<DataStream>& stream, UINT32 offset)
	{
		stream->seek(offset);

		INT8 header[MAIN_CHUNK_SIZE];
		if (stream->read(header, sizeof(header)) < (sizeof(header)))
			return false;

		return (header[0] == 'R') && (header[1] == 'I') && (header[2] == 'F') && (header[3] == 'F')
			&& (header[8] == 'W') && (header[9] == 'A') && (header[10] == 'V') && (header[11] == 'E');
	}

	bool WaveDecoder::open(const SPtr<DataStream>& stream, AudioDataInfo& info, UINT32 offset)
	{
		if (stream == nullptr)
			return false;

		mStream = stream;
		mStream->seek(offset + MAIN_CHUNK_SIZE);
		
		if (!parseHeader(info))
		{
			LOGERR("Provided file is not a valid WAVE file.");
			return false;
		}

		return true;
	}

	void WaveDecoder::seek(UINT32 offset)
	{
		mStream->seek(mDataOffset + offset * mBytesPerSample);
	}

	UINT32 WaveDecoder::read(UINT8* samples, UINT32 numSamples)
	{
		UINT32 numRead = (UINT32)mStream->read(samples, numSamples * mBytesPerSample);

		if(mBytesPerSample == 1) // 8-bit samples are stored as unsigned, but engine convention is to store all bit depths as signed
		{
			for(UINT32 i = 0; i < numRead; i++)
			{
				INT8 val = samples[i] - 128;
				samples[i] = *((UINT8*)&val);
			}
		}

		return numRead;
	}

	bool WaveDecoder::parseHeader(AudioDataInfo& info)
	{
		bool foundData = false;
		while (!foundData)
		{
			// Get sub-chunk ID and size
			UINT8 subChunkId[4];
			if (mStream->read(subChunkId, sizeof(subChunkId)) != sizeof(subChunkId))
				return false;

			UINT32 subChunkSize = 0;
			if (mStream->read(&subChunkSize, sizeof(subChunkSize)) != sizeof(subChunkSize))
				return false;

			// FMT chunk
			if (subChunkId[0] == 'f' && subChunkId[1] == 'm' && subChunkId[2] == 't' && subChunkId[3] == ' ')
			{
				UINT16 format = 0;
				if (mStream->read(&format, sizeof(format)) != sizeof(format))
					return false;

				if (format != WAVE_FORMAT_PCM && format != WAVE_FORMAT_EXTENDED)
				{
					LOGWRN("Wave file doesn't contain raw PCM data. Not supported.");
					return false;
				}

				UINT16 numChannels = 0;
				if (mStream->read(&numChannels, sizeof(numChannels)) != sizeof(numChannels))
					return false;

				UINT32 sampleRate = 0;
				if (mStream->read(&sampleRate, sizeof(sampleRate)) != sizeof(sampleRate))
					return false;

				UINT32 byteRate = 0;
				if (mStream->read(&byteRate, sizeof(byteRate)) != sizeof(byteRate))
					return false;

				UINT16 blockAlign = 0;
				if (mStream->read(&blockAlign, sizeof(blockAlign)) != sizeof(blockAlign))
					return false;

				UINT16 bitDepth = 0;
				if (mStream->read(&bitDepth, sizeof(bitDepth)) != sizeof(bitDepth))
					return false;

				info.numChannels = numChannels;
				info.sampleRate = sampleRate;
				info.bitDepth = bitDepth;

				if (bitDepth != 8 && bitDepth != 16 && bitDepth != 24 && bitDepth != 32)
				{
					LOGERR("Unsupported number of bits per sample: " + toString(bitDepth));
					return false;
				}

				// Read extension data, and get the actual format
				if(format == WAVE_FORMAT_EXTENDED)
				{
					UINT16 extensionSize = 0;
					if (mStream->read(&extensionSize, sizeof(extensionSize)) != sizeof(extensionSize))
						return false;

					if(extensionSize != 22)
					{
						LOGWRN("Wave file doesn't contain raw PCM data. Not supported.");
						return false;
					}

					UINT16 validBitDepth = 0;
					if (mStream->read(&validBitDepth, sizeof(validBitDepth)) != sizeof(validBitDepth))
						return false;

					UINT32 channelMask = 0;
					if (mStream->read(&channelMask, sizeof(channelMask)) != sizeof(channelMask))
						return false;

					UINT8 subFormat[16];
					if (mStream->read(subFormat, sizeof(subFormat)) != sizeof(subFormat))
						return false;

					memcpy(&format, subFormat, sizeof(format));
					if (format != WAVE_FORMAT_PCM)
					{
						LOGWRN("Wave file doesn't contain raw PCM data. Not supported.");
						return false;
					}
				}

				mBytesPerSample = bitDepth / 8;
			}
			// DATA chunk
			else if (subChunkId[0] == 'd' && subChunkId[1] == 'a' && subChunkId[2] == 't' && subChunkId[3] == 'a')
			{
				info.numSamples = subChunkSize / mBytesPerSample;
				mDataOffset = (UINT32)mStream->tell();

				foundData = true;
			}
			// Unsupported chunk type
			else
			{
				mStream->skip(subChunkSize);
				if (mStream->eof())
					return false;
			}
		}

		return true;
	}
}
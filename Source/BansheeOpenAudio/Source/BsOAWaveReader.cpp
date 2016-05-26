//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsOAWaveReader.h"
#include "BsDataStream.h"

namespace BansheeEngine
{
	OAWaveReader::OAWaveReader()
		:mDataOffset(0), mBytesPerSample(0)
	{ }

	bool OAWaveReader::isValid(const SPtr<DataStream>& stream)
	{
		stream->seek(0);

		INT8 header[MAIN_CHUNK_SIZE];
		if (stream->read(header, sizeof(header)) < (sizeof(header)))
			return false;

		return (header[0] == 'R') && (header[1] == 'I') && (header[2] == 'F') && (header[3] == 'F')
			&& (header[8] == 'W') && (header[9] == 'A') && (header[10] == 'V') && (header[11] == 'E');
	}

	bool OAWaveReader::open(const SPtr<DataStream>& stream, AudioFileInfo& info)
	{
		if (stream == nullptr)
			return false;

		mStream = stream;
		mStream->seek(MAIN_CHUNK_SIZE);

		if (!parseHeader(info))
		{
			LOGERR("Provided file is not a valid WAVE file.");
			return false;
		}

		return true;
	}

	void OAWaveReader::seek(UINT32 offset)
	{
		mStream->seek(mDataOffset + offset * mBytesPerSample);
	}

	UINT32 OAWaveReader::read(UINT8* samples, UINT32 numSamples)
	{
		return (UINT32)mStream->read(samples, numSamples * mBytesPerSample);
	}

	bool OAWaveReader::parseHeader(AudioFileInfo& info)
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

				if (format != 1) // It's not PCM
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

				mBytesPerSample = bitDepth / 8;

				// Skip extra data, if any
				if (subChunkSize > 16)
				{
					mStream->skip(subChunkSize - 16);
					if (mStream->eof())
						return false;
				}
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
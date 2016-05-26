//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsOAOggVorbisReader.h"
#include "BsDataStream.h"
#include <vorbis\codec.h>

namespace BansheeEngine
{
	size_t oggRead(void* ptr, size_t size, size_t nmemb, void* data)
	{
		DataStream* stream = static_cast<DataStream*>(data);
		return static_cast<std::size_t>(stream->read(ptr, size * nmemb));
	}

	int oggSeek(void* data, ogg_int64_t offset, int whence)
	{
		DataStream* stream = static_cast<DataStream*>(data);
		switch (whence)
		{
		case SEEK_SET:
			break;
		case SEEK_CUR:
			offset += stream->tell();
			break;
		case SEEK_END:
			offset = stream->size() - offset;
		}

		stream->seek(offset);
		return (int)stream->tell();
	}

	long oggTell(void* data)
	{
		DataStream* stream = static_cast<DataStream*>(data);
		return static_cast<long>(stream->tell());
	}

	static ov_callbacks callbacks = { &oggRead, &oggSeek, nullptr, &oggTell };

	OAOggVorbisReader::OAOggVorbisReader()
		:mChannelCount(0)
	{ }

	OAOggVorbisReader::~OAOggVorbisReader()
	{
		if (mOggVorbisFile.datasource != nullptr)
			ov_clear(&mOggVorbisFile);
	}

	bool OAOggVorbisReader::isValid(const SPtr<DataStream>& stream)
	{
		stream->seek(0);

		OggVorbis_File file;
		if (ov_test_callbacks(stream.get(), &file, nullptr, 0, callbacks) == 0)
		{
			ov_clear(&file);
			return true;
		}
			
		return false;
	}

	bool OAOggVorbisReader::open(const SPtr<DataStream>& stream, AudioFileInfo& info)
	{
		if (stream == nullptr)
			return false;

		stream->seek(0);
		mStream = stream;

		int status = ov_open_callbacks(stream.get(), &mOggVorbisFile, nullptr, 0, callbacks);
		if (status < 0)
		{
			LOGERR("Failed to open Ogg Vorbis file.");
			return false;
		}

		vorbis_info* vorbisInfo = ov_info(&mOggVorbisFile, -1);
		info.numChannels = vorbisInfo->channels;
		info.sampleRate = vorbisInfo->rate;
		info.numSamples = (UINT32)(ov_pcm_total(&mOggVorbisFile, -1) * vorbisInfo->channels);
		info.bitDepth = 16;

		mChannelCount = info.numChannels;
		return true;
	}

	void OAOggVorbisReader::seek(UINT32 offset)
	{
		ov_pcm_seek(&mOggVorbisFile, offset / mChannelCount);
	}

	UINT32 OAOggVorbisReader::read(UINT8* samples, UINT32 numSamples)
	{
		UINT32 numReadSamples = 0;
		while (numReadSamples < numSamples)
		{
			INT32 bytesToRead = (INT32)(numSamples - numReadSamples) * sizeof(INT16);
			UINT32 bytesRead = ov_read(&mOggVorbisFile, (char*)samples, bytesToRead, 0, 2, 1, nullptr);
			if (bytesRead > 0)
			{
				UINT32 samplesRead = bytesRead / sizeof(INT16);
				numReadSamples += samplesRead;
				samples += samplesRead * sizeof(INT16);
			}
			else
				break;
		}

		return numReadSamples;
	}
}
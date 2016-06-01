//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsOggVorbisDecoder.h"
#include "BsDataStream.h"
#include <vorbis\codec.h>

namespace BansheeEngine
{
	size_t oggRead(void* ptr, size_t size, size_t nmemb, void* data)
	{
		OggDecoderData* decoderData = static_cast<OggDecoderData*>(data);
		return static_cast<std::size_t>(decoderData->stream->read(ptr, size * nmemb));
	}

	int oggSeek(void* data, ogg_int64_t offset, int whence)
	{
		OggDecoderData* decoderData = static_cast<OggDecoderData*>(data);
		switch (whence)
		{
		case SEEK_SET:
			offset += decoderData->offset;
			break;
		case SEEK_CUR:
			offset += decoderData->stream->tell();
			break;
		case SEEK_END:
			offset = std::max(0, (INT32)decoderData->stream->size() - 1);
			break;
		}

		decoderData->stream->seek(offset);
		return (int)(decoderData->stream->tell() - decoderData->offset);
	}

	long oggTell(void* data)
	{
		OggDecoderData* decoderData = static_cast<OggDecoderData*>(data);
		return (long)(decoderData->stream->tell() - decoderData->offset);
	}

	static ov_callbacks callbacks = { &oggRead, &oggSeek, nullptr, &oggTell };

	OggVorbisDecoder::OggVorbisDecoder()
		:mChannelCount(0)
	{
		mOggVorbisFile.datasource = nullptr;
	}

	OggVorbisDecoder::~OggVorbisDecoder()
	{
		if (mOggVorbisFile.datasource != nullptr)
			ov_clear(&mOggVorbisFile);
	}

	bool OggVorbisDecoder::isValid(const SPtr<DataStream>& stream, UINT32 offset)
	{
		stream->seek(offset);
		mDecoderData.stream = stream;
		mDecoderData.offset = offset;

		OggVorbis_File file;
		if (ov_test_callbacks(&mDecoderData, &file, nullptr, 0, callbacks) == 0)
		{
			ov_clear(&file);
			return true;
		}
			
		return false;
	}

	bool OggVorbisDecoder::open(const SPtr<DataStream>& stream, AudioDataInfo& info, UINT32 offset)
	{
		if (stream == nullptr)
			return false;

		stream->seek(offset);
		mDecoderData.stream = stream;
		mDecoderData.offset = offset;

		int status = ov_open_callbacks(&mDecoderData, &mOggVorbisFile, nullptr, 0, callbacks);
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

	void OggVorbisDecoder::seek(UINT32 offset)
	{
		ov_pcm_seek(&mOggVorbisFile, offset / mChannelCount);
	}

	UINT32 OggVorbisDecoder::read(UINT8* samples, UINT32 numSamples)
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
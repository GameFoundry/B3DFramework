//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DOggVorbisDecoder.h"
#include "FileSystem/B3DDataStream.h"
#include <vorbis/codec.h>

using namespace b3d;

size_t OggRead(void* ptr, size_t size, size_t nmemb, void* data)
{
	OggDecoderData* decoderData = static_cast<OggDecoderData*>(data);
	return static_cast<std::size_t>(decoderData->Stream->Read(ptr, size * nmemb));
}

int OggSeek(void* data, ogg_int64_t offset, int whence)
{
	OggDecoderData* decoderData = static_cast<OggDecoderData*>(data);
	switch(whence)
	{
	case SEEK_SET:
		offset += decoderData->Offset;
		break;
	case SEEK_CUR:
		offset += decoderData->Stream->Tell();
		break;
	case SEEK_END:
		offset = std::max(0, (i32)decoderData->Stream->Size() - 1);
		break;
	}

	decoderData->Stream->Seek((u32)offset);
	return (int)(decoderData->Stream->Tell() - decoderData->Offset);
}

long OggTell(void* data)
{
	OggDecoderData* decoderData = static_cast<OggDecoderData*>(data);
	return (long)(decoderData->Stream->Tell() - decoderData->Offset);
}

static ov_callbacks callbacks = { &OggRead, &OggSeek, nullptr, &OggTell };

OggVorbisDecoder::OggVorbisDecoder()
{
	mOggVorbisFile.datasource = nullptr;
}

OggVorbisDecoder::~OggVorbisDecoder()
{
	if(mOggVorbisFile.datasource != nullptr)
		ov_clear(&mOggVorbisFile);
}

bool OggVorbisDecoder::IsValid(const TShared<DataStream>& stream, u32 offset)
{
	stream->Seek(offset);
	mDecoderData.Stream = stream;
	mDecoderData.Offset = offset;

	OggVorbis_File file;
	if(ov_test_callbacks(&mDecoderData, &file, nullptr, 0, callbacks) == 0)
	{
		ov_clear(&file);
		return true;
	}

	return false;
}

bool OggVorbisDecoder::Open(const TShared<DataStream>& stream, AudioDataInfo& info, u32 offset)
{
	if(stream == nullptr)
		return false;

	stream->Seek(offset);
	mDecoderData.Stream = stream;
	mDecoderData.Offset = offset;

	int status = ov_open_callbacks(&mDecoderData, &mOggVorbisFile, nullptr, 0, callbacks);
	if(status < 0)
	{
		B3D_LOG(Error, LogAudio, "Failed to open Ogg Vorbis file.");
		return false;
	}

	vorbis_info* vorbisInfo = ov_info(&mOggVorbisFile, -1);
	info.ChannelCount = vorbisInfo->channels;
	info.SampleRate = vorbisInfo->rate;
	info.SampleCount = (u32)(ov_pcm_total(&mOggVorbisFile, -1) * vorbisInfo->channels);
	info.BitDepth = 16;

	mChannelCount = info.ChannelCount;
	return true;
}

void OggVorbisDecoder::Seek(u32 offset)
{
	ov_pcm_seek(&mOggVorbisFile, offset / mChannelCount);
}

u32 OggVorbisDecoder::Read(u8* samples, u32 numSamples)
{
	u32 numReadSamples = 0;
	while(numReadSamples < numSamples)
	{
		i32 bytesToRead = (i32)(numSamples - numReadSamples) * sizeof(i16);
		u32 bytesRead = ov_read(&mOggVorbisFile, (char*)samples, bytesToRead, 0, 2, 1, nullptr);
		if(bytesRead > 0)
		{
			u32 samplesRead = bytesRead / sizeof(i16);
			numReadSamples += samplesRead;
			samples += samplesRead * sizeof(i16);
		}
		else
			break;
	}

	return numReadSamples;
}

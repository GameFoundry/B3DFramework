//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DFMODAudioClip.h"
#include "B3DFMODAudio.h"
#include "FileSystem/B3DDataStream.h"

using namespace b3d;

FMOD_RESULT F_CALLBACK PCMReadCallback(FMOD_SOUND* sound, void* data, unsigned int dataLen)
{
	FMODOggDecompressorData* decompressor = nullptr;
	((FMOD::Sound*)sound)->getUserData((void**)&decompressor);

	const FMODAudioClip* clip = decompressor->Clip;
	u32 bytesPerSample = (clip->GetBitDepth() / 8);

	B3D_ASSERT(dataLen % bytesPerSample == 0);
	u32 numSamples = dataLen / bytesPerSample;

	decompressor->VorbisReader.Seek(decompressor->ReadPos);
	u32 readSamples = decompressor->VorbisReader.Read((u8*)data, numSamples);
	while(readSamples < numSamples) // Looping
	{
		decompressor->VorbisReader.Seek(0);

		u8* writePtr = (u8*)data;
		writePtr += readSamples * bytesPerSample;

		readSamples += decompressor->VorbisReader.Read(writePtr, numSamples - readSamples);
	}

	B3D_ASSERT(readSamples == numSamples);

	decompressor->ReadPos += readSamples;
	decompressor->ReadPos %= clip->GetSampleCount();

	return FMOD_OK;
}

FMOD_RESULT F_CALLBACK PCMSetPosCallback(FMOD_SOUND* sound, int subsound, unsigned int position, FMOD_TIMEUNIT posType)
{
	FMODOggDecompressorData* decompressor = nullptr;
	((FMOD::Sound*)sound)->getUserData((void**)&decompressor);

	const FMODAudioClip* clip = decompressor->Clip;
	u32 bytesPerSample = (clip->GetBitDepth() / 8);

	switch(posType)
	{
	case FMOD_TIMEUNIT_MS:
		decompressor->ReadPos = (u32)((clip->GetFrequency() * clip->GetChannelCount()) * (position / 1000.0f));
		break;
	case FMOD_TIMEUNIT_PCM:
		decompressor->ReadPos = clip->GetChannelCount() * position;
		break;
	case FMOD_TIMEUNIT_PCMBYTES:
		B3D_ASSERT(position % bytesPerSample == 0);
		decompressor->ReadPos = position / bytesPerSample;
		break;
	default:
		B3D_LOG(Error, LogAudio, "Invalid time unit.");
		break;
	}

	decompressor->ReadPos %= clip->GetSampleCount();
	decompressor->VorbisReader.Seek(decompressor->ReadPos);
	return FMOD_OK;
}

FMODAudioClip::FMODAudioClip(const TShared<DataStream>& samples, u32 streamSize, u32 sampleCount, const AudioClipCreateInformation& createInformation)
	: AudioClip(samples, streamSize, sampleCount, createInformation)
{}

FMODAudioClip::~FMODAudioClip()
{
	if(mSound != nullptr)
		mSound->release();
}

void FMODAudioClip::Initialize()
{
	AudioDataInfo info;
	info.BitDepth = mInformation.BitDepth;
	info.ChannelCount = mInformation.ChannelCount;
	info.SampleCount = mSampleCount;
	info.SampleRate = mInformation.Frequency;

	// If we need to keep source data, read everything into memory and keep a copy
	if(mKeepSourceData)
	{
		mStreamData->Seek(mStreamOffset);

		u8* sampleBuffer = (u8*)B3DAllocate(mStreamSize);
		mStreamData->Read(sampleBuffer, mStreamSize);

		mSourceStreamData = B3DMakeShared<MemoryDataStream>(sampleBuffer, mStreamSize);
		mSourceStreamSize = mStreamSize;
	}

	// If streaming is not required, create the sound right away
	if(!RequiresStreaming())
	{
		TShared<DataStream> stream;
		u32 offset = 0;
		if(mSourceStreamData != nullptr) // If it's already loaded in memory, use it directly
			stream = mSourceStreamData;
		else
		{
			stream = mStreamData;
			offset = mStreamOffset;
		}

		u32 bufferSize = info.SampleCount * (info.BitDepth / 8);

		FMOD_CREATESOUNDEXINFO exInfo;
		memset(&exInfo, 0, sizeof(exInfo));
		exInfo.cbsize = sizeof(exInfo);
		exInfo.length = bufferSize;

		FMOD_MODE flags = FMOD_CREATESAMPLE | FMOD_OPENMEMORY;

		if(Is3D())
			flags |= FMOD_3D;
		else
			flags |= FMOD_2D;

		if(mInformation.Format == AudioFormat::PCM)
		{
			flags |= FMOD_OPENRAW;

			switch(mInformation.BitDepth)
			{
			case 8:
				exInfo.format = FMOD_SOUND_FORMAT_PCM8;
				break;
			case 16:
				exInfo.format = FMOD_SOUND_FORMAT_PCM16;
				break;
			case 24:
				exInfo.format = FMOD_SOUND_FORMAT_PCM24;
				break;
			case 32:
				exInfo.format = FMOD_SOUND_FORMAT_PCM32;
				break;
			default:
				B3D_ASSERT(false);
				break;
			}

			exInfo.numchannels = mInformation.ChannelCount;
			exInfo.defaultfrequency = mInformation.Frequency;
		}

		u8* sampleBuffer = (u8*)B3DStackAllocate(bufferSize);
		stream->Seek(offset);
		stream->Read(sampleBuffer, bufferSize);

		FMOD::System* fmod = GetFMODAudio().GetFMOD();
		if(fmod->createSound((const char*)sampleBuffer, flags, &exInfo, &mSound) != FMOD_OK)
		{
			B3D_LOG(Error, LogAudio, "Failed creating sound.");
		}
		else
		{
			mSound->setMode(FMOD_LOOP_OFF);
		}

		mStreamData = nullptr;
		mStreamOffset = 0;
		mStreamSize = 0;

		B3DStackFree(sampleBuffer);
	}
	else // Streaming
	{
		// If reading from file, make a copy of data in memory, otherwise just take ownership of the existing buffer
		if(mInformation.ReadMode == AudioReadMode::LoadCompressed && mStreamData->IsFile())
		{
			if(mSourceStreamData != nullptr) // If it's already loaded in memory, use it directly
				mStreamData = mSourceStreamData;
			else
			{
				u8* data = (u8*)B3DAllocate(mStreamSize);

				mStreamData->Seek(mStreamOffset);
				mStreamData->Read(data, mStreamSize);

				mStreamData = B3DMakeShared<MemoryDataStream>(data, mStreamSize);
			}

			mStreamOffset = 0;
		}
	}

	AudioClip::Initialize();
}

FMOD::Sound* FMODAudioClip::CreateStreamingSound() const
{
	if(!RequiresStreaming() || mStreamData == nullptr)
	{
		B3D_LOG(Error, LogAudio, "Invalid audio stream data.");
		return nullptr;
	}

	FMOD_MODE flags = FMOD_CREATESTREAM;
	const char* streamData;

	FMOD_CREATESOUNDEXINFO exInfo;
	memset(&exInfo, 0, sizeof(exInfo));
	exInfo.cbsize = sizeof(exInfo);

	String pathStr;
	if(mStreamData->IsFile())
	{
		// Initialize() guarantees the data was loaded in memory if it's not streaming
		B3D_ASSERT(mInformation.ReadMode == AudioReadMode::Stream);

		exInfo.length = mStreamSize;
		exInfo.fileoffset = mStreamOffset;

		TShared<FileDataStream> fileStream = std::static_pointer_cast<FileDataStream>(mStreamData);
		pathStr = fileStream->GetPath().ToString();

		streamData = pathStr.c_str();
	}
	else
	{
		TShared<MemoryDataStream> memStream = std::static_pointer_cast<MemoryDataStream>(mStreamData);

		if(mInformation.ReadMode == AudioReadMode::Stream)
		{
			// Note: I could use FMOD_OPENMEMORY_POINT here to save on memory, but then the caller would need to make
			// sure the memory is not deallocated. I'm ignoring this for now as streaming from memory should be a rare
			// occurence (normally only in editor)
			flags |= FMOD_OPENMEMORY;

			memStream->Seek(mStreamOffset);
			streamData = (const char*)memStream->Cursor();

			exInfo.length = mStreamSize;
		}
		else // Load compressed
		{
			flags |= FMOD_OPENUSER;

			exInfo.decodebuffersize = mInformation.Frequency;
			exInfo.pcmreadcallback = PCMReadCallback;
			exInfo.pcmsetposcallback = PCMSetPosCallback;

			AudioDataInfo info;
			info.BitDepth = mInformation.BitDepth;
			info.ChannelCount = mInformation.ChannelCount;
			info.SampleCount = mSampleCount;
			info.SampleRate = mInformation.Frequency;

			FMODOggDecompressorData* decompressorData = B3DNew<FMODOggDecompressorData>();
			decompressorData->Clip = this;

			if(!decompressorData->VorbisReader.Open(memStream, info, mStreamOffset))
			{
				B3D_LOG(Error, LogAudio, "Failed decompressing AudioClip stream.");
				return nullptr;
			}

			exInfo.userdata = decompressorData;
			exInfo.length = mSampleCount * (mInformation.BitDepth / 8);

			streamData = nullptr;
		}
	}

	if(Is3D())
		flags |= FMOD_3D;
	else
		flags |= FMOD_2D;

	if(mInformation.Format == AudioFormat::PCM || mInformation.ReadMode == AudioReadMode::LoadCompressed)
	{
		switch(mInformation.BitDepth)
		{
		case 8:
			exInfo.format = FMOD_SOUND_FORMAT_PCM8;
			break;
		case 16:
			exInfo.format = FMOD_SOUND_FORMAT_PCM16;
			break;
		case 24:
			exInfo.format = FMOD_SOUND_FORMAT_PCM24;
			break;
		case 32:
			exInfo.format = FMOD_SOUND_FORMAT_PCM32;
			break;
		default:
			B3D_ASSERT(false);
			break;
		}

		exInfo.numchannels = mInformation.ChannelCount;
		exInfo.defaultfrequency = mInformation.Frequency;

		if(mInformation.ReadMode != AudioReadMode::LoadCompressed)
			flags |= FMOD_OPENRAW;
	}

	FMOD::Sound* sound = nullptr;
	FMOD::System* fmod = GetFMODAudio().GetFMOD();
	if(fmod->createSound(streamData, flags, &exInfo, &sound) != FMOD_OK)
	{
		B3D_LOG(Error, LogAudio, "Failed creating a streaming sound.");
		return nullptr;
	}

	sound->setMode(FMOD_LOOP_OFF);
	return sound;
}

void FMODAudioClip::ReleaseStreamingSound(FMOD::Sound* sound)
{
	FMODOggDecompressorData* decompressorData = nullptr;
	((FMOD::Sound*)sound)->getUserData((void**)&decompressorData);

	if(decompressorData != nullptr)
		B3DDelete(decompressorData);

	sound->release();
}

TShared<DataStream> FMODAudioClip::GetSourceStream(u32& outSize)
{
	outSize = mSourceStreamSize;
	mSourceStreamData->Seek(0);

	return mSourceStreamData;
}

bool FMODAudioClip::RequiresStreaming() const
{
	return mInformation.ReadMode == AudioReadMode::Stream ||
		(mInformation.ReadMode == AudioReadMode::LoadCompressed && mInformation.Format == AudioFormat::VORBIS);
}

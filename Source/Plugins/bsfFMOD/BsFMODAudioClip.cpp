//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsFMODAudioClip.h"
#include "BsFMODAudio.h"
#include "FileSystem/BsDataStream.h"

using namespace bs;

FMOD_RESULT F_CALLBACK pcmReadCallback(FMOD_SOUND* sound, void* data, unsigned int dataLen)
{
	FMODOggDecompressorData* decompressor = nullptr;
	((FMOD::Sound*)sound)->getUserData((void**)&decompressor);

	const FMODAudioClip* clip = decompressor->Clip;
	u32 bytesPerSample = (clip->GetBitDepth() / 8);

	assert(dataLen % bytesPerSample == 0);
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

	assert(readSamples == numSamples);

	decompressor->ReadPos += readSamples;
	decompressor->ReadPos %= clip->GetNumSamples();

	return FMOD_OK;
}

FMOD_RESULT F_CALLBACK pcmSetPosCallback(FMOD_SOUND* sound, int subsound, unsigned int position, FMOD_TIMEUNIT posType)
{
	FMODOggDecompressorData* decompressor = nullptr;
	((FMOD::Sound*)sound)->getUserData((void**)&decompressor);

	const FMODAudioClip* clip = decompressor->Clip;
	u32 bytesPerSample = (clip->GetBitDepth() / 8);

	switch(posType)
	{
	case FMOD_TIMEUNIT_MS:
		decompressor->ReadPos = (u32)((clip->GetFrequency() * clip->GetNumChannels()) * (position / 1000.0f));
		break;
	case FMOD_TIMEUNIT_PCM:
		decompressor->ReadPos = clip->GetNumChannels() * position;
		break;
	case FMOD_TIMEUNIT_PCMBYTES:
		assert(position % bytesPerSample == 0);
		decompressor->ReadPos = position / bytesPerSample;
		break;
	default:
		BS_LOG(Error, Audio, "Invalid time unit.");
		break;
	}

	decompressor->ReadPos %= clip->GetNumSamples();
	decompressor->VorbisReader.Seek(decompressor->ReadPos);
	return FMOD_OK;
}

FMODAudioClip::FMODAudioClip(const SPtr<DataStream>& samples, u32 streamSize, u32 numSamples, const AUDIO_CLIP_DESC& desc)
	: AudioClip(samples, streamSize, numSamples, desc)
{}

FMODAudioClip::~FMODAudioClip()
{
	if(mSound != nullptr)
		mSound->release();
}

void FMODAudioClip::Initialize()
{
	AudioDataInfo info;
	info.BitDepth = mDesc.BitDepth;
	info.NumChannels = mDesc.NumChannels;
	info.NumSamples = mNumSamples;
	info.SampleRate = mDesc.Frequency;

	// If we need to keep source data, read everything into memory and keep a copy
	if(mKeepSourceData)
	{
		mStreamData->Seek(mStreamOffset);

		u8* sampleBuffer = (u8*)bs_alloc(mStreamSize);
		mStreamData->Read(sampleBuffer, mStreamSize);

		mSourceStreamData = bs_shared_ptr_new<MemoryDataStream>(sampleBuffer, mStreamSize);
		mSourceStreamSize = mStreamSize;
	}

	// If streaming is not required, create the sound right away
	if(!RequiresStreaming())
	{
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

		FMOD_CREATESOUNDEXINFO exInfo;
		memset(&exInfo, 0, sizeof(exInfo));
		exInfo.cbsize = sizeof(exInfo);
		exInfo.length = bufferSize;

		FMOD_MODE flags = FMOD_CREATESAMPLE | FMOD_OPENMEMORY;

		if(Is3D())
			flags |= FMOD_3D;
		else
			flags |= FMOD_2D;

		if(mDesc.Format == AudioFormat::PCM)
		{
			flags |= FMOD_OPENRAW;

			switch(mDesc.BitDepth)
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
				assert(false);
				break;
			}

			exInfo.numchannels = mDesc.NumChannels;
			exInfo.defaultfrequency = mDesc.Frequency;
		}

		u8* sampleBuffer = (u8*)bs_stack_alloc(bufferSize);
		stream->Seek(offset);
		stream->Read(sampleBuffer, bufferSize);

		FMOD::System* fmod = GetFMODAudio().GetFMODInternal();
		if(fmod->createSound((const char*)sampleBuffer, flags, &exInfo, &mSound) != FMOD_OK)
		{
			BS_LOG(Error, Audio, "Failed creating sound.");
		}
		else
		{
			mSound->setMode(FMOD_LOOP_OFF);
		}

		mStreamData = nullptr;
		mStreamOffset = 0;
		mStreamSize = 0;

		bs_stack_free(sampleBuffer);
	}
	else // Streaming
	{
		// If reading from file, make a copy of data in memory, otherwise just take ownership of the existing buffer
		if(mDesc.ReadMode == AudioReadMode::LoadCompressed && mStreamData->IsFile())
		{
			if(mSourceStreamData != nullptr) // If it's already loaded in memory, use it directly
				mStreamData = mSourceStreamData;
			else
			{
				u8* data = (u8*)bs_alloc(mStreamSize);

				mStreamData->Seek(mStreamOffset);
				mStreamData->Read(data, mStreamSize);

				mStreamData = bs_shared_ptr_new<MemoryDataStream>(data, mStreamSize);
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
		BS_LOG(Error, Audio, "Invalid audio stream data.");
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
		assert(mDesc.ReadMode == AudioReadMode::Stream);

		exInfo.length = mStreamSize;
		exInfo.fileoffset = mStreamOffset;

		SPtr<FileDataStream> fileStream = std::static_pointer_cast<FileDataStream>(mStreamData);
		pathStr = fileStream->GetPath().ToString();

		streamData = pathStr.c_str();
	}
	else
	{
		SPtr<MemoryDataStream> memStream = std::static_pointer_cast<MemoryDataStream>(mStreamData);

		if(mDesc.ReadMode == AudioReadMode::Stream)
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

			exInfo.decodebuffersize = mDesc.Frequency;
			exInfo.pcmreadcallback = pcmReadCallback;
			exInfo.pcmsetposcallback = pcmSetPosCallback;

			AudioDataInfo info;
			info.BitDepth = mDesc.BitDepth;
			info.NumChannels = mDesc.NumChannels;
			info.NumSamples = mNumSamples;
			info.SampleRate = mDesc.Frequency;

			FMODOggDecompressorData* decompressorData = bs_new<FMODOggDecompressorData>();
			decompressorData->Clip = this;

			if(!decompressorData->VorbisReader.Open(memStream, info, mStreamOffset))
			{
				BS_LOG(Error, Audio, "Failed decompressing AudioClip stream.");
				return nullptr;
			}

			exInfo.userdata = decompressorData;
			exInfo.length = mNumSamples * (mDesc.BitDepth / 8);

			streamData = nullptr;
		}
	}

	if(Is3D())
		flags |= FMOD_3D;
	else
		flags |= FMOD_2D;

	if(mDesc.Format == AudioFormat::PCM || mDesc.ReadMode == AudioReadMode::LoadCompressed)
	{
		switch(mDesc.BitDepth)
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
			assert(false);
			break;
		}

		exInfo.numchannels = mDesc.NumChannels;
		exInfo.defaultfrequency = mDesc.Frequency;

		if(mDesc.ReadMode != AudioReadMode::LoadCompressed)
			flags |= FMOD_OPENRAW;
	}

	FMOD::Sound* sound = nullptr;
	FMOD::System* fmod = GetFMODAudio().GetFMODInternal();
	if(fmod->createSound(streamData, flags, &exInfo, &sound) != FMOD_OK)
	{
		BS_LOG(Error, Audio, "Failed creating a streaming sound.");
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
		bs_delete(decompressorData);

	sound->release();
}

SPtr<DataStream> FMODAudioClip::GetSourceStream(u32& size)
{
	size = mSourceStreamSize;
	mSourceStreamData->Seek(0);

	return mSourceStreamData;
}

bool FMODAudioClip::RequiresStreaming() const
{
	return mDesc.ReadMode == AudioReadMode::Stream ||
		(mDesc.ReadMode == AudioReadMode::LoadCompressed && mDesc.Format == AudioFormat::VORBIS);
}

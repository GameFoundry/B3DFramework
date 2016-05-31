//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsFMODAudioClip.h"
#include "BsFMODAudio.h"
#include "BsDataStream.h"

namespace BansheeEngine
{
	FMODAudioClip::FMODAudioClip(const SPtr<DataStream>& samples, UINT32 streamSize, UINT32 numSamples, const AUDIO_CLIP_DESC& desc)
		:AudioClip(samples, streamSize, numSamples, desc), mSound(nullptr), mSourceStreamSize(0)
	{ }

	FMODAudioClip::~FMODAudioClip()
	{
		if(mSound != nullptr)
			mSound->release();
	}

	void FMODAudioClip::initialize()
	{
		AudioDataInfo info;
		info.bitDepth = mDesc.bitDepth;
		info.numChannels = mDesc.numChannels;
		info.numSamples = mNumSamples;
		info.sampleRate = mDesc.frequency;

		// If we need to keep source data, read everything into memory and keep a copy
		if (mKeepSourceData)
		{
			mStreamData->seek(mStreamOffset);

			UINT8* sampleBuffer = (UINT8*)bs_alloc(mStreamSize);
			mStreamData->read(sampleBuffer, mStreamSize);

			mSourceStreamData = bs_shared_ptr_new<MemoryDataStream>(sampleBuffer, mStreamSize);
			mSourceStreamSize = mStreamSize;
		}

		FMOD::System* fmod = gFMODAudio()._getFMOD();
		FMOD_MODE flags = FMOD_OPENMEMORY;

		if (is3D())
			flags |= FMOD_3D;
		else
			flags |= FMOD_2D;

		// Load data into a sound buffer
		if(mDesc.readMode == AudioReadMode::LoadDecompressed || mDesc.readMode == AudioReadMode::LoadCompressed)
		{
			// Read all data into memory
			SPtr<DataStream> stream;
			UINT32 offset = 0;
			if (mSourceStreamData != nullptr) // If it's already loaded in memory, use it directly
				stream = mSourceStreamData;
			else
			{
				stream = mStreamData;
				offset = mStreamOffset;
			}

			UINT32 bufferSize = info.numSamples * (info.bitDepth / 8);
			UINT8* sampleBuffer = (UINT8*)bs_stack_alloc(bufferSize);

			stream->seek(offset);
			stream->read(sampleBuffer, bufferSize);

			FMOD_CREATESOUNDEXINFO exInfo;
			memset(&exInfo, 0, sizeof(exInfo));
			exInfo.cbsize = sizeof(exInfo);
			exInfo.length = bufferSize;

			bool loadCompressed = mDesc.readMode == AudioReadMode::LoadCompressed && mDesc.format != AudioFormat::PCM;

			if (loadCompressed)
				flags |= FMOD_CREATECOMPRESSEDSAMPLE;
			else
				flags |= FMOD_CREATESAMPLE;

			if(mDesc.format == AudioFormat::PCM)
			{
				switch(mDesc.bitDepth)
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

				exInfo.numchannels = mDesc.numChannels;
				exInfo.defaultfrequency = mDesc.frequency;

				flags |= FMOD_OPENRAW;
			}

			if(fmod->createSound((const char*)sampleBuffer, flags, &exInfo, &mSound) != FMOD_OK)
			{
				LOGERR("Failed creating sound.");
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
			// Do nothing, we rely on AudioSource from creating sounds as only one streaming sound can ever be playing
		}

		AudioClip::initialize();
	}

	FMOD::Sound* FMODAudioClip::createStreamingSound() const
	{
		if(getReadMode() != AudioReadMode::Stream || mStreamData == nullptr)
		{
			LOGERR("Invalid audio stream data.");
			return nullptr;
		}

		FMOD_MODE flags = FMOD_CREATESTREAM;
		const char* streamData;

		FMOD_CREATESOUNDEXINFO exInfo;
		memset(&exInfo, 0, sizeof(exInfo));
		exInfo.cbsize = sizeof(exInfo);

		String pathStr;
		if (mStreamData->isFile())
		{
			exInfo.length = mStreamSize;
			exInfo.fileoffset = mStreamOffset;

			SPtr<FileDataStream> fileStream = std::static_pointer_cast<FileDataStream>(mStreamData);
			pathStr = fileStream->getPath().toString();

			streamData = pathStr.c_str();
		}
		else
		{
			SPtr<MemoryDataStream> memStream = std::static_pointer_cast<MemoryDataStream>(mStreamData);

			// Note: I could use FMOD_OPENMEMORY_POINT here to save on memory, but then the caller would need to make
			// sure the memory is not deallocated. I'm ignoring this for now as streaming from memory should be a rare
			// occurence (normally only in editor)
			flags |= FMOD_OPENMEMORY;
			exInfo.length = mStreamSize;

			memStream->seek(mStreamOffset);
			streamData = (const char*)memStream->getCurrentPtr();
		}

		if (is3D())
			flags |= FMOD_3D;
		else
			flags |= FMOD_2D;

		if (mDesc.format == AudioFormat::PCM)
		{
			switch (mDesc.bitDepth)
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

			exInfo.numchannels = mDesc.numChannels;
			exInfo.defaultfrequency = mDesc.frequency;

			flags |= FMOD_OPENRAW;
		}

		FMOD::Sound* sound = nullptr;
		FMOD::System* fmod = gFMODAudio()._getFMOD();
		if (fmod->createSound(streamData, flags, &exInfo, &sound) != FMOD_OK)
		{
			LOGERR("Failed creating a streaming sound.");
			return nullptr;
		}

		sound->setMode(FMOD_LOOP_OFF);
		return sound;
	}

	SPtr<DataStream> FMODAudioClip::getSourceStream(UINT32& size)
	{
		size = mSourceStreamSize;
		mSourceStreamData->seek(0);

		return mSourceStreamData;
	}
}
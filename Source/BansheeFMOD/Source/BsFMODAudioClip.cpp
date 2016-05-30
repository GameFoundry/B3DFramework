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
		AudioFileInfo info;
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
		// TODO - Vorbis cannot be decompressed from memory by FMOD. Instead we force AudioReadMode::Stream for it.
		if(mDesc.readMode == AudioReadMode::LoadDecompressed || 
			(mDesc.readMode == AudioReadMode::LoadCompressed && mDesc.format != AudioFormat::VORBIS))
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

			if(fmod->createSound((const char*)sampleBuffer, flags, &exInfo, &mSound) != FMOD_OK)
			{
				LOGERR("Failed playing sound.");
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

		FMOD_CREATESOUNDEXINFO exInfo;
		memset(&exInfo, 0, sizeof(exInfo));
		exInfo.cbsize = sizeof(exInfo);
		
		// Streaming from memory not supported.
		assert(mStreamData->isFile());

		exInfo.length = mStreamSize;
		exInfo.fileoffset = mStreamOffset;

		SPtr<FileDataStream> fileStream = std::static_pointer_cast<FileDataStream>(mStreamData);
		String pathStr = fileStream->getPath().toString();

		if (is3D())
			flags |= FMOD_3D;
		else
			flags |= FMOD_2D;

		FMOD::Sound* sound = nullptr;
		FMOD::System* fmod = gFMODAudio()._getFMOD();
		if (fmod->createSound(pathStr.c_str(), flags, &exInfo, &sound) != FMOD_OK)
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
//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsNullAudio.h"
#include "Threading/BsTaskScheduler.h"
#include "FileSystem/BsDataStream.h"

using namespace bs;

NullAudio::NullAudio()
{
	mDefaultDevice.Name = "NullDevice";
	mActiveDevice = mDefaultDevice;
	mAllDevices.push_back(mActiveDevice);
}

SPtr<AudioClip> NullAudio::CreateClip(const SPtr<DataStream>& samples, u32 streamSize, u32 numSamples, const AUDIO_CLIP_DESC& desc)
{
	return bs_core_ptr_new<NullAudioClip>(samples, streamSize, numSamples, desc);
}

SPtr<AudioListener> NullAudio::CreateListener()
{
	return bs_shared_ptr_new<NullAudioListener>();
}

SPtr<AudioSource> NullAudio::CreateSource()
{
	return bs_shared_ptr_new<NullAudioSource>();
}

NullAudioClip::NullAudioClip(const SPtr<DataStream>& samples, u32 streamSize, u32 numSamples, const AUDIO_CLIP_DESC& desc)
	: AudioClip(samples, streamSize, numSamples, desc)
{}

void NullAudioClip::Initialize()
{
	// If we need to keep source data, read everything into memory and keep a copy
	if(mKeepSourceData)
	{
		mStreamData->Seek(mStreamOffset);

		u8* sampleBuffer = (u8*)bs_alloc(mStreamSize);
		mStreamData->Read(sampleBuffer, mStreamSize);

		mSourceStreamData = bs_shared_ptr_new<MemoryDataStream>(sampleBuffer, mStreamSize);
		mSourceStreamSize = mStreamSize;
	}

	AudioClip::Initialize();
}

SPtr<DataStream> NullAudioClip::GetSourceStream(u32& size)
{
	size = mSourceStreamSize;
	mSourceStreamData->Seek(0);

	return mSourceStreamData;
}

NullAudio& GetNullAudio()
{
	return static_cast<NullAudio&>(NullAudio::Instance());
}

//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsNullAudio.h"
#include "Threading/BsTaskScheduler.h"
#include "FileSystem/BsDataStream.h"

using namespace b3d;

NullAudio::NullAudio()
{
	mDefaultDevice.Name = "NullDevice";
	mActiveDevice = mDefaultDevice;
	mAllDevices.push_back(mActiveDevice);
}

SPtr<AudioClip> NullAudio::CreateClip(const SPtr<DataStream>& samples, u32 streamSize, u32 numSamples, const AUDIO_CLIP_DESC& desc)
{
	return B3DMakeCoreShared<NullAudioClip>(samples, streamSize, numSamples, desc);
}

SPtr<AudioListener> NullAudio::CreateListener()
{
	return B3DMakeShared<NullAudioListener>();
}

SPtr<AudioSource> NullAudio::CreateSource()
{
	return B3DMakeShared<NullAudioSource>();
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

		u8* sampleBuffer = (u8*)B3DAllocate(mStreamSize);
		mStreamData->Read(sampleBuffer, mStreamSize);

		mSourceStreamData = B3DMakeShared<MemoryDataStream>(sampleBuffer, mStreamSize);
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

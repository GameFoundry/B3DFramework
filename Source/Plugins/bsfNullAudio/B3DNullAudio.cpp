//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DNullAudio.h"
#include "FileSystem/B3DDataStream.h"

using namespace b3d;

NullAudio::NullAudio()
{
	mDefaultDevice.Name = "NullDevice";
	mActiveDevice = mDefaultDevice;
	mAllDevices.push_back(mActiveDevice);
}

TShared<AudioClip> NullAudio::CreateClip(const TShared<DataStream>& samples, u32 streamSize, u32 sampleCount, const AudioClipCreateInformation& createInformation)
{
	return B3DMakeShared<NullAudioClip>(samples, streamSize, sampleCount, createInformation);
}

TShared<IAudioListenerImplementation> NullAudio::CreateListener()
{
	return B3DMakeShared<NullAudioListener>();
}

TShared<IAudioSourceImplementation> NullAudio::CreateSource()
{
	return B3DMakeShared<NullAudioSource>();
}

NullAudioClip::NullAudioClip(const TShared<DataStream>& samples, u32 streamSize, u32 sampleCount, const AudioClipCreateInformation& createInformation)
	: AudioClip(samples, streamSize, sampleCount, createInformation)
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

TShared<DataStream> NullAudioClip::GetSourceStream(u32& outSize)
{
	outSize = mSourceStreamSize;
	mSourceStreamData->Seek(0);

	return mSourceStreamData;
}

NullAudio& GetNullAudio()
{
	return static_cast<NullAudio&>(NullAudio::Instance());
}

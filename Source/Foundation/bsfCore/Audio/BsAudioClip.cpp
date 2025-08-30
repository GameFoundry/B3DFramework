//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Audio/BsAudioClip.h"
#include "Resources/BsResources.h"
#include "Audio/BsAudio.h"
#include "Private/RTTI/BsAudioClipRTTI.h"

using namespace b3d;

AudioClip::AudioClip(const SPtr<DataStream>& samples, u32 streamSize, u32 numSamples, const AudioClipCreateInformation& desc)
	: Resource(false), mInformation(desc), mSampleCount(numSamples), mStreamSize(streamSize), mStreamData(samples)
{
	if(samples != nullptr)
		mStreamOffset = (u32)samples->Tell();

	mKeepSourceData = desc.KeepSourceData;
}

void AudioClip::Initialize()
{
	mLength = mSampleCount / mInformation.ChannelCount / (float)mInformation.Frequency;

	Resource::Initialize();
}

HAudioClip AudioClip::Create(const SPtr<DataStream>& samples, u32 streamSize, u32 sampleCount, const AudioClipCreateInformation& createInformation)
{
	return B3DStaticResourceCast<AudioClip>(GetResources().CreateResourceHandle(CreatePtrInternal(samples, streamSize, sampleCount, createInformation)));
}

SPtr<AudioClip> AudioClip::CreatePtrInternal(const SPtr<DataStream>& samples, u32 streamSize, u32 numSamples, const AudioClipCreateInformation& desc)
{
	SPtr<AudioClip> newClip = GetAudio().CreateClip(samples, streamSize, numSamples, desc);
	newClip->SetShared(newClip);
	newClip->Initialize();

	return newClip;
}

SPtr<AudioClip> AudioClip::CreateEmpty()
{
	AudioClipCreateInformation desc;

	SPtr<AudioClip> newClip = GetAudio().CreateClip(nullptr, 0, 0, desc);
	newClip->SetShared(newClip);

	return newClip;
}

RTTIType* AudioClip::GetRttiStatic()
{
	return AudioClipRTTI::Instance();
}

RTTIType* AudioClip::GetRtti() const
{
	return GetRttiStatic();
}

//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Audio/B3DAudioClip.h"
#include "Resources/B3DResources.h"
#include "Audio/B3DAudio.h"
#include "RTTI/B3DAudioClipRTTI.h"

using namespace b3d;

AudioClip::AudioClip(const TShared<DataStream>& samples, u32 streamSize, u32 sampleCount, const AudioClipCreateInformation& createInformation)
	: Resource(false), mInformation(createInformation), mSampleCount(sampleCount), mStreamSize(streamSize), mStreamData(samples)
{
	if(samples != nullptr)
		mStreamOffset = (u32)samples->Tell();

	mKeepSourceData = createInformation.KeepSourceData;
}

void AudioClip::Initialize()
{
	mLength = mSampleCount / mInformation.ChannelCount / (float)mInformation.Frequency;

	Resource::Initialize();
}

HAudioClip AudioClip::Create(const TShared<DataStream>& samples, u32 streamSize, u32 sampleCount, const AudioClipCreateInformation& createInformation)
{
	return B3DStaticResourceCast<AudioClip>(GetResources().CreateResourceHandle(CreateShared(samples, streamSize, sampleCount, createInformation)));
}

TShared<AudioClip> AudioClip::CreateShared(const TShared<DataStream>& samples, u32 streamSize, u32 sampleCount, const AudioClipCreateInformation& createInformation)
{
	TShared<AudioClip> newClip = GetAudio().CreateClip(samples, streamSize, sampleCount, createInformation);
	newClip->SetShared(newClip);
	newClip->Initialize();

	return newClip;
}

TShared<AudioClip> AudioClip::CreateEmpty()
{
	AudioClipCreateInformation createInformation;

	TShared<AudioClip> newClip = GetAudio().CreateClip(nullptr, 0, 0, createInformation);
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

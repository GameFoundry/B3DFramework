//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Audio/BsAudioClip.h"
#include "Resources/BsResources.h"
#include "Audio/BsAudio.h"
#include "Private/RTTI/BsAudioClipRTTI.h"

using namespace bs;

AudioClip::AudioClip(const SPtr<DataStream>& samples, u32 streamSize, u32 numSamples, const AUDIO_CLIP_DESC& desc)
	: Resource(false), mDesc(desc), mNumSamples(numSamples), mStreamSize(streamSize), mStreamData(samples)
{
	if(samples != nullptr)
		mStreamOffset = (u32)samples->Tell();

	mKeepSourceData = desc.KeepSourceData;
}

void AudioClip::Initialize()
{
	mLength = mNumSamples / mDesc.NumChannels / (float)mDesc.Frequency;

	Resource::Initialize();
}

HAudioClip AudioClip::Create(const SPtr<DataStream>& samples, u32 streamSize, u32 numSamples, const AUDIO_CLIP_DESC& desc)
{
	return static_resource_cast<AudioClip>(gResources().CreateResourceHandleInternal(CreatePtrInternal(samples, streamSize, numSamples, desc)));
}

SPtr<AudioClip> AudioClip::CreatePtrInternal(const SPtr<DataStream>& samples, u32 streamSize, u32 numSamples, const AUDIO_CLIP_DESC& desc)
{
	SPtr<AudioClip> newClip = gAudio().CreateClip(samples, streamSize, numSamples, desc);
	newClip->SetThisPtrInternal(newClip);
	newClip->Initialize();

	return newClip;
}

SPtr<AudioClip> AudioClip::CreateEmpty()
{
	AUDIO_CLIP_DESC desc;

	SPtr<AudioClip> newClip = gAudio().CreateClip(nullptr, 0, 0, desc);
	newClip->SetThisPtrInternal(newClip);

	return newClip;
}

RTTITypeBase* AudioClip::GetRttiStatic()
{
	return AudioClipRTTI::Instance();
}

RTTITypeBase* AudioClip::GetRtti() const
{
	return GetRttiStatic();
}

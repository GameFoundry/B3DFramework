//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Audio/BsAudioSource.h"
#include "Audio/BsAudio.h"
#include "Audio/BsAudioClip.h"
#include "Math/BsMath.h"
#include "Private/RTTI/BsAudioSourceRTTI.h"

using namespace bs;

void AudioSource::SetClip(const HAudioClip& clip)
{
	mAudioClip = clip;

	MarkListenerResourcesDirty();
}

void AudioSource::SetVelocity(const Vector3& velocity)
{
	mVelocity = velocity;
}

void AudioSource::SetVolume(float volume)
{
	mVolume = Math::Clamp01(volume);
}

void AudioSource::SetPitch(float pitch)
{
	mPitch = pitch;
}

void AudioSource::SetIsLooping(bool loop)
{
	mLoop = loop;
}

void AudioSource::SetPriority(i32 priority)
{
	mPriority = priority;
}

void AudioSource::SetMinDistance(float distance)
{
	mMinDistance = distance;
}

void AudioSource::SetAttenuation(float attenuation)
{
	mAttenuation = attenuation;
}

SPtr<AudioSource> AudioSource::Create()
{
	return GetAudio().CreateSource();
}

void AudioSource::GetListenerResources(Vector<HResource>& resources)
{
	if(mAudioClip != nullptr)
		resources.push_back(mAudioClip);
}

void AudioSource::NotifyResourceChanged(const HResource& resource)
{
	OnClipChanged();
}

RTTIType* AudioSource::GetRttiStatic()
{
	return AudioSourceRTTI::Instance();
}

RTTIType* AudioSource::GetRtti() const
{
	return AudioSource::GetRttiStatic();
}

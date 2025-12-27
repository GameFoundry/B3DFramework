//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/B3DAudioSource.h"

#include "Audio/B3DAudio.h"
#include "Scene/B3DSceneObject.h"
#include "Utility/B3DTime.h"
#include "RTTI/B3DAudioSourceRTTI.h"

using namespace b3d;

AudioSource::AudioSource(const HSceneObject& parent)
	: Component(parent)
{ }

AudioSource::AudioSource()
	: AudioSource(nullptr)
{
	SetName("AudioSource");

	mNotifyFlags = TCF_Transform;
}

void AudioSource::SetClip(const HAudioClip& clip)
{
	if(mAudioClip == clip)
		return;

	mAudioClip = clip;
	MarkListenerResourcesDirty();

	if(mImplementation != nullptr)
		mImplementation->SetClip(clip);
}

void AudioSource::SetVolume(float volume)
{
	volume = Math::Clamp01(volume);

	if(mVolume == volume)
		return;

	mVolume = volume;

	if(mImplementation != nullptr)
		mImplementation->SetVolume(volume);
}

void AudioSource::SetPitch(float pitch)
{
	if(mPitch == pitch)
		return;

	mPitch = pitch;

	if(mImplementation != nullptr)
		mImplementation->SetPitch(pitch);
}

void AudioSource::SetIsLooping(bool loop)
{
	if(mLoop == loop)
		return;

	mLoop = loop;

	if(mImplementation != nullptr)
		mImplementation->SetIsLooping(loop);
}

void AudioSource::SetPriority(u32 priority)
{
	if(mPriority == priority)
		return;

	mPriority = priority;

	if(mImplementation != nullptr)
		mImplementation->SetPriority(priority);
}

void AudioSource::SetMinDistance(float distance)
{
	if(mMinDistance == distance)
		return;

	mMinDistance = distance;

	if(mImplementation != nullptr)
		mImplementation->SetMinDistance(distance);
}

void AudioSource::SetAttenuation(float attenuation)
{
	if(mAttenuation == attenuation)
		return;

	mAttenuation = attenuation;

	if(mImplementation != nullptr)
		mImplementation->SetAttenuation(attenuation);
}

void AudioSource::Play()
{
	if(mImplementation != nullptr)
		mImplementation->Play();
}

void AudioSource::Pause()
{
	if(mImplementation != nullptr)
		mImplementation->Pause();
}

void AudioSource::Stop()
{
	if(mImplementation != nullptr)
		mImplementation->Stop();
}

void AudioSource::SetTime(float time)
{
	if(mImplementation != nullptr)
		mImplementation->SetTime(time);
}

float AudioSource::GetTime() const
{
	if(mImplementation != nullptr)
		return mImplementation->GetTime();

	return 0.0f;
}

AudioSourceState AudioSource::GetState() const
{
	if(mImplementation != nullptr)
		return mImplementation->GetState();

	return AudioSourceState::Stopped;
}

void AudioSource::OnDestroyed()
{
	DestroyInternal();
}

void AudioSource::OnDisabled()
{
	DestroyInternal();
}

void AudioSource::OnEnabled()
{
	RestoreInternal();

	if(mPlayOnStart)
		Play();
}

void AudioSource::OnTransformChanged(TransformChangedFlags flags)
{
	if(!GetEnabled())
		return;

	if((flags & (TCF_Parent | TCF_Transform)) != 0)
		UpdateTransform();
}

void AudioSource::Update()
{
	const Vector3 worldPos = SO()->GetTransform().GetPosition();

	const float frameDelta = ::GetTime().GetFrameDelta();
	if(frameDelta > 0.0f)
		mVelocity = (worldPos - mLastPosition) / frameDelta;
	else
		mVelocity = Vector3::kZero;

	mLastPosition = worldPos;
}

void AudioSource::RestoreInternal()
{
	if(mImplementation == nullptr)
		mImplementation = Audio::Instance().CreateSource();

	// Note: Merge into one call to avoid many virtual function calls
	mImplementation->SetClip(mAudioClip);
	mImplementation->SetVolume(mVolume);
	mImplementation->SetPitch(mPitch);
	mImplementation->SetIsLooping(mLoop);
	mImplementation->SetPriority(mPriority);
	mImplementation->SetMinDistance(mMinDistance);
	mImplementation->SetAttenuation(mAttenuation);

	UpdateTransform();
}

void AudioSource::DestroyInternal()
{
	// This should release the last reference and destroy the internal listener
	mImplementation = nullptr;
}

void AudioSource::UpdateTransform()
{
	mImplementation->SetTransform(SO()->GetTransform());
	mImplementation->SetVelocity(mVelocity);
}

void AudioSource::GetListenerResources(Vector<HResource>& resources)
{
	if(mAudioClip != nullptr)
		resources.push_back(mAudioClip);
}

void AudioSource::NotifyResourceChanged(const HResource& resource)
{
	AudioSourceState state = GetState();
	float savedTime = GetTime();

	SetClip(mAudioClip);

	SetTime(savedTime);

	if(state != AudioSourceState::Stopped)
		Play();

	if(state == AudioSourceState::Paused)
		Pause();
}

RTTIType* AudioSource::GetRttiStatic()
{
	return AudioSourceRTTI::Instance();
}

RTTIType* AudioSource::GetRtti() const
{
	return AudioSource::GetRttiStatic();
}

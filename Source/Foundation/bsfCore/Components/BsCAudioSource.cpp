//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsCAudioSource.h"

#include "Audio/BsAudio.h"
#include "Scene/BsSceneObject.h"
#include "Utility/BsTime.h"
#include "Private/RTTI/BsCAudioSourceRTTI.h"

using namespace b3d;

CAudioSource::CAudioSource(const HSceneObject& parent)
	: Component(parent)
{ }

CAudioSource::CAudioSource()
	: CAudioSource(nullptr)
{
	SetName("AudioSource");

	mNotifyFlags = TCF_Transform;
}

void CAudioSource::SetClip(const HAudioClip& clip)
{
	if(mAudioClip == clip)
		return;

	mAudioClip = clip;
	MarkListenerResourcesDirty();

	if(mImplementation != nullptr)
		mImplementation->SetClip(clip);
}

void CAudioSource::SetVolume(float volume)
{
	volume = Math::Clamp01(volume);

	if(mVolume == volume)
		return;

	mVolume = volume;

	if(mImplementation != nullptr)
		mImplementation->SetVolume(volume);
}

void CAudioSource::SetPitch(float pitch)
{
	if(mPitch == pitch)
		return;

	mPitch = pitch;

	if(mImplementation != nullptr)
		mImplementation->SetPitch(pitch);
}

void CAudioSource::SetIsLooping(bool loop)
{
	if(mLoop == loop)
		return;

	mLoop = loop;

	if(mImplementation != nullptr)
		mImplementation->SetIsLooping(loop);
}

void CAudioSource::SetPriority(u32 priority)
{
	if(mPriority == priority)
		return;

	mPriority = priority;

	if(mImplementation != nullptr)
		mImplementation->SetPriority(priority);
}

void CAudioSource::SetMinDistance(float distance)
{
	if(mMinDistance == distance)
		return;

	mMinDistance = distance;

	if(mImplementation != nullptr)
		mImplementation->SetMinDistance(distance);
}

void CAudioSource::SetAttenuation(float attenuation)
{
	if(mAttenuation == attenuation)
		return;

	mAttenuation = attenuation;

	if(mImplementation != nullptr)
		mImplementation->SetAttenuation(attenuation);
}

void CAudioSource::Play()
{
	if(mImplementation != nullptr)
		mImplementation->Play();
}

void CAudioSource::Pause()
{
	if(mImplementation != nullptr)
		mImplementation->Pause();
}

void CAudioSource::Stop()
{
	if(mImplementation != nullptr)
		mImplementation->Stop();
}

void CAudioSource::SetTime(float position)
{
	if(mImplementation != nullptr)
		mImplementation->SetTime(position);
}

float CAudioSource::GetTime() const
{
	if(mImplementation != nullptr)
		return mImplementation->GetTime();

	return 0.0f;
}

AudioSourceState CAudioSource::GetState() const
{
	if(mImplementation != nullptr)
		return mImplementation->GetState();

	return AudioSourceState::Stopped;
}

void CAudioSource::OnDestroyed()
{
	DestroyInternal();
}

void CAudioSource::OnDisabled()
{
	DestroyInternal();
}

void CAudioSource::OnEnabled()
{
	RestoreInternal();

	if(mPlayOnStart)
		Play();
}

void CAudioSource::OnTransformChanged(TransformChangedFlags flags)
{
	if(!GetEnabled())
		return;

	if((flags & (TCF_Parent | TCF_Transform)) != 0)
		UpdateTransform();
}

void CAudioSource::Update()
{
	const Vector3 worldPos = SO()->GetTransform().GetPosition();

	const float frameDelta = ::GetTime().GetFrameDelta();
	if(frameDelta > 0.0f)
		mVelocity = (worldPos - mLastPosition) / frameDelta;
	else
		mVelocity = Vector3::kZero;

	mLastPosition = worldPos;
}

void CAudioSource::RestoreInternal()
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

void CAudioSource::DestroyInternal()
{
	// This should release the last reference and destroy the internal listener
	mImplementation = nullptr;
}

void CAudioSource::UpdateTransform()
{
	mImplementation->SetTransform(SO()->GetTransform());
	mImplementation->SetVelocity(mVelocity);
}

void CAudioSource::GetListenerResources(Vector<HResource>& resources)
{
	if(mAudioClip != nullptr)
		resources.push_back(mAudioClip);
}

void CAudioSource::NotifyResourceChanged(const HResource& resource)
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

RTTIType* CAudioSource::GetRttiStatic()
{
	return CAudioSourceRTTI::Instance();
}

RTTIType* CAudioSource::GetRtti() const
{
	return CAudioSource::GetRttiStatic();
}

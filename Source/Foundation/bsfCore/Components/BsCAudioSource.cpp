//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Components/BsCAudioSource.h"
#include "Scene/BsSceneObject.h"
#include "Utility/BsTime.h"
#include "Private/RTTI/BsCAudioSourceRTTI.h"

using namespace std::placeholders;

namespace bs
{
	CAudioSource::CAudioSource()
	{
		setName("AudioSource");

		mNotifyFlags = TCF_Transform;
	}

	CAudioSource::CAudioSource(const HSceneObject& parent)
		: Component(parent)
	{
		setName("AudioSource");

		mNotifyFlags = TCF_Transform;
	}

	void CAudioSource::SetClip(const HAudioClip& clip)
	{
		if (mAudioClip == clip)
			return;

		mAudioClip = clip;

		if (mInternal != nullptr)
			mInternal->SetClip(clip);
	}

	void CAudioSource::SetVolume(float volume)
	{
		if (mVolume == volume)
			return;

		mVolume = volume;

		if (mInternal != nullptr)
			mInternal->SetVolume(volume);
	}

	void CAudioSource::SetPitch(float pitch)
	{
		if (mPitch == pitch)
			return;

		mPitch = pitch;

		if (mInternal != nullptr)
			mInternal->SetPitch(pitch);
	}

	void CAudioSource::SetIsLooping(bool loop)
	{
		if (mLoop == loop)
			return;

		mLoop = loop;

		if (mInternal != nullptr)
			mInternal->SetIsLooping(loop);
	}

	void CAudioSource::SetPriority(UINT32 priority)
	{
		if (mPriority == priority)
			return;

		mPriority = priority;

		if (mInternal != nullptr)
			mInternal->SetPriority(priority);
	}

	void CAudioSource::SetMinDistance(float distance)
	{
		if (mMinDistance == distance)
			return;

		mMinDistance = distance;

		if (mInternal != nullptr)
			mInternal->SetMinDistance(distance);
	}

	void CAudioSource::SetAttenuation(float attenuation)
	{
		if (mAttenuation == attenuation)
			return;

		mAttenuation = attenuation;

		if (mInternal != nullptr)
			mInternal->SetAttenuation(attenuation);
	}

	void CAudioSource::Play()
	{
		if (mInternal != nullptr)
			mInternal->Play();
	}

	void CAudioSource::Pause()
	{
		if (mInternal != nullptr)
			mInternal->Pause();
	}

	void CAudioSource::Stop()
	{
		if (mInternal != nullptr)
			mInternal->Stop();
	}

	void CAudioSource::SetTime(float position)
	{
		if (mInternal != nullptr)
			mInternal->SetTime(position);
	}

	float CAudioSource::GetTime() const
	{
		if (mInternal != nullptr)
			return mInternal->GetTime();

		return 0.0f;
	}

	AudioSourceState CAudioSource::GetState() const
	{
		if (mInternal != nullptr)
			return mInternal->GetState();

		return AudioSourceState::Stopped;
	}

	void CAudioSource::OnInitialized()
	{

	}

	void CAudioSource::OnDestroyed()
	{
		destroyInternal();
	}

	void CAudioSource::OnDisabled()
	{
		destroyInternal();
	}

	void CAudioSource::OnEnabled()
	{
		restoreInternal();

		if (mPlayOnStart)
			play();
	}

	void CAudioSource::OnTransformChanged(TransformChangedFlags flags)
	{
		if (!SO()->GetActive())
			return;

		if ((flags & (TCF_Parent | TCF_Transform)) != 0)
			updateTransform();
	}

	void CAudioSource::Update()
	{
		const Vector3 worldPos = SO()->GetTransform().GetPosition();

		const float frameDelta = gTime().GetFrameDelta();
		if(frameDelta > 0.0f)
			mVelocity = (worldPos - mLastPosition) / frameDelta;
		else
			mVelocity = Vector3::ZERO;

		mLastPosition = worldPos;
	}

	void CAudioSource::RestoreInternal()
	{
		if (mInternal == nullptr)
			mInternal = AudioSource::create();

		// Note: Merge into one call to avoid many virtual function calls
		mInternal->SetClip(mAudioClip);
		mInternal->SetVolume(mVolume);
		mInternal->SetPitch(mPitch);
		mInternal->SetIsLooping(mLoop);
		mInternal->SetPriority(mPriority);
		mInternal->SetMinDistance(mMinDistance);
		mInternal->SetAttenuation(mAttenuation);

		updateTransform();
	}

	void CAudioSource::DestroyInternal()
	{
		// This should release the last reference and destroy the internal listener
		mInternal = nullptr;
	}

	void CAudioSource::UpdateTransform()
	{
		mInternal->SetTransform(SO()->getTransform());
		mInternal->SetVelocity(mVelocity);
	}

	RTTITypeBase* CAudioSource::getRTTIStatic()
	{
		return CAudioSourceRTTI::Instance();
	}

	RTTITypeBase* CAudioSource::getRTTI() const
	{
		return CAudioSource::GetRTTIStatic();
	}
}

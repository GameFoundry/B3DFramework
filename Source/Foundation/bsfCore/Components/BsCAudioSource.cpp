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
			mInternal->setClip(clip);
	}

	void CAudioSource::SetVolume(float volume)
	{
		if (mVolume == volume)
			return;

		mVolume = volume;

		if (mInternal != nullptr)
			mInternal->setVolume(volume);
	}

	void CAudioSource::SetPitch(float pitch)
	{
		if (mPitch == pitch)
			return;

		mPitch = pitch;

		if (mInternal != nullptr)
			mInternal->setPitch(pitch);
	}

	void CAudioSource::SetIsLooping(bool loop)
	{
		if (mLoop == loop)
			return;

		mLoop = loop;

		if (mInternal != nullptr)
			mInternal->setIsLooping(loop);
	}

	void CAudioSource::SetPriority(UINT32 priority)
	{
		if (mPriority == priority)
			return;

		mPriority = priority;

		if (mInternal != nullptr)
			mInternal->setPriority(priority);
	}

	void CAudioSource::SetMinDistance(float distance)
	{
		if (mMinDistance == distance)
			return;

		mMinDistance = distance;

		if (mInternal != nullptr)
			mInternal->setMinDistance(distance);
	}

	void CAudioSource::SetAttenuation(float attenuation)
	{
		if (mAttenuation == attenuation)
			return;

		mAttenuation = attenuation;

		if (mInternal != nullptr)
			mInternal->setAttenuation(attenuation);
	}

	void CAudioSource::Play()
	{
		if (mInternal != nullptr)
			mInternal->play();
	}

	void CAudioSource::Pause()
	{
		if (mInternal != nullptr)
			mInternal->pause();
	}

	void CAudioSource::Stop()
	{
		if (mInternal != nullptr)
			mInternal->stop();
	}

	void CAudioSource::SetTime(float position)
	{
		if (mInternal != nullptr)
			mInternal->setTime(position);
	}

	float CAudioSource::GetTime() const
	{
		if (mInternal != nullptr)
			return mInternal->getTime();

		return 0.0f;
	}

	AudioSourceState CAudioSource::GetState() const
	{
		if (mInternal != nullptr)
			return mInternal->getState();

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
		if (!SO()->getActive())
			return;

		if ((flags & (TCF_Parent | TCF_Transform)) != 0)
			updateTransform();
	}

	void CAudioSource::Update()
	{
		const Vector3 worldPos = SO()->getTransform().getPosition();

		const float frameDelta = gTime().getFrameDelta();
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
		mInternal->setClip(mAudioClip);
		mInternal->setVolume(mVolume);
		mInternal->setPitch(mPitch);
		mInternal->setIsLooping(mLoop);
		mInternal->setPriority(mPriority);
		mInternal->setMinDistance(mMinDistance);
		mInternal->setAttenuation(mAttenuation);

		updateTransform();
	}

	void CAudioSource::DestroyInternal()
	{
		// This should release the last reference and destroy the internal listener
		mInternal = nullptr;
	}

	void CAudioSource::UpdateTransform()
	{
		mInternal->setTransform(SO()->getTransform());
		mInternal->setVelocity(mVelocity);
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

//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsFMODAudioSource.h"
#include "BsFMODAudio.h"
#include "BsFMODAudioClip.h"

namespace bs
{
	FMODAudioSource::FMODAudioSource()
	{
		gFMODAudio()._registerSource(this);
	}

	FMODAudioSource::~FMODAudioSource()
	{
		gFMODAudio()._unregisterSource(this);

		if (mStreamingSound != nullptr)
			FMODAudioClip::releaseStreamingSound(mStreamingSound);

		if (mChannel != nullptr)
			mChannel->stop();
	}

	void FMODAudioSource::setClip(const HAudioClip& clip)
	{
		stop();

		AudioSource::setClip(clip);
	}

	void FMODAudioSource::setTransform(const Transform& transform)
	{
		AudioSource::setTransform(transform);

		if(mChannel != nullptr)
		{
			Vector3 position = transform.getPosition();

			FMOD_VECTOR fmodPosition = { position.x, position.y, position.z };
			mChannel->set3DAttributes(&fmodPosition, nullptr);
		}
	}

	void FMODAudioSource::setVelocity(const Vector3& velocity)
	{
		AudioSource::setVelocity(velocity);

		if (mChannel != nullptr)
		{
			FMOD_VECTOR fmodVelocity = { velocity.x, velocity.y, velocity.z };
			mChannel->set3DAttributes(nullptr, &fmodVelocity);
		}
	}

	void FMODAudioSource::setVolume(float volume)
	{
		AudioSource::setVolume(volume);

		if (mChannel != nullptr)
			mChannel->setVolume(mVolume);
	}

	void FMODAudioSource::setPitch(float pitch)
	{
		AudioSource::setPitch(pitch);

		if (mChannel != nullptr)
			mChannel->setPitch(mPitch);
	}

	void FMODAudioSource::setIsLooping(bool loop)
	{
		AudioSource::setIsLooping(loop);

		if (mChannel != nullptr)
			mChannel->setMode(loop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF);
	}

	void FMODAudioSource::setPriority(INT32 priority)
	{
		AudioSource::setPriority(priority);

		if (mChannel != nullptr)
			mChannel->setPriority(priority);
	}

	void FMODAudioSource::play()
	{
		mGlobalUnpauseState = AudioSourceState::Playing;

		if (mGloballyPaused)
			return;

		if (!mAudioClip.isLoaded())
			return;

		if (mChannel == nullptr)
		{
			assert(mStreamingSound == nullptr);

			FMOD::System* fmod = gFMODAudio()._getFMOD();
			
			FMODAudioClip* fmodClip = static_cast<FMODAudioClip*>(mAudioClip.get());
			FMOD::Sound* sound;
			if(fmodClip->requiresStreaming())
			{
				mStreamingSound = fmodClip->createStreamingSound();
				sound = mStreamingSound;
			}
			else
			{
				sound = fmodClip->getSound();
			}
			
			if(fmod->playSound(sound, nullptr, true, &mChannel) != FMOD_OK)
			{
				BS_LOG(Error, Audio, "Failed playing sound.");

				if (mStreamingSound != nullptr)
				{
					FMODAudioClip::releaseStreamingSound(mStreamingSound);
					mStreamingSound = nullptr;
				}

				return;
			}

			mChannel->setUserData(this);
			mChannel->setVolume(mVolume);
			mChannel->setPitch(mVolume);
			mChannel->setMode(mLoop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF);
			mChannel->setPriority(mPriority);
			mChannel->setPosition((UINT32)(mTime * 1000.0f), FMOD_TIMEUNIT_MS);

			Vector3 position = getTransform().getPosition();

			FMOD_VECTOR fmodPosition = { position.x, position.y, position.z };
			FMOD_VECTOR fmodVelocity = { mVelocity.x, mVelocity.y, mVelocity.z };
			mChannel->set3DAttributes(&fmodPosition, &fmodVelocity);
		}
			
		mChannel->setPaused(false);
	}

	void FMODAudioSource::pause()
	{
		mGlobalUnpauseState = AudioSourceState::Paused;

		if (mChannel != nullptr)
			mChannel->setPaused(true);
	}

	void FMODAudioSource::stop()
	{
		mGlobalUnpauseState = AudioSourceState::Stopped;

		if (mChannel != nullptr)
		{
			mChannel->stop();
			mChannel = nullptr;
		}

		if(mStreamingSound != nullptr)
		{
			FMODAudioClip::releaseStreamingSound(mStreamingSound);
			mStreamingSound = nullptr;
		}

		mTime = 0.0f;
	}

	void FMODAudioSource::setGlobalPause(bool doPause)
	{
		if (mGloballyPaused == doPause)
			return;

		mGloballyPaused = doPause;
		
		if(doPause)
		{
			AudioSourceState currentState = getState();

			if (getState() == AudioSourceState::Playing)
				pause();

			mGlobalUnpauseState = currentState;
		}
		else
		{
			if (mGlobalUnpauseState == AudioSourceState::Playing)
				play();
		}
	}

	AudioSourceState FMODAudioSource::getState() const
	{
		if(mChannel == nullptr)
			return AudioSourceState::Stopped;

		bool isPlaying = false;
		mChannel->isPlaying(&isPlaying);

		if (isPlaying)
			return AudioSourceState::Playing;

		bool isPaused = false;
		mChannel->getPaused(&isPaused);
		if (isPaused)
			return AudioSourceState::Paused;

		return AudioSourceState::Stopped;
	}

	void FMODAudioSource::setTime(float time)
	{
		if (mChannel != nullptr)
			mChannel->setPosition((UINT32)(time * 1000.0f), FMOD_TIMEUNIT_MS);
		else
			mTime = time;
	}

	float FMODAudioSource::getTime() const
	{
		if(mChannel != nullptr)
		{
			UINT32 position = 0;
			mChannel->getPosition(&position, FMOD_TIMEUNIT_MS);

			return position / 1000.0f;
		}

		return 0.0f;
	}

	void FMODAudioSource::onClipChanged()
	{
		AudioSourceState state = getState();
		float savedTime = getTime();

		stop();

		setTime(savedTime);

		if (state != AudioSourceState::Stopped)
			play();

		if (state == AudioSourceState::Paused)
			pause();
	}
}

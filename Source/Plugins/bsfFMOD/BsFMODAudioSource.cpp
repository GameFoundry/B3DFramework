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
			mChannel->Stop();
	}

	void FMODAudioSource::SetClip(const HAudioClip& clip)
	{
		stop();

		AudioSource::setClip(clip);
	}

	void FMODAudioSource::SetTransform(const Transform& transform)
	{
		AudioSource::setTransform(transform);

		if(mChannel != nullptr)
		{
			Vector3 position = transform.GetPosition();

			FMOD_VECTOR fmodPosition = { position.x, position.y, position.z };
			mChannel->Set3DAttributes(&fmodPosition, nullptr);
		}
	}

	void FMODAudioSource::SetVelocity(const Vector3& velocity)
	{
		AudioSource::setVelocity(velocity);

		if (mChannel != nullptr)
		{
			FMOD_VECTOR fmodVelocity = { velocity.x, velocity.y, velocity.z };
			mChannel->Set3DAttributes(nullptr, &fmodVelocity);
		}
	}

	void FMODAudioSource::SetVolume(float volume)
	{
		AudioSource::setVolume(volume);

		if (mChannel != nullptr)
			mChannel->SetVolume(mVolume);
	}

	void FMODAudioSource::SetPitch(float pitch)
	{
		AudioSource::setPitch(pitch);

		if (mChannel != nullptr)
			mChannel->SetPitch(mPitch);
	}

	void FMODAudioSource::SetIsLooping(bool loop)
	{
		AudioSource::setIsLooping(loop);

		if (mChannel != nullptr)
			mChannel->SetMode(loop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF);
	}

	void FMODAudioSource::SetPriority(INT32 priority)
	{
		AudioSource::setPriority(priority);

		if (mChannel != nullptr)
			mChannel->SetPriority(priority);
	}

	void FMODAudioSource::Play()
	{
		mGlobalUnpauseState = AudioSourceState::Playing;

		if (mGloballyPaused)
			return;

		if (!mAudioClip.IsLoaded())
			return;

		if (mChannel == nullptr)
		{
			assert(mStreamingSound == nullptr);

			FMOD::System* fmod = gFMODAudio()._getFMOD();
			
			FMODAudioClip* fmodClip = static_cast<FMODAudioClip*>(mAudioClip.Get());
			FMOD::Sound* sound;
			if(fmodClip->RequiresStreaming())
			{
				mStreamingSound = fmodClip->CreateStreamingSound();
				sound = mStreamingSound;
			}
			else
			{
				sound = fmodClip->GetSound();
			}
			
			if(fmod->PlaySound(sound, nullptr, true, &mChannel) != FMOD_OK)
			{
				BS_LOG(Error, Audio, "Failed playing sound.");

				if (mStreamingSound != nullptr)
				{
					FMODAudioClip::releaseStreamingSound(mStreamingSound);
					mStreamingSound = nullptr;
				}

				return;
			}

			mChannel->SetUserData(this);
			mChannel->SetVolume(mVolume);
			mChannel->SetPitch(mVolume);
			mChannel->SetMode(mLoop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF);
			mChannel->SetPriority(mPriority);
			mChannel->SetPosition((UINT32)(mTime * 1000.0f), FMOD_TIMEUNIT_MS);

			Vector3 position = getTransform().GetPosition();

			FMOD_VECTOR fmodPosition = { position.x, position.y, position.z };
			FMOD_VECTOR fmodVelocity = { mVelocity.x, mVelocity.y, mVelocity.z };
			mChannel->Set3DAttributes(&fmodPosition, &fmodVelocity);
		}
			
		mChannel->SetPaused(false);
	}

	void FMODAudioSource::Pause()
	{
		mGlobalUnpauseState = AudioSourceState::Paused;

		if (mChannel != nullptr)
			mChannel->SetPaused(true);
	}

	void FMODAudioSource::Stop()
	{
		mGlobalUnpauseState = AudioSourceState::Stopped;

		if (mChannel != nullptr)
		{
			mChannel->Stop();
			mChannel = nullptr;
		}

		if(mStreamingSound != nullptr)
		{
			FMODAudioClip::releaseStreamingSound(mStreamingSound);
			mStreamingSound = nullptr;
		}

		mTime = 0.0f;
	}

	void FMODAudioSource::SetGlobalPause(bool doPause)
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

	AudioSourceState FMODAudioSource::GetState() const
	{
		if(mChannel == nullptr)
			return AudioSourceState::Stopped;

		bool isPlaying = false;
		mChannel->IsPlaying(&isPlaying);

		if (isPlaying)
			return AudioSourceState::Playing;

		bool isPaused = false;
		mChannel->GetPaused(&isPaused);
		if (isPaused)
			return AudioSourceState::Paused;

		return AudioSourceState::Stopped;
	}

	void FMODAudioSource::SetTime(float time)
	{
		if (mChannel != nullptr)
			mChannel->SetPosition((UINT32)(time * 1000.0f), FMOD_TIMEUNIT_MS);
		else
			mTime = time;
	}

	float FMODAudioSource::GetTime() const
	{
		if(mChannel != nullptr)
		{
			UINT32 position = 0;
			mChannel->GetPosition(&position, FMOD_TIMEUNIT_MS);

			return position / 1000.0f;
		}

		return 0.0f;
	}

	void FMODAudioSource::OnClipChanged()
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

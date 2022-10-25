//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsFMODAudioSource.h"
#include "BsFMODAudio.h"
#include "BsFMODAudioClip.h"

namespace bs
{
	FMODAudioSource::FMODAudioSource()
	{
		gFMODAudio().RegisterSourceInternal(this);
	}

	FMODAudioSource::~FMODAudioSource()
	{
		gFMODAudio().UnregisterSourceInternal(this);

		if(mStreamingSound != nullptr)
			FMODAudioClip::ReleaseStreamingSound(mStreamingSound);

		if(mChannel != nullptr)
			mChannel->stop();
	}

	void FMODAudioSource::SetClip(const HAudioClip& clip)
	{
		Stop();

		AudioSource::SetClip(clip);
	}

	void FMODAudioSource::SetTransform(const Transform& transform)
	{
		SceneActor::SetTransform(transform);

		if(mChannel != nullptr)
		{
			Vector3 position = transform.GetPosition();

			FMOD_VECTOR fmodPosition = { position.x, position.y, position.z };
			mChannel->set3DAttributes(&fmodPosition, nullptr);
		}
	}

	void FMODAudioSource::SetVelocity(const Vector3& velocity)
	{
		AudioSource::SetVelocity(velocity);

		if(mChannel != nullptr)
		{
			FMOD_VECTOR fmodVelocity = { velocity.x, velocity.y, velocity.z };
			mChannel->set3DAttributes(nullptr, &fmodVelocity);
		}
	}

	void FMODAudioSource::SetVolume(float volume)
	{
		AudioSource::SetVolume(volume);

		if(mChannel != nullptr)
			mChannel->setVolume(mVolume);
	}

	void FMODAudioSource::SetPitch(float pitch)
	{
		AudioSource::SetPitch(pitch);

		if(mChannel != nullptr)
			mChannel->setPitch(mPitch);
	}

	void FMODAudioSource::SetIsLooping(bool loop)
	{
		AudioSource::SetIsLooping(loop);

		if(mChannel != nullptr)
			mChannel->setMode(loop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF);
	}

	void FMODAudioSource::SetPriority(i32 priority)
	{
		AudioSource::SetPriority(priority);

		if(mChannel != nullptr)
			mChannel->setPriority(priority);
	}

	void FMODAudioSource::Play()
	{
		mGlobalUnpauseState = AudioSourceState::Playing;

		if(mGloballyPaused)
			return;

		if(!mAudioClip.IsLoaded())
			return;

		if(mChannel == nullptr)
		{
			assert(mStreamingSound == nullptr);

			FMOD::System* fmod = gFMODAudio().GetFMODInternal();

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

			if(fmod->playSound(sound, nullptr, true, &mChannel) != FMOD_OK)
			{
				BS_LOG(Error, Audio, "Failed playing sound.");

				if(mStreamingSound != nullptr)
				{
					FMODAudioClip::ReleaseStreamingSound(mStreamingSound);
					mStreamingSound = nullptr;
				}

				return;
			}

			mChannel->setUserData(this);
			mChannel->setVolume(mVolume);
			mChannel->setPitch(mVolume);
			mChannel->setMode(mLoop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF);
			mChannel->setPriority(mPriority);
			mChannel->setPosition((u32)(mTime * 1000.0f), FMOD_TIMEUNIT_MS);

			Vector3 position = GetTransform().GetPosition();

			FMOD_VECTOR fmodPosition = { position.x, position.y, position.z };
			FMOD_VECTOR fmodVelocity = { mVelocity.x, mVelocity.y, mVelocity.z };
			mChannel->set3DAttributes(&fmodPosition, &fmodVelocity);
		}

		mChannel->setPaused(false);
	}

	void FMODAudioSource::Pause()
	{
		mGlobalUnpauseState = AudioSourceState::Paused;

		if(mChannel != nullptr)
			mChannel->setPaused(true);
	}

	void FMODAudioSource::Stop()
	{
		mGlobalUnpauseState = AudioSourceState::Stopped;

		if(mChannel != nullptr)
		{
			mChannel->stop();
			mChannel = nullptr;
		}

		if(mStreamingSound != nullptr)
		{
			FMODAudioClip::ReleaseStreamingSound(mStreamingSound);
			mStreamingSound = nullptr;
		}

		mTime = 0.0f;
	}

	void FMODAudioSource::SetGlobalPause(bool doPause)
	{
		if(mGloballyPaused == doPause)
			return;

		mGloballyPaused = doPause;

		if(doPause)
		{
			AudioSourceState currentState = GetState();

			if(GetState() == AudioSourceState::Playing)
				Pause();

			mGlobalUnpauseState = currentState;
		}
		else
		{
			if(mGlobalUnpauseState == AudioSourceState::Playing)
				Play();
		}
	}

	AudioSourceState FMODAudioSource::GetState() const
	{
		if(mChannel == nullptr)
			return AudioSourceState::Stopped;

		bool isPlaying = false;
		mChannel->isPlaying(&isPlaying);

		if(isPlaying)
			return AudioSourceState::Playing;

		bool isPaused = false;
		mChannel->getPaused(&isPaused);
		if(isPaused)
			return AudioSourceState::Paused;

		return AudioSourceState::Stopped;
	}

	void FMODAudioSource::SetTime(float time)
	{
		if(mChannel != nullptr)
			mChannel->setPosition((u32)(time * 1000.0f), FMOD_TIMEUNIT_MS);
		else
			mTime = time;
	}

	float FMODAudioSource::GetTime() const
	{
		if(mChannel != nullptr)
		{
			u32 position = 0;
			mChannel->getPosition(&position, FMOD_TIMEUNIT_MS);

			return position / 1000.0f;
		}

		return 0.0f;
	}

	void FMODAudioSource::OnClipChanged()
	{
		AudioSourceState state = GetState();
		float savedTime = GetTime();

		Stop();

		SetTime(savedTime);

		if(state != AudioSourceState::Stopped)
			Play();

		if(state == AudioSourceState::Paused)
			Pause();
	}
} // namespace bs

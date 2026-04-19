//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DFMODAudioSource.h"
#include "B3DFMODAudio.h"
#include "B3DFMODAudioClip.h"
#include "Math/B3DTransform.h"

using namespace b3d;

FMODAudioSource::FMODAudioSource()
{
	GetFMODAudio().RegisterSource(this);
}

FMODAudioSource::~FMODAudioSource()
{
	GetFMODAudio().UnregisterSource(this);

	if(mStreamingSound != nullptr)
		FMODAudioClip::ReleaseStreamingSound(mStreamingSound);

	if(mChannel != nullptr)
		mChannel->stop();
}

void FMODAudioSource::SetClip(const HAudioClip& clip)
{
	Stop();

	mAudioClip = B3DStaticResourceCast<FMODAudioClip>(clip);
}

void FMODAudioSource::SetTransform(const Transform& transform)
{
	mPosition = transform.GetPosition();

	if(mChannel != nullptr)
	{
		FMOD_VECTOR fmodPosition = { mPosition.X, mPosition.Y, mPosition.Z };
		mChannel->set3DAttributes(&fmodPosition, nullptr);
	}
}

void FMODAudioSource::SetVelocity(const Vector3& velocity)
{
	mVelocity = velocity;

	if(mChannel != nullptr)
	{
		FMOD_VECTOR fmodVelocity = { velocity.X, velocity.Y, velocity.Z };
		mChannel->set3DAttributes(nullptr, &fmodVelocity);
	}
}

void FMODAudioSource::SetVolume(float volume)
{
	mVolume = volume;

	if(mChannel != nullptr)
		mChannel->setVolume(mVolume);
}

void FMODAudioSource::SetPitch(float pitch)
{
	mPitch = pitch;

	if(mChannel != nullptr)
		mChannel->setPitch(mPitch);
}

void FMODAudioSource::SetIsLooping(bool loop)
{
	mLoop = loop;

	if(mChannel != nullptr)
		mChannel->setMode(loop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF);
}

void FMODAudioSource::SetPriority(i32 priority)
{
	mPriority = priority;

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
		B3D_ASSERT(mStreamingSound == nullptr);

		FMOD::System* fmod = GetFMODAudio().GetFMOD();

		FMOD::Sound* sound;
		if(mAudioClip->RequiresStreaming())
		{
			mStreamingSound = mAudioClip->CreateStreamingSound();
			sound = mStreamingSound;
		}
		else
		{
			sound = mAudioClip->GetSound();
		}

		if(fmod->playSound(sound, nullptr, true, &mChannel) != FMOD_OK)
		{
			B3D_LOG(Error, LogAudio, "Failed playing sound.");

			if(mStreamingSound != nullptr)
			{
				FMODAudioClip::ReleaseStreamingSound(mStreamingSound);
				mStreamingSound = nullptr;
			}

			return;
		}

		mChannel->setUserData(this);
		mChannel->setVolume(mVolume);
		mChannel->setPitch(mPitch);
		mChannel->setMode(mLoop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF);
		mChannel->setPriority(mPriority);
		mChannel->setPosition((u32)(mTime * 1000.0f), FMOD_TIMEUNIT_MS);

		FMOD_VECTOR fmodPosition = { mPosition.X, mPosition.Y, mPosition.Z };
		FMOD_VECTOR fmodVelocity = { mVelocity.X, mVelocity.Y, mVelocity.Z };
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

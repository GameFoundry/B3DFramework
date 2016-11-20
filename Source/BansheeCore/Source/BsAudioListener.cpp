//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsAudioListener.h"
#include "BsAudio.h"

namespace bs
{
	AudioListener::AudioListener()
		:mPosition(BsZero), mDirection(-Vector3::UNIT_Z), mUp(Vector3::UNIT_Y), mVelocity(BsZero)
	{
		
	}

	void AudioListener::setPosition(const Vector3& position)
	{
		mPosition = position;
	}

	void AudioListener::setDirection(const Vector3& direction)
	{
		mDirection = direction;
	}

	void AudioListener::setUp(const Vector3& up)
	{
		mUp = up;
	}

	void AudioListener::setVelocity(const Vector3& velocity)
	{
		mVelocity = velocity;
	}

	SPtr<AudioListener> AudioListener::create()
	{
		return gAudio().createListener();
	}	
}
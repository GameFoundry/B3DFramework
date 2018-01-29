//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Audio/BsAudioListener.h"
#include "Audio/BsAudio.h"
#include "Private/RTTI/BsAudioListenerRTTI.h"

namespace bs
{
	AudioListener::AudioListener()
		:mVelocity(BsZero)
	{
		
	}

	void AudioListener::setVelocity(const Vector3& velocity)
	{
		mVelocity = velocity;
	}

	SPtr<AudioListener> AudioListener::create()
	{
		return gAudio().createListener();
	}	

	RTTITypeBase* AudioListener::getRTTIStatic()
	{
		return AudioListenerRTTI::instance();
	}

	RTTITypeBase* AudioListener::getRTTI() const
	{
		return AudioListener::getRTTIStatic();
	}
}
//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Audio/BsAudioListener.h"
#include "Audio/BsAudio.h"
#include "Private/RTTI/BsAudioListenerRTTI.h"

namespace bs
{
	void AudioListener::SetVelocity(const Vector3& velocity)
	{
		mVelocity = velocity;
	}

	SPtr<AudioListener> AudioListener::Create()
	{
		return GAudio().CreateListener();
	}	

	RTTITypeBase* AudioListener::getRTTIStatic()
	{
		return AudioListenerRTTI::Instance();
	}

	RTTITypeBase* AudioListener::getRTTI() const
	{
		return AudioListener::GetRTTIStatic();
	}
}

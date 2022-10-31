//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Audio/BsAudioListener.h"
#include "Audio/BsAudio.h"
#include "Private/RTTI/BsAudioListenerRTTI.h"

using namespace bs;

void AudioListener::SetVelocity(const Vector3& velocity)
{
	mVelocity = velocity;
}

SPtr<AudioListener> AudioListener::Create()
{
	return GetAudio().CreateListener();
}

RTTITypeBase* AudioListener::GetRttiStatic()
{
	return AudioListenerRTTI::Instance();
}

RTTITypeBase* AudioListener::GetRtti() const
{
	return AudioListener::GetRttiStatic();
}

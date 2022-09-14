//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsFMODAudioListener.h"
#include "BsFMODAudio.h"

namespace bs
{
	FMODAudioListener::FMODAudioListener()
	{
		gFMODAudio().RegisterListenerInternal(this);
	}

	FMODAudioListener::~FMODAudioListener()
	{
		gFMODAudio().UnregisterListenerInternal(this);
	}

	void FMODAudioListener::setTransform(const Transform& transform)
	{
		AudioListener::setTransform(transform);

		Vector3 position = transform.getPosition();
		Vector3 direction = transform.getForward();
		Vector3 up = transform.getUp();

		FMOD::System* fmod = gFMODAudio().GetFMODInternal();
		FMOD_VECTOR fmodPos = { position.x, position.y, position.z };
		FMOD_VECTOR fmodDir = { direction.x, direction.y, direction.z };
		FMOD_VECTOR fmodUp = { up.x, up.y, up.z };

		fmod->set3DListenerAttributes(mId, &fmodPos, nullptr, &fmodDir, &fmodUp);
	}

	void FMODAudioListener::setVelocity(const Vector3& velocity)
	{
		AudioListener::setVelocity(velocity);

		FMOD::System* fmod = gFMODAudio().GetFMODInternal();
		FMOD_VECTOR value = { velocity.x, velocity.y, velocity.z };

		fmod->set3DListenerAttributes(mId, nullptr, &value, nullptr, nullptr);
	}

	void FMODAudioListener::rebuild(INT32 id)
	{
		mId = id;

		Vector3 position = mTransform.getPosition();
		Vector3 direction = mTransform.getForward();
		Vector3 up = mTransform.getUp();

		FMOD::System* fmod = gFMODAudio().GetFMODInternal();
		FMOD_VECTOR fmodPosition = { position.x, position.y, position.z };
		FMOD_VECTOR fmodVelocity = { mVelocity.x, mVelocity.y, mVelocity.z };
		FMOD_VECTOR fmodForward = { direction.x, direction.y, direction.z };
		FMOD_VECTOR fmodUp = { up.x, up.y, up.z };

		fmod->set3DListenerAttributes(mId, &fmodPosition, &fmodVelocity, &fmodForward, &fmodUp);
	}
}

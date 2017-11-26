//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsFMODAudioListener.h"
#include "BsFMODAudio.h"

namespace bs
{
	FMODAudioListener::FMODAudioListener()
	{
		gFMODAudio()._registerListener(this);
	}

	FMODAudioListener::~FMODAudioListener()
	{
		gFMODAudio()._unregisterListener(this);
	}

	void FMODAudioListener::setTransform(const Transform& transform)
	{
		AudioListener::setTransform(transform);

		Vector3 position = transform.getPosition();
		Vector3 direction = transform.getForward();
		Vector3 up = transform.getUp();

		FMOD::System* fmod = gFMODAudio()._getFMOD();
		FMOD_VECTOR fmodPos = { position.x, position.y, position.z };
		FMOD_VECTOR fmodDir = { direction.x, direction.y, direction.z };
		FMOD_VECTOR fmodUp = { up.x, up.y, up.z };

		fmod->set3DListenerAttributes(mId, &fmodPos, nullptr, &fmodDir, &fmodUp);
	}

	void FMODAudioListener::setVelocity(const Vector3& velocity)
	{
		AudioListener::setVelocity(velocity);

		FMOD::System* fmod = gFMODAudio()._getFMOD();
		FMOD_VECTOR value = { velocity.x, velocity.y, velocity.z };

		fmod->set3DListenerAttributes(mId, nullptr, &value, nullptr, nullptr);
	}

	void FMODAudioListener::rebuild(INT32 id)
	{
		mId = id;

		Vector3 position = mTransform.getPosition();
		Vector3 direction = mTransform.getForward();
		Vector3 up = mTransform.getUp();

		FMOD::System* fmod = gFMODAudio()._getFMOD();
		FMOD_VECTOR fmodPosition = { position.x, position.y, position.z };
		FMOD_VECTOR fmodVelocity = { mVelocity.x, mVelocity.y, mVelocity.z };
		FMOD_VECTOR fmodForward = { direction.x, direction.y, direction.z };
		FMOD_VECTOR fmodUp = { up.x, up.y, up.z };

		fmod->set3DListenerAttributes(mId, &fmodPosition, &fmodVelocity, &fmodForward, &fmodUp);
	}
}
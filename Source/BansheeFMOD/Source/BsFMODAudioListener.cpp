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

	void FMODAudioListener::setPosition(const Vector3& position)
	{
		AudioListener::setPosition(position);

		FMOD::System* fmod = gFMODAudio()._getFMOD();
		FMOD_VECTOR value = { position.x, position.y, position.z };

		fmod->set3DListenerAttributes(mId, &value, nullptr, nullptr, nullptr);
	}

	void FMODAudioListener::setDirection(const Vector3& direction)
	{
		AudioListener::setDirection(direction);

		FMOD::System* fmod = gFMODAudio()._getFMOD();
		FMOD_VECTOR value = { direction.x, direction.y, direction.z };

		fmod->set3DListenerAttributes(mId, nullptr, nullptr, &value, nullptr);
	}

	void FMODAudioListener::setUp(const Vector3& up)
	{
		AudioListener::setUp(up);

		FMOD::System* fmod = gFMODAudio()._getFMOD();
		FMOD_VECTOR value = { up.x, up.y, up.z };

		fmod->set3DListenerAttributes(mId, nullptr, nullptr, nullptr, &value);
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

		FMOD::System* fmod = gFMODAudio()._getFMOD();
		FMOD_VECTOR position = { mPosition.x, mPosition.y, mPosition.z };
		FMOD_VECTOR velocity = { mVelocity.x, mVelocity.y, mVelocity.z };
		FMOD_VECTOR forward = { mDirection.x, mDirection.y, mDirection.z };
		FMOD_VECTOR up = { mUp.x, mUp.y, mUp.z };

		fmod->set3DListenerAttributes(mId, &position, &velocity, &forward, &up);
	}
}
//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DFMODAudioListener.h"
#include "B3DFMODAudio.h"

using namespace b3d;

FMODAudioListener::FMODAudioListener()
{
	GetFMODAudio().RegisterListener(this);
}

FMODAudioListener::~FMODAudioListener()
{
	GetFMODAudio().UnregisterListener(this);
}

void FMODAudioListener::SetTransform(const Transform& transform)
{
	mTransform = transform;

	const Vector3& position = transform.GetPosition();
	const Vector3& direction = transform.GetForward();
	const Vector3& up = transform.GetUp();

	FMOD::System* fmod = GetFMODAudio().GetFMOD();
	FMOD_VECTOR fmodPos = { position.X, position.Y, position.Z };
	FMOD_VECTOR fmodDir = { direction.X, direction.Y, direction.Z };
	FMOD_VECTOR fmodUp = { up.X, up.Y, up.Z };

	fmod->set3DListenerAttributes(mId, &fmodPos, nullptr, &fmodDir, &fmodUp);
}

void FMODAudioListener::SetVelocity(const Vector3& velocity)
{
	mVelocity = velocity;

	FMOD::System* fmod = GetFMODAudio().GetFMOD();
	FMOD_VECTOR value = { velocity.X, velocity.Y, velocity.Z };

	fmod->set3DListenerAttributes(mId, nullptr, &value, nullptr, nullptr);
}

void FMODAudioListener::Rebuild(i32 id)
{
	mId = id;

	const Vector3& position = mTransform.GetPosition();
	const Vector3& direction = mTransform.GetForward();
	const Vector3& up = mTransform.GetUp();

	FMOD::System* fmod = GetFMODAudio().GetFMOD();
	FMOD_VECTOR fmodPosition = { position.X, position.Y, position.Z };
	FMOD_VECTOR fmodVelocity = { mVelocity.X, mVelocity.Y, mVelocity.Z };
	FMOD_VECTOR fmodForward = { direction.X, direction.Y, direction.Z };
	FMOD_VECTOR fmodUp = { up.X, up.Y, up.Y };

	fmod->set3DListenerAttributes(mId, &fmodPosition, &fmodVelocity, &fmodForward, &fmodUp);
}

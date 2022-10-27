//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsFMODAudioListener.h"
#include "BsFMODAudio.h"

using namespace bs;

FMODAudioListener::FMODAudioListener()
{
	gFMODAudio().RegisterListenerInternal(this);
}

FMODAudioListener::~FMODAudioListener()
{
	gFMODAudio().UnregisterListenerInternal(this);
}

void FMODAudioListener::SetTransform(const Transform& transform)
{
	SceneActor::SetTransform(transform);

	Vector3 position = transform.GetPosition();
	Vector3 direction = transform.GetForward();
	Vector3 up = transform.GetUp();

	FMOD::System* fmod = gFMODAudio().GetFMODInternal();
	FMOD_VECTOR fmodPos = { position.X, position.Y, position.Z };
	FMOD_VECTOR fmodDir = { direction.X, direction.Y, direction.Z };
	FMOD_VECTOR fmodUp = { up.X, up.Y, up.Z };

	fmod->set3DListenerAttributes(mId, &fmodPos, nullptr, &fmodDir, &fmodUp);
}

void FMODAudioListener::SetVelocity(const Vector3& velocity)
{
	AudioListener::SetVelocity(velocity);

	FMOD::System* fmod = gFMODAudio().GetFMODInternal();
	FMOD_VECTOR value = { velocity.X, velocity.Y, velocity.Z };

	fmod->set3DListenerAttributes(mId, nullptr, &value, nullptr, nullptr);
}

void FMODAudioListener::Rebuild(i32 id)
{
	mId = id;

	Vector3 position = mTransform.GetPosition();
	Vector3 direction = mTransform.GetForward();
	Vector3 up = mTransform.GetUp();

	FMOD::System* fmod = gFMODAudio().GetFMODInternal();
	FMOD_VECTOR fmodPosition = { position.X, position.Y, position.Z };
	FMOD_VECTOR fmodVelocity = { mVelocity.X, mVelocity.Y, mVelocity.Z };
	FMOD_VECTOR fmodForward = { direction.X, direction.Y, direction.Z };
	FMOD_VECTOR fmodUp = { up.X, up.Y, up.Y };

	fmod->set3DListenerAttributes(mId, &fmodPosition, &fmodVelocity, &fmodForward, &fmodUp);
}

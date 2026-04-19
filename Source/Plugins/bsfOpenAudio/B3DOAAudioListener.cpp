//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DOAAudioListener.h"
#include "B3DOAAudio.h"
#include "AL/al.h"

using namespace b3d;

OAAudioListener::OAAudioListener()
{
	GetOAAudio().RegisterListener(this);
	Rebuild();
}

OAAudioListener::~OAAudioListener()
{
	GetOAAudio().UnregisterListener(this);
}

void OAAudioListener::SetTransform(const Transform& transform)
{
	mTransform = transform;

	std::array<float, 6> orientation = GetOrientation();
	auto& contexts = GetOAAudio().GetContexts();

	if(contexts.size() > 1) // If only one context is available it is guaranteed it is always active, so we can avoid setting it
	{
		auto context = GetOAAudio().GetContext(this);
		alcMakeContextCurrent(context);
	}

	UpdatePosition();
	UpdateOrientation(orientation);
}

void OAAudioListener::SetVelocity(const Vector3& velocity)
{
	mVelocity = velocity;

	auto& contexts = GetOAAudio().GetContexts();
	if(contexts.size() > 1)
	{
		auto context = GetOAAudio().GetContext(this);
		alcMakeContextCurrent(context);
	}

	UpdateVelocity();
}

void OAAudioListener::Rebuild()
{
	auto contexts = GetOAAudio().GetContexts();

	float globalVolume = GetAudio().GetVolume();
	std::array<float, 6> orientation = GetOrientation();

	if(contexts.size() > 1)
	{
		auto context = GetOAAudio().GetContext(this);
		alcMakeContextCurrent(context);
	}

	UpdatePosition();
	UpdateOrientation(orientation);
	UpdateVelocity();
	UpdateVolume(globalVolume);
}

std::array<float, 6> OAAudioListener::GetOrientation() const
{
	const Vector3& direction = mTransform.GetForward();
	const Vector3& up = mTransform.GetUp();

	return { { direction.X,
			   direction.Y,
			   direction.Z,
			   up.X,
			   up.Y,
			   up.Z } };
}

void OAAudioListener::UpdatePosition()
{
	const Vector3& position = mTransform.GetPosition();

	alListener3f(AL_POSITION, position.X, position.Y, position.Z);
}

void OAAudioListener::UpdateOrientation(const std::array<float, 6>& orientation)
{
	alListenerfv(AL_ORIENTATION, orientation.data());
}

void OAAudioListener::UpdateVelocity()
{
	alListener3f(AL_VELOCITY, mVelocity.X, mVelocity.Y, mVelocity.Z);
}

void OAAudioListener::UpdateVolume(float volume)
{
	alListenerf(AL_GAIN, volume);
}

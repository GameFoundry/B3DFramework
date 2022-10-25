//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsOAAudioListener.h"
#include "BsOAAudio.h"
#include "AL/al.h"

namespace bs
{
	OAAudioListener::OAAudioListener()
	{
		gOAAudio().RegisterListenerInternal(this);
		Rebuild();
	}

	OAAudioListener::~OAAudioListener()
	{
		gOAAudio().UnregisterListenerInternal(this);
	}

	void OAAudioListener::SetTransform(const Transform& transform)
	{
		SceneActor::SetTransform(transform);

		std::array<float, 6> orientation = GetOrientation();
		auto& contexts = gOAAudio().GetContextsInternal();

		if(contexts.size() > 1) // If only one context is available it is guaranteed it is always active, so we can avoid setting it
		{
			auto context = gOAAudio().GetContextInternal(this);
			alcMakeContextCurrent(context);
		}

		UpdatePosition();
		UpdateOrientation(orientation);
	}

	void OAAudioListener::SetVelocity(const Vector3& velocity)
	{
		AudioListener::SetVelocity(velocity);

		auto& contexts = gOAAudio().GetContextsInternal();
		if(contexts.size() > 1)
		{
			auto context = gOAAudio().GetContextInternal(this);
			alcMakeContextCurrent(context);
		}

		UpdateVelocity();
	}

	void OAAudioListener::Rebuild()
	{
		auto contexts = gOAAudio().GetContextsInternal();

		float globalVolume = gAudio().GetVolume();
		std::array<float, 6> orientation = GetOrientation();

		if(contexts.size() > 1)
		{
			auto context = gOAAudio().GetContextInternal(this);
			alcMakeContextCurrent(context);
		}

		UpdatePosition();
		UpdateOrientation(orientation);
		UpdateVelocity();
		UpdateVolume(globalVolume);
	}

	std::array<float, 6> OAAudioListener::GetOrientation() const
	{
		Vector3 direction = GetTransform().GetForward();
		Vector3 up = GetTransform().GetUp();

		return { { direction.X,
				   direction.Y,
				   direction.Z,
				   up.X,
				   up.Y,
				   up.Z } };
	}

	void OAAudioListener::UpdatePosition()
	{
		Vector3 position = GetTransform().GetPosition();

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
} // namespace bs

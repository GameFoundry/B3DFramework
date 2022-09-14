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
		rebuild();
	}

	OAAudioListener::~OAAudioListener()
	{
		gOAAudio().UnregisterListenerInternal(this);
	}

	void OAAudioListener::setTransform(const Transform& transform)
	{
		AudioListener::setTransform(transform);

		std::array<float, 6> orientation = getOrientation();
		auto& contexts = gOAAudio().GetContextsInternal();

		if (contexts.size() > 1) // If only one context is available it is guaranteed it is always active, so we can avoid setting it
		{
			auto context = gOAAudio().GetContextInternal(this);
			alcMakeContextCurrent(context);
		}

		updatePosition();
		updateOrientation(orientation);
	}

	void OAAudioListener::setVelocity(const Vector3& velocity)
	{
		AudioListener::setVelocity(velocity);

		auto& contexts = gOAAudio().GetContextsInternal();
		if (contexts.size() > 1)
		{
			auto context = gOAAudio().GetContextInternal(this);
			alcMakeContextCurrent(context);
		}

		updateVelocity();
	}

	void OAAudioListener::rebuild()
	{
		auto contexts = gOAAudio().GetContextsInternal();
		
		float globalVolume = gAudio().getVolume();
		std::array<float, 6> orientation = getOrientation();

		if (contexts.size() > 1)
		{
			auto context = gOAAudio().GetContextInternal(this);
			alcMakeContextCurrent(context);
		}

		updatePosition();
		updateOrientation(orientation);
		updateVelocity();
		updateVolume(globalVolume);
	}

	std::array<float, 6> OAAudioListener::getOrientation() const
	{
		Vector3 direction = getTransform().getForward();
		Vector3 up = getTransform().getUp();

		return
		{{
			direction.x,
			direction.y,
			direction.z,
			up.x,
			up.y,
			up.z
		}};
	}

	void OAAudioListener::updatePosition()
	{
		Vector3 position = getTransform().getPosition();

		alListener3f(AL_POSITION, position.x, position.y, position.z);
	}

	void OAAudioListener::updateOrientation(const std::array<float, 6>& orientation)
	{
		alListenerfv(AL_ORIENTATION, orientation.data());
	}

	void OAAudioListener::updateVelocity()
	{
		alListener3f(AL_VELOCITY, mVelocity.x, mVelocity.y, mVelocity.z);
	}

	void OAAudioListener::updateVolume(float volume)
	{
		alListenerf(AL_GAIN, volume);
	}
}

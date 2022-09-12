//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsOAAudioListener.h"
#include "BsOAAudio.h"
#include "AL/al.h"

namespace bs
{
	OAAudioListener::OAAudioListener()
	{
		gOAAudio()._registerListener(this);
		rebuild();
	}

	OAAudioListener::~OAAudioListener()
	{
		gOAAudio()._unregisterListener(this);
	}

	void OAAudioListener::SetTransform(const Transform& transform)
	{
		AudioListener::setTransform(transform);

		std::array<float, 6> orientation = getOrientation();
		auto& contexts = gOAAudio()._getContexts();

		if (contexts.size() > 1) // If only one context is available it is guaranteed it is always active, so we can avoid setting it
		{
			auto context = gOAAudio()._getContext(this);
			alcMakeContextCurrent(context);
		}

		updatePosition();
		updateOrientation(orientation);
	}

	void OAAudioListener::SetVelocity(const Vector3& velocity)
	{
		AudioListener::setVelocity(velocity);

		auto& contexts = gOAAudio()._getContexts();
		if (contexts.size() > 1)
		{
			auto context = gOAAudio()._getContext(this);
			alcMakeContextCurrent(context);
		}

		updateVelocity();
	}

	void OAAudioListener::Rebuild()
	{
		auto contexts = gOAAudio()._getContexts();
		
		float globalVolume = gAudio().getVolume();
		std::array<float, 6> orientation = getOrientation();

		if (contexts.size() > 1)
		{
			auto context = gOAAudio()._getContext(this);
			alcMakeContextCurrent(context);
		}

		updatePosition();
		updateOrientation(orientation);
		updateVelocity();
		updateVolume(globalVolume);
	}

	std::array<float, 6> OAAudioListener::GetOrientation() const
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

	void OAAudioListener::UpdatePosition()
	{
		Vector3 position = getTransform().getPosition();

		alListener3f(AL_POSITION, position.x, position.y, position.z);
	}

	void OAAudioListener::UpdateOrientation(const std::array<float, 6>& orientation)
	{
		alListenerfv(AL_ORIENTATION, orientation.data());
	}

	void OAAudioListener::UpdateVelocity()
	{
		alListener3f(AL_VELOCITY, mVelocity.x, mVelocity.y, mVelocity.z);
	}

	void OAAudioListener::UpdateVolume(float volume)
	{
		alListenerf(AL_GAIN, volume);
	}
}

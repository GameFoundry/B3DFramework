//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsFMODAudioSource.h"
#include "BsFMODAudio.h"
#include "BsFMODAudioClip.h"

namespace BansheeEngine
{
	FMODAudioSource::FMODAudioSource()
	{
		gFMODAudio()._registerSource(this);
		// TODO
	}

	FMODAudioSource::~FMODAudioSource()
	{
		gFMODAudio()._unregisterSource(this);
		// TODO
	}

	void FMODAudioSource::setClip(const HAudioClip& clip)
	{
		AudioSource::setClip(clip);

		// TODO
	}

	void FMODAudioSource::setPosition(const Vector3& position)
	{
		AudioSource::setPosition(position);

		// TODO
	}

	void FMODAudioSource::setVelocity(const Vector3& velocity)
	{
		AudioSource::setVelocity(velocity);

		// TODO
	}

	void FMODAudioSource::setVolume(float volume)
	{
		AudioSource::setVolume(volume);

		// TODO
	}

	void FMODAudioSource::setPitch(float pitch)
	{
		AudioSource::setPitch(pitch);

		// TODO
	}

	void FMODAudioSource::setIsLooping(bool loop)
	{
		AudioSource::setIsLooping(loop);

		// TODO
	}

	void FMODAudioSource::setPriority(UINT32 priority)
	{
		AudioSource::setPriority(priority);

		// TODO
	}

	void FMODAudioSource::setMinDistance(float distance)
	{
		AudioSource::setMinDistance(distance);

		// TODO
	}

	void FMODAudioSource::setAttenuation(float attenuation)
	{
		AudioSource::setAttenuation(attenuation);

		// TODO
	}

	void FMODAudioSource::play()
	{
		AudioSource::play();

		// TODO
	}

	void FMODAudioSource::pause()
	{
		AudioSource::pause();

		// TODO
	}

	void FMODAudioSource::stop()
	{
		AudioSource::stop();

		// TODO
	}

	void FMODAudioSource::setTime(float time)
	{
		// TODO
	}

	float FMODAudioSource::getTime() const
	{
		// TODO

		return 0.0f;
	}
}
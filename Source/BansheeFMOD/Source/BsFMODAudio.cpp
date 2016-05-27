//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsFMODAudio.h"
#include "BsMath.h"

namespace BansheeEngine
{
	FMODAudio::FMODAudio()
		:mVolume(1.0f), mIsPaused(false)
	{
		// TODO - Enumerate devices, init FMOD
	}

	FMODAudio::~FMODAudio()
	{
		// TODO - Shutdown FMOD
	}

	void FMODAudio::setVolume(float volume)
	{
		mVolume = Math::clamp01(volume);

		// TODO
	}

	float FMODAudio::getVolume() const
	{
		return mVolume;
	}

	void FMODAudio::setPaused(bool paused)
	{
		if (mIsPaused == paused)
			return;

		mIsPaused = paused;

		// TODO
	}

	void FMODAudio::_update()
	{
		// TODO - Call FMOD update, update streaming?
	}

	void FMODAudio::setActiveDevice(const AudioDevice& device)
	{
		// TODO
	}

	SPtr<AudioClip> FMODAudio::createClip(const SPtr<DataStream>& samples, UINT32 streamSize, UINT32 numSamples,
		const AUDIO_CLIP_DESC& desc)
	{
		// TODO
		return nullptr;
	}

	SPtr<AudioListener> FMODAudio::createListener()
	{
		// TODO
		return nullptr;
	}

	SPtr<AudioSource> FMODAudio::createSource()
	{
		// TODO
		return nullptr;
	}

	FMODAudio& gFMODAudio()
	{
		return static_cast<FMODAudio&>(FMODAudio::instance());
	}
}
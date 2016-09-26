//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsAudio.h"
#include "BsAudioSource.h"

namespace BansheeEngine
{
	void Audio::play(const HAudioClip& clip, const Vector3& position, float volume)
	{
		SPtr<AudioSource> source = createSource();
		source->setClip(clip);
		source->setPosition(position);
		source->setVolume(volume);
		source->play();

		mManualSources.push_back(source);
	}

	void Audio::stopManualSources()
	{
		for (auto& source : mManualSources)
			source->stop();

		mManualSources.clear();
	}

	void Audio::_update()
	{
		UINT32 numSources = (UINT32)mManualSources.size();
		for(UINT32 i = 0; i < numSources; i++)
		{
			if (mManualSources[i]->getState() != AudioSourceState::Stopped)
				mTempSources.push_back(mManualSources[i]);
		}

		std::swap(mTempSources, mManualSources);
		mTempSources.clear();
	}

	Audio& gAudio()
	{
		return Audio::instance();
	}
}
//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Audio/BsAudio.h"
#include "Audio/BsAudioSource.h"

namespace bs
{
	void Audio::play(const HAudioClip& clip, const Vector3& position, float volume)
	{
		Transform transform;
		transform.setPosition(position);

		SPtr<AudioSource> source = createSource();
		source->setClip(clip);
		source->setTransform(transform);
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
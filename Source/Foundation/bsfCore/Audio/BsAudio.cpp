//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
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
		const UINT32 numSources = (UINT32)mManualSources.size();
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
//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Audio/BsAudio.h"
#include "Audio/BsAudioSource.h"

using namespace bs;

void Audio::Play(const HAudioClip& clip, const Vector3& position, float volume)
{
	Transform transform;
	transform.SetPosition(position);

	SPtr<AudioSource> source = CreateSource();
	source->SetClip(clip);
	source->SetTransform(transform);
	source->SetVolume(volume);
	source->Play();

	mManualSources.push_back(source);
}

void Audio::StopManualSources()
{
	for(auto& source : mManualSources)
		source->Stop();

	mManualSources.clear();
}

void Audio::UpdateInternal()
{
	const u32 numSources = (u32)mManualSources.size();
	for(u32 i = 0; i < numSources; i++)
	{
		if(mManualSources[i]->GetState() != AudioSourceState::Stopped)
			mTempSources.push_back(mManualSources[i]);
	}

	std::swap(mTempSources, mManualSources);
	mTempSources.clear();
}

namespace bs
{
Audio& GetAudio()
{
	return Audio::Instance();
}
} // namespace bs

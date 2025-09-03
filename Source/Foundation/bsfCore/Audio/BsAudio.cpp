//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Audio/BsAudio.h"
#include "Components/BsCAudioSource.h"
#include "Scene/BsTransform.h"

using namespace b3d;

void Audio::Play(const HAudioClip& clip, const Vector3& position, float volume)
{
	Transform transform;
	transform.SetPosition(position);

	SPtr<IAudioSourceImplementation> source = CreateSource();
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

void Audio::Update()
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

namespace b3d
{
Audio& GetAudio()
{
	return Audio::Instance();
}
} // namespace b3d

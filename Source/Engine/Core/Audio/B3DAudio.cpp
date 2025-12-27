//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Audio/B3DAudio.h"
#include "Components/B3DAudioSource.h"
#include "Scene/B3DTransform.h"

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
	const u32 sourceCount = (u32)mManualSources.size();
	for(u32 sourceIndex = 0; sourceIndex < sourceCount; sourceIndex++)
	{
		if(mManualSources[sourceIndex]->GetState() != AudioSourceState::Stopped)
			mTempSources.push_back(mManualSources[sourceIndex]);
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

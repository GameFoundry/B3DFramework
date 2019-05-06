//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Audio/BsAudioClip.h"
#include "Resources/BsResources.h"
#include "Audio/BsAudio.h"
#include "Private/RTTI/BsAudioClipRTTI.h"

namespace bs
{
	AudioClip::AudioClip(const SPtr<DataStream>& samples, UINT32 streamSize, UINT32 numSamples, const AUDIO_CLIP_DESC& desc)
		: Resource(false), mDesc(desc), mNumSamples(numSamples), mStreamSize(streamSize), mStreamData(samples)
	{
		if (samples != nullptr)
			mStreamOffset = (UINT32)samples->tell();

		mKeepSourceData = desc.keepSourceData;
	}

	void AudioClip::initialize()
	{
		mLength = mNumSamples / mDesc.numChannels / (float)mDesc.frequency;

		Resource::initialize();
	}

	HAudioClip AudioClip::create(const SPtr<DataStream>& samples, UINT32 streamSize, UINT32 numSamples, const AUDIO_CLIP_DESC& desc)
	{
		return static_resource_cast<AudioClip>(gResources()._createResourceHandle(_createPtr(samples, streamSize, numSamples, desc)));
	}

	SPtr<AudioClip> AudioClip::_createPtr(const SPtr<DataStream>& samples, UINT32 streamSize, UINT32 numSamples, const AUDIO_CLIP_DESC& desc)
	{
		SPtr<AudioClip> newClip = gAudio().createClip(samples, streamSize, numSamples, desc);
		newClip->_setThisPtr(newClip);
		newClip->initialize();

		return newClip;
	}

	SPtr<AudioClip> AudioClip::createEmpty()
	{
		AUDIO_CLIP_DESC desc;

		SPtr<AudioClip> newClip = gAudio().createClip(nullptr, 0, 0, desc);
		newClip->_setThisPtr(newClip);

		return newClip;
	}

	RTTITypeBase* AudioClip::getRTTIStatic()
	{
		return AudioClipRTTI::instance();
	}

	RTTITypeBase* AudioClip::getRTTI() const
	{
		return getRTTIStatic();
	}
}
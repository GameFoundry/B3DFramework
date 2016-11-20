//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsAudioClip.h"
#include "BsResources.h"
#include "BsAudio.h"
#include "BsAudioClipRTTI.h"

namespace bs
{
	AudioClip::AudioClip(const SPtr<DataStream>& samples, UINT32 streamSize, UINT32 numSamples, const AUDIO_CLIP_DESC& desc)
		: Resource(false), mDesc(desc), mNumSamples(numSamples), mStreamData(samples), mStreamSize(streamSize)
		, mStreamOffset(0), mLength(0.0f)
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
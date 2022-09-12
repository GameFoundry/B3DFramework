//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsNullAudio.h"
#include "Threading/BsTaskScheduler.h"
#include "FileSystem/BsDataStream.h"

namespace bs
{
	NullAudio::NullAudio()
	{
		mDefaultDevice.name = "NullDevice";
		mActiveDevice = mDefaultDevice;
		mAllDevices.push_back(mActiveDevice);
	}

	SPtr<AudioClip> NullAudio::createClip(const SPtr<DataStream>& samples, UINT32 streamSize, UINT32 numSamples,
		const AUDIO_CLIP_DESC& desc)
	{
		return bs_core_ptr_new<NullAudioClip>(samples, streamSize, numSamples, desc);
	}

	SPtr<AudioListener> NullAudio::CreateListener()
	{
		return bs_shared_ptr_new<NullAudioListener>();
	}

	SPtr<AudioSource> NullAudio::CreateSource()
	{
		return bs_shared_ptr_new<NullAudioSource>();
	}

	NullAudioClip::NullAudioClip(const SPtr<DataStream>& samples, UINT32 streamSize, UINT32 numSamples, const AUDIO_CLIP_DESC& desc)
		:AudioClip(samples, streamSize, numSamples, desc)
	{ }

	void NullAudioClip::Initialize()
	{
		// If we need to keep source data, read everything into memory and keep a copy
		if (mKeepSourceData)
		{
			mStreamData->seek(mStreamOffset);

			UINT8* sampleBuffer = (UINT8*)bs_alloc(mStreamSize);
			mStreamData->read(sampleBuffer, mStreamSize);

			mSourceStreamData = bs_shared_ptr_new<MemoryDataStream>(sampleBuffer, mStreamSize);
			mSourceStreamSize = mStreamSize;
		}

		AudioClip::initialize();
	}

	SPtr<DataStream> NullAudioClip::GetSourceStream(UINT32& size)
	{
		size = mSourceStreamSize;
		mSourceStreamData->seek(0);

		return mSourceStreamData;
	}

	NullAudio& GNullAudio()
	{
		return static_cast<NullAudio&>(NullAudio::instance());
	}
}

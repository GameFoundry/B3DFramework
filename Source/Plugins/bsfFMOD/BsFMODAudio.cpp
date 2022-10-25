//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsFMODAudio.h"
#include "BsFMODAudioClip.h"
#include "BsFMODAudioSource.h"
#include "BsFMODAudioListener.h"
#include "Math/BsMath.h"

namespace bs
{
	void* F_CALLBACK FMODAlloc(unsigned int size, FMOD_MEMORY_TYPE type, const char* sourcestr)
	{
		return bs_alloc(size);
	}

	void* F_CALLBACK FMODRealloc(void* ptr, unsigned int size, FMOD_MEMORY_TYPE type, const char* sourcestr)
	{
		// Note: Not using framework's allocators, but have no easy alternative to implement realloc manually.
		// This is okay to use in combination with general purpose bs_alloc/bs_free since they internally use malloc/free.
		return realloc(ptr, size);
	}

	void F_CALLBACK FMODFree(void* ptr, FMOD_MEMORY_TYPE type, const char* sourcestr)
	{
		bs_free(ptr);
	}

	float F_CALLBACK FMOD3DRolloff(FMOD_CHANNELCONTROL* channelControl, float distance)
	{
		FMODAudioSource* source = nullptr;
		FMOD::ChannelControl* channel = (FMOD::ChannelControl*)channelControl;
		channel->getUserData((void**)&source);

		if(source == nullptr)
			return 1.0f;

		// Calculate standard inverse rolloff, but use different attenuation per source (also ignore max distance)
		float minDistance = source->GetMinDistance();
		float attenuation = source->GetAttenuation();

		distance = std::max(distance, minDistance);
		return minDistance / (minDistance + attenuation * (distance - minDistance));
	}

	FMODAudio::FMODAudio()
	{
		FMOD::Memory_Initialize(nullptr, 0, &FMODAlloc, &FMODRealloc, &FMODFree);
		FMOD::System_Create(&mFMOD);

		FMOD_ADVANCEDSETTINGS advancedSettings;
		memset(&advancedSettings, 0, sizeof(advancedSettings));
		advancedSettings.cbSize = sizeof(advancedSettings);
		advancedSettings.vol0virtualvol = 0.001f;

		mFMOD->setAdvancedSettings(&advancedSettings);
		mFMOD->init(512, FMOD_INIT_3D_RIGHTHANDED | FMOD_INIT_VOL0_BECOMES_VIRTUAL, nullptr);
		mFMOD->setStreamBufferSize(65536, FMOD_TIMEUNIT_RAWBYTES);
		mFMOD->set3DRolloffCallback(&FMOD3DRolloff);

		mFMOD->getMasterChannelGroup(&mMasterChannelGroup);

		i32 numDevices;
		mFMOD->getNumDrivers(&numDevices);

		mAllDevices.resize(numDevices);
		char nameBuffer[256];
		for(i32 i = 0; i < numDevices; i++)
		{
			mFMOD->getDriverInfo(i, nameBuffer, sizeof(nameBuffer), nullptr, nullptr, nullptr, nullptr);
			mAllDevices[i].name = String(nameBuffer);
		}

		i32 defaultDevice = 0;
		mFMOD->getDriver(&defaultDevice);
		if(defaultDevice < numDevices)
		{
			mDefaultDevice.name = mAllDevices[defaultDevice].name;
			mActiveDevice.name = mAllDevices[defaultDevice].name;
		}
	}

	FMODAudio::~FMODAudio()
	{
		StopManualSources();

		assert(mListeners.empty() && mSources.empty()); // Everything should be destroyed at this point
		mFMOD->release();
	}

	void FMODAudio::SetVolume(float volume)
	{
		mVolume = Math::Clamp01(volume);
		mMasterChannelGroup->setVolume(mVolume);
	}

	float FMODAudio::GetVolume() const
	{
		return mVolume;
	}

	void FMODAudio::SetPaused(bool paused)
	{
		if(mIsPaused == paused)
			return;

		mIsPaused = paused;

		for(auto& source : mSources)
			source->SetGlobalPause(paused);
	}

	void FMODAudio::UpdateInternal()
	{
		mFMOD->update();

		Audio::UpdateInternal();
	}

	void FMODAudio::SetActiveDevice(const AudioDevice& device)
	{
		for(u32 i = 0; i < (u32)mAllDevices.size(); i++)
		{
			if(device.name == mAllDevices[i].name)
			{
				mFMOD->setDriver(i);
				return;
			}
		}

		BS_LOG(Warning, Audio, "Failed changing audio device to: {0}", device.name);
	}

	SPtr<AudioClip> FMODAudio::CreateClip(const SPtr<DataStream>& samples, u32 streamSize, u32 numSamples, const AUDIO_CLIP_DESC& desc)
	{
		return bs_core_ptr_new<FMODAudioClip>(samples, streamSize, numSamples, desc);
	}

	SPtr<AudioListener> FMODAudio::CreateListener()
	{
		return bs_shared_ptr_new<FMODAudioListener>();
	}

	SPtr<AudioSource> FMODAudio::CreateSource()
	{
		return bs_shared_ptr_new<FMODAudioSource>();
	}

	void FMODAudio::RegisterListenerInternal(FMODAudioListener* listener)
	{
		mListeners.push_back(listener);

		RebuildListeners();
	}

	void FMODAudio::UnregisterListenerInternal(FMODAudioListener* listener)
	{
		auto iterFind = std::find(mListeners.begin(), mListeners.end(), listener);
		if(iterFind != mListeners.end())
			mListeners.erase(iterFind);

		RebuildListeners();
	}

	void FMODAudio::RebuildListeners()
	{
		i32 numListeners = (i32)mListeners.size();
		if(numListeners > 0)
		{
			mFMOD->set3DNumListeners(numListeners);
			for(i32 i = 0; i < numListeners; i++)
				mListeners[i]->Rebuild(i);
		}
		else // Always keep at least one listener
		{
			mFMOD->set3DNumListeners(1);
			FMOD_VECTOR zero = { 0.0f, 0.0f, 0.0f };
			FMOD_VECTOR forward = { 0.0f, 0.0f, -1.0f };
			FMOD_VECTOR up = { 0.0f, 1.0f, 0.0f };

			mFMOD->set3DListenerAttributes(0, &zero, &zero, &forward, &up);
		}
	}

	void FMODAudio::RegisterSourceInternal(FMODAudioSource* source)
	{
		mSources.insert(source);
	}

	void FMODAudio::UnregisterSourceInternal(FMODAudioSource* source)
	{
		mSources.erase(source);
	}

	FMODAudio& gFMODAudio()
	{
		return static_cast<FMODAudio&>(FMODAudio::Instance());
	}
} // namespace bs

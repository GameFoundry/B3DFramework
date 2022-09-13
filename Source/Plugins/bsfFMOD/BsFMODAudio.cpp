//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsFMODAudio.h"
#include "BsFMODAudioClip.h"
#include "BsFMODAudioSource.h"
#include "BsFMODAudioListener.h"
#include "Math/BsMath.h"

namespace bs
{
	void* F_CALLBACK FMODAlloc(unsigned int size, FMOD_MEMORY_TYPE type, const char *sourcestr)
	{
		return bs_alloc(size);
	}

	void* F_CALLBACK FMODRealloc(void *ptr, unsigned int size, FMOD_MEMORY_TYPE type, const char *sourcestr)
	{
		// Note: Not using framework's allocators, but have no easy alternative to implement realloc manually.
		// This is okay to use in combination with general purpose bs_alloc/bs_free since they internally use malloc/free.
		return Realloc(ptr, size);
	}

	void F_CALLBACK FMODFree(void *ptr, FMOD_MEMORY_TYPE type, const char *sourcestr)
	{
		bs_free(ptr);
	}

	float F_CALLBACK FMOD3DRolloff(FMOD_CHANNELCONTROL* channelControl, float distance)
	{
		FMODAudioSource* source = nullptr;
		FMOD::ChannelControl* channel = (FMOD::ChannelControl*)channelControl;
		channel->GetUserData((void**)&source);

		if (source == nullptr)
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

		mFMOD->SetAdvancedSettings(&advancedSettings);
		mFMOD->Init(512, FMOD_INIT_3D_RIGHTHANDED | FMOD_INIT_VOL0_BECOMES_VIRTUAL, nullptr);
		mFMOD->SetStreamBufferSize(65536, FMOD_TIMEUNIT_RAWBYTES);
		mFMOD->Set3DRolloffCallback(&FMOD3DRolloff);

		mFMOD->GetMasterChannelGroup(&mMasterChannelGroup);

		INT32 numDevices;
		mFMOD->GetNumDrivers(&numDevices);

		mAllDevices.Resize(numDevices);
		char nameBuffer[256];
		for(INT32 i = 0; i < numDevices; i++)
		{
			mFMOD->GetDriverInfo(i, nameBuffer, sizeof(nameBuffer), nullptr, nullptr, nullptr, nullptr);
			mAllDevices[i].name = String(nameBuffer);
		}

		INT32 defaultDevice = 0;
		mFMOD->GetDriver(&defaultDevice);
		if(defaultDevice < numDevices)
		{
			mDefaultDevice.name = mAllDevices[defaultDevice].name;
			mActiveDevice.name = mAllDevices[defaultDevice].name;
		}
	}

	FMODAudio::~FMODAudio()
	{
		stopManualSources();

		assert(mListeners.empty() && mSources.empty()); // Everything should be destroyed at this point
		mFMOD->Release();
	}

	void FMODAudio::SetVolume(float volume)
	{
		mVolume = Math::Clamp01(volume);
		mMasterChannelGroup->SetVolume(mVolume);
	}

	float FMODAudio::GetVolume() const
	{
		return mVolume;
	}

	void FMODAudio::SetPaused(bool paused)
	{
		if (mIsPaused == paused)
			return;

		mIsPaused = paused;

		for (auto& source : mSources)
			source->SetGlobalPause(paused);
	}

	void FMODAudio::_update()
	{
		mFMOD->Update();

		Audio::_update();
	}

	void FMODAudio::SetActiveDevice(const AudioDevice& device)
	{
		for(UINT32 i = 0; i < (UINT32)mAllDevices.size(); i++)
		{
			if(device.name == mAllDevices[i].name)
			{
				mFMOD->SetDriver(i);
				return;
			}
		}

		BS_LOG(Warning, Audio, "Failed changing audio device to: {0}", device.name);
	}

	SPtr<AudioClip> FMODAudio::createClip(const SPtr<DataStream>& samples, UINT32 streamSize, UINT32 numSamples,
		const AUDIO_CLIP_DESC& desc)
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

	void FMODAudio::_registerListener(FMODAudioListener* listener)
	{
		mListeners.push_back(listener);

		rebuildListeners();
	}

	void FMODAudio::_unregisterListener(FMODAudioListener* listener)
	{
		auto iterFind = std::find(mListeners.begin(), mListeners.end(), listener);
		if (iterFind != mListeners.end())
			mListeners.erase(iterFind);

		rebuildListeners();
	}

	void FMODAudio::RebuildListeners()
	{
		INT32 numListeners = (INT32)mListeners.size();
		if (numListeners > 0)
		{
			mFMOD->Set3DNumListeners(numListeners);
			for (INT32 i = 0; i < numListeners; i++)
				mListeners[i]->Rebuild(i);
		}
		else // Always keep at least one listener
		{
			mFMOD->Set3DNumListeners(1);
			FMOD_VECTOR zero = { 0.0f, 0.0f, 0.0f };
			FMOD_VECTOR forward = { 0.0f, 0.0f, -1.0f };
			FMOD_VECTOR up = { 0.0f, 1.0f, 0.0f };

			mFMOD->Set3DListenerAttributes(0, &zero, &zero, &forward, &up);
		}
	}

	void FMODAudio::_registerSource(FMODAudioSource* source)
	{
		mSources.Insert(source);
	}

	void FMODAudio::_unregisterSource(FMODAudioSource* source)
	{
		mSources.erase(source);
	}

	FMODAudio& GFMODAudio()
	{
		return static_cast<FMODAudio&>(FMODAudio::instance());
	}
}

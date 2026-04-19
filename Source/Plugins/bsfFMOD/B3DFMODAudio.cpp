//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DFMODAudio.h"
#include "B3DFMODAudioClip.h"
#include "B3DFMODAudioSource.h"
#include "B3DFMODAudioListener.h"
#include "Math/B3DMath.h"

using namespace b3d;

void* F_CALLBACK FMODAlloc(unsigned int size, FMOD_MEMORY_TYPE type, const char* sourcestr)
{
	return B3DAllocate(size);
}

void* F_CALLBACK FMODRealloc(void* ptr, unsigned int size, FMOD_MEMORY_TYPE type, const char* sourcestr)
{
	// Note: Not using framework's allocators, but have no easy alternative to implement realloc manually.
	// This is okay to use in combination with general purpose B3DAllocate/B3DFree since they internally use malloc/free.
	return realloc(ptr, size);
}

void F_CALLBACK FMODFree(void* ptr, FMOD_MEMORY_TYPE type, const char* sourcestr)
{
	B3DFree(ptr);
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
		mAllDevices[i].Name = String(nameBuffer);
	}

	i32 defaultDevice = 0;
	mFMOD->getDriver(&defaultDevice);
	if(defaultDevice < numDevices)
	{
		mDefaultDevice.Name = mAllDevices[defaultDevice].Name;
		mActiveDevice.Name = mAllDevices[defaultDevice].Name;
	}
}

FMODAudio::~FMODAudio()
{
	StopManualSources();

	B3D_ASSERT(mListeners.empty() && mSources.empty()); // Everything should be destroyed at this point
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

void FMODAudio::Update()
{
	mFMOD->update();

	Audio::Update();
}

void FMODAudio::SetActiveDevice(const AudioDevice& device)
{
	for(u32 i = 0; i < (u32)mAllDevices.size(); i++)
	{
		if(device.Name == mAllDevices[i].Name)
		{
			mFMOD->setDriver(i);
			return;
		}
	}

	B3D_LOG(Warning, LogAudio, "Failed changing audio device to: {0}", device.Name);
}

SPtr<AudioClip> FMODAudio::CreateClip(const SPtr<DataStream>& samples, u32 streamSize, u32 sampleCount, const AudioClipCreateInformation& createInformation)
{
	return B3DMakeShared<FMODAudioClip>(samples, streamSize, sampleCount, createInformation);
}

SPtr<IAudioListenerImplementation> FMODAudio::CreateListener()
{
	return B3DMakeShared<FMODAudioListener>();
}

SPtr<IAudioSourceImplementation> FMODAudio::CreateSource()
{
	return B3DMakeShared<FMODAudioSource>();
}

void FMODAudio::RegisterListener(FMODAudioListener* listener)
{
	mListeners.push_back(listener);

	RebuildListeners();
}

void FMODAudio::UnregisterListener(FMODAudioListener* listener)
{
	auto iterFind = std::find(mListeners.begin(), mListeners.end(), listener);
	if(iterFind != mListeners.end())
		mListeners.erase(iterFind);

	RebuildListeners();
}

void FMODAudio::RebuildListeners()
{
	i32 listenerCount = (i32)mListeners.size();
	if(listenerCount > 0)
	{
		mFMOD->set3DNumListeners(listenerCount);
		for(i32 i = 0; i < listenerCount; i++)
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

void FMODAudio::RegisterSource(FMODAudioSource* source)
{
	mSources.insert(source);
}

void FMODAudio::UnregisterSource(FMODAudioSource* source)
{
	mSources.erase(source);
}

namespace b3d {
FMODAudio& GetFMODAudio()
{
	return static_cast<FMODAudio&>(FMODAudio::Instance());
}
} // namespace b3d

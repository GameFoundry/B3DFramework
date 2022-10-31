//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsFMODPrerequisites.h"
#include "Audio/BsAudio.h"
#include "fmod.hpp"

namespace bs
{
	/** @addtogroup FMOD
	 *  @{
	 */

	/** Global manager for the audio implementation using FMOD as the backend. */
	class FMODAudio : public Audio
	{
	public:
		FMODAudio();
		virtual ~FMODAudio();

		void SetVolume(float volume) override;
		float GetVolume() const override;
		void SetPaused(bool paused) override;
		bool IsPaused() const override { return mIsPaused; }
		void UpdateInternal() override;
		void SetActiveDevice(const AudioDevice& device) override;
		AudioDevice GetActiveDevice() const override { return mActiveDevice; }
		AudioDevice GetDefaultDevice() const override { return mDefaultDevice; }
		const Vector<AudioDevice>& GetAllDevices() const override { return mAllDevices; }

		/** @name Internal
		 *  @{
		 */

		/** Registers a new AudioListener. Should be called on listener creation. */
		void RegisterListenerInternal(FMODAudioListener* listener);

		/** Unregisters an existing AudioListener. Should be called before listener destruction. */
		void UnregisterListenerInternal(FMODAudioListener* listener);

		/** Registers a new AudioSource. Should be called on source creation. */
		void RegisterSourceInternal(FMODAudioSource* source);

		/** Unregisters an existing AudioSource. Should be called before source destruction. */
		void UnregisterSourceInternal(FMODAudioSource* source);

		/** Returns internal FMOD system instance. */
		FMOD::System* GetFMODInternal() const { return mFMOD; }

		/** @} */
	private:
		SPtr<AudioClip> CreateClip(const SPtr<DataStream>& samples, u32 streamSize, u32 numSamples, const AUDIO_CLIP_DESC& desc) override;
		SPtr<AudioListener> CreateListener() override;
		SPtr<AudioSource> CreateSource() override;

		/** Rebuilds information about all listeners. Should be called when listener list changes. */
		void RebuildListeners();

		FMOD::System* mFMOD = nullptr;
		FMOD::ChannelGroup* mMasterChannelGroup = nullptr;

		float mVolume = 1.0f;
		bool mIsPaused = false;

		Vector<FMODAudioListener*> mListeners;
		UnorderedSet<FMODAudioSource*> mSources;

		Vector<AudioDevice> mAllDevices;
		AudioDevice mDefaultDevice;
		AudioDevice mActiveDevice;
	};

	/** Provides easier access to FMODAudio. */
	FMODAudio& GetFMODAudio();

	/** @} */
} // namespace bs

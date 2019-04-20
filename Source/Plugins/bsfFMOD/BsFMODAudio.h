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

		/** @copydoc Audio::setVolume */
		void setVolume(float volume) override;

		/** @copydoc Audio::getVolume */
		float getVolume() const override;

		/** @copydoc Audio::setPaused */
		void setPaused(bool paused) override;

		/** @copydoc Audio::isPaused */
		bool isPaused() const override { return mIsPaused; }

		/** @copydoc Audio::_update */
		void _update() override;

		/** @copydoc Audio::setActiveDevice */
		void setActiveDevice(const AudioDevice& device) override;

		/** @copydoc Audio::getActiveDevice */
		AudioDevice getActiveDevice() const override { return mActiveDevice; }

		/** @copydoc Audio::getDefaultDevice */
		AudioDevice getDefaultDevice() const override { return mDefaultDevice; }

		/** @copydoc Audio::getAllDevices */
		const Vector<AudioDevice>& getAllDevices() const override { return mAllDevices; }

		/** @name Internal 
		 *  @{
		 */

		/** Registers a new AudioListener. Should be called on listener creation. */
		void _registerListener(FMODAudioListener* listener);

		/** Unregisters an existing AudioListener. Should be called before listener destruction. */
		void _unregisterListener(FMODAudioListener* listener);

		/** Registers a new AudioSource. Should be called on source creation. */
		void _registerSource(FMODAudioSource* source);

		/** Unregisters an existing AudioSource. Should be called before source destruction. */
		void _unregisterSource(FMODAudioSource* source);

		/** Returns internal FMOD system instance. */
		FMOD::System* _getFMOD() const { return mFMOD; }

		/** @} */
	private:
		/** @copydoc Audio::createClip */
		SPtr<AudioClip> createClip(const SPtr<DataStream>& samples, UINT32 streamSize, UINT32 numSamples,
			const AUDIO_CLIP_DESC& desc) override;

		/** @copydoc Audio::createListener */
		SPtr<AudioListener> createListener() override;

		/** @copydoc Audio::createSource */
		SPtr<AudioSource> createSource() override;

		/** Rebuilds information about all listeners. Should be called when listener list changes. */
		void rebuildListeners();

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
	FMODAudio& gFMODAudio();

	/** @} */
}
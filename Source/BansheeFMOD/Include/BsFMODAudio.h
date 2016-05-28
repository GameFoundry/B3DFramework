//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsFMODPrerequisites.h"
#include "BsAudio.h"
#include "fmod.hpp"

namespace BansheeEngine
{
	/** @addtogroup FMOD
	 *  @{
	 */
	
	/** Global manager for the audio implementation using FMOD as the backend. */
	class BS_FMOD_EXPORT FMODAudio : public Audio
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

		/** @copydoc Audio::update */
		void _update() override;

		/** @copydoc Audio::setActiveDevice */
		void setActiveDevice(const AudioDevice& device) override;

		/** @copydoc Audio::getActiveDevice */
		AudioDevice getActiveDevice() const override { return mActiveDevice; }

		/** @copydoc Audio::getDefaultDevice */
		AudioDevice getDefaultDevice() const override { return mDefaultDevice; }

		/** @copydoc Audio::getAllDevices */
		const Vector<AudioDevice>& getAllDevices() const override { return mAllDevices; };
	private:
		/** @copydoc Audio::createClip */
		SPtr<AudioClip> createClip(const SPtr<DataStream>& samples, UINT32 streamSize, UINT32 numSamples,
			const AUDIO_CLIP_DESC& desc) override;

		/** @copydoc Audio::createListener */
		SPtr<AudioListener> createListener() override;

		/** @copydoc Audio::createSource */
		SPtr<AudioSource> createSource() override;

		FMOD::System* mFMOD;
		FMOD::ChannelGroup* mMasterChannelGroup;

		float mVolume;
		bool mIsPaused;

		Vector<AudioDevice> mAllDevices;
		AudioDevice mDefaultDevice;
		AudioDevice mActiveDevice;
	};

	/** Provides easier access to FMODAudio. */
	FMODAudio& gFMODAudio();

	/** @} */
}
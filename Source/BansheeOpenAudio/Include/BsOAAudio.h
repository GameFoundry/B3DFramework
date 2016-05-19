//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsOAPrerequisites.h"
#include "BsAudio.h"
#include "AL/alc.h"

namespace BansheeEngine
{
	/** @addtogroup OpenAudio
	 *  @{
	 */
	
	class BS_OA_EXPORT OAAudio : public Audio
	{
	public:
		OAAudio();
		virtual ~OAAudio();

		/** @copydoc Audio::setVolume */
		void setVolume(float volume) override;

		/** @copydoc Audio::getVolume */
		float getVolume() const override;

		/** @copydoc Audio::setPaused */
		void setPaused(bool paused) override;

		/** @copydoc Audio::isPaused */
		bool isPaused() const override;

		/** @copydoc Audio::update */
		void update() override;

		/** @copydoc Audio::setActiveDevice */
		void setActiveDevice(const AudioDevice& device) override;

		/** @copydoc Audio::getActiveDevice */
		AudioDevice getActiveDevice() const override { return mActiveDevice; }

		/** @copydoc Audio::getDefaultDevice */
		AudioDevice getDefaultDevice() const override { return mDefaultDevice; }

		/** @copydoc Audio::getAllDevices */
		const Vector<AudioDevice>& getAllDevices() const override { return mAllDevices; };

		/** @name Internal 
		 *  @{
		 */

		bool _isExtensionSupported(const String& extension) const;

		void _registerListener(OAAudioListener* listener);
		void _unregisterListener(OAAudioListener* listener);

		void _registerSource(OAAudioSource* source);
		void _unregisterSource(OAAudioSource* source);

		const Vector<ALCcontext*>& _getContexts() const { return mContexts; }

		/** @} */

	private:
		/** @copydoc Audio::createClip */
		SPtr<AudioClip> createClip(const SPtr<DataStream>& samples, UINT32 streamSize, UINT32 numSamples,
			const AUDIO_CLIP_DESC& desc) override;

		/** @copydoc Audio::createListener */
		SPtr<AudioListener> createListener() override;

		/** @copydoc Audio::createSource */
		SPtr<AudioSource> createSource() override;

		void rebuildContexts();
		void clearContexts();

		float mVolume;

		ALCdevice* mDevice;
		Vector<AudioDevice> mAllDevices;
		AudioDevice mDefaultDevice;
		AudioDevice mActiveDevice;

		Vector<OAAudioListener*> mListeners;
		Vector<ALCcontext*> mContexts;
		UnorderedSet<OAAudioSource*> mSources;

	};

	/** Provides easier access to OAAudio. */
	OAAudio& gOAAudio();

	/** @} */
}
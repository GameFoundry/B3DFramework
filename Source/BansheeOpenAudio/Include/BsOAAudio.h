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
		AudioDevice getActiveDevice() const override;

		/** @copydoc Audio::getDefaultDevice */
		AudioDevice getDefaultDevice() const override;

		/** @copydoc Audio::getAllDevices */
		Vector<AudioDevice> getAllDevices() const override;

		/** @name Internal 
		 *  @{
		 */

		bool isExtensionSupported(const String& extension) const;

		/** @} */

	private:
		ALCdevice* mDevice;
	};

	/** Provides easier access to OAAudio. */
	OAAudio& gOAAudio();

	/** @} */
}
//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Utility/BsModule.h"
#include "Math/BsVector3.h"

namespace bs
{
	/** @addtogroup Audio
	 *  @{
	 */

	/** Identifier for a device that can be used for playing audio. */
	struct BS_SCRIPT_EXPORT(m:Audio,pl:true) AudioDevice
	{
		String name;
	};

	/** Provides global functionality relating to sounds and music. */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Audio) Audio : public Module<Audio>
	{
	public:
		virtual ~Audio() = default;

		/** 
		 * Starts playback of the provided audio clip. This can be used for a quicker way of creating audio sources if you
		 * don't need the full control provided by creating AudioSource manually.
		 *
		 * @param[in]	clip		Audio clip to play.
		 * @param[in]	position	Position in world space to play the clip at. Only relevant if the clip is 3D.
		 * @param[in]	volume		Volume to play the clip at.
		 */
		void play(const HAudioClip& clip, const Vector3& position = Vector3::ZERO, float volume = 1.0f);

		/** Determines global audio volume. In range [0, 1]. */
		BS_SCRIPT_EXPORT(n:Volume,pr:setter)
		virtual void setVolume(float volume) = 0;

		/** @copydoc setVolume() */
		BS_SCRIPT_EXPORT(n:Volume,pr:getter)
		virtual float getVolume() const = 0;

		/** Determines if audio reproduction is paused globally. */
		BS_SCRIPT_EXPORT(n:Paused,pr:setter)
		virtual void setPaused(bool paused) = 0;

		/** @copydoc setPaused() */
		BS_SCRIPT_EXPORT(n:Paused,pr:getter)
		virtual bool isPaused() const = 0;

		/** Determines the device on which is the audio played back on. */
		BS_SCRIPT_EXPORT(n:ActiveDevice,pr:setter)
		virtual void setActiveDevice(const AudioDevice& device) = 0;

		/** @copydoc setActiveDevice() */
		BS_SCRIPT_EXPORT(n:ActiveDevice,pr:getter)
		virtual AudioDevice getActiveDevice() const = 0;
		
		/** Returns the default audio device identifier. */
		BS_SCRIPT_EXPORT(n:DefaultDevice,pr:getter)
		virtual AudioDevice getDefaultDevice() const = 0;

		/** Returns a list of all available audio devices. */
		BS_SCRIPT_EXPORT(n:AllDevices,pr:getter)
		virtual const Vector<AudioDevice>& getAllDevices() const = 0;

		/** @name Internal
		 *  @{
		 */

		/** Called once per frame. Queues streaming audio requests. */
		virtual void _update();

		/** @} */
	protected:
		friend class AudioClip;
		friend class AudioListener;
		friend class AudioSource;

		/** 
		 * Creates a new audio clip.
		 *
		 * @param[in]	samples		Stream containing audio samples in format specified in @p desc.
		 * @param[in]	streamSize	Size of the audio data in the provided stream, in bytes.
		 * @param[in]	numSamples	Number of samples in @p samples stream.
		 * @param[in]	desc		Descriptor describing the type of the audio stream (format, sample rate, etc.).
		 * @return					Newly created AudioClip. Must be manually initialized.
		 */
		virtual SPtr<AudioClip> createClip(const SPtr<DataStream>& samples, UINT32 streamSize, UINT32 numSamples,
			const AUDIO_CLIP_DESC& desc) = 0;

		/** Creates a new AudioListener. */
		virtual SPtr<AudioListener> createListener() = 0;

		/** Creates a new AudioSource. */
		virtual SPtr<AudioSource> createSource() = 0;

		/** Stops playback of all sources started with Audio::play calls. */
		void stopManualSources();

	private:
		Vector<SPtr<AudioSource>> mManualSources;
		Vector<SPtr<AudioSource>> mTempSources;
	};

	/** Provides easier access to Audio. */
	BS_CORE_EXPORT Audio& gAudio();

	/** @} */
}
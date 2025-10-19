//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Utility/B3DModule.h"
#include "Math/B3DVector3.h"

namespace b3d
{
	class IAudioSourceImplementation;
	class IAudioListenerImplementation;

	/** @addtogroup Audio
	 *  @{
	 */

	/** Identifier for a device that can be used for playing audio. */
	struct B3D_SCRIPT_EXPORT(DocumentationGroup(Audio), ExportAsStruct(true)) AudioDevice
	{
		String Name;
	};

	/** Provides global functionality relating to sounds and music. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Audio)) Audio : public Module<Audio>
	{
	public:
		virtual ~Audio() = default;

		/**
		 * Starts playback of the provided audio clip. This can be used for a quicker way of creating audio sources if you
		 * don't need the full control provided by creating AudioSource manually.
		 *
		 * @param	clip		Audio clip to play.
		 * @param	position	Position in world space to play the clip at. Only relevant if the clip is 3D.
		 * @param	volume		Volume to play the clip at.
		 */
		void Play(const HAudioClip& clip, const Vector3& position = Vector3::kZero, float volume = 1.0f);

		/** Determines global audio volume. In range [0, 1]. */
		B3D_SCRIPT_EXPORT(ExportName(Volume), Property(Setter))
		virtual void SetVolume(float volume) = 0;

		/** @copydoc SetVolume() */
		B3D_SCRIPT_EXPORT(ExportName(Volume), Property(Getter))
		virtual float GetVolume() const = 0;

		/** Determines if audio reproduction is paused globally. */
		B3D_SCRIPT_EXPORT(ExportName(Paused), Property(Setter))
		virtual void SetPaused(bool paused) = 0;

		/** @copydoc SetPaused() */
		B3D_SCRIPT_EXPORT(ExportName(Paused), Property(Getter))
		virtual bool IsPaused() const = 0;

		/** Determines the device on which is the audio played back on. */
		B3D_SCRIPT_EXPORT(ExportName(ActiveDevice), Property(Setter))
		virtual void SetActiveDevice(const AudioDevice& device) = 0;

		/** @copydoc SetActiveDevice() */
		B3D_SCRIPT_EXPORT(ExportName(ActiveDevice), Property(Getter))
		virtual AudioDevice GetActiveDevice() const = 0;

		/** Returns the default audio device identifier. */
		B3D_SCRIPT_EXPORT(ExportName(DefaultDevice), Property(Getter))
		virtual AudioDevice GetDefaultDevice() const = 0;

		/** Returns a list of all available audio devices. */
		B3D_SCRIPT_EXPORT(ExportName(AllDevices), Property(Getter))
		virtual const Vector<AudioDevice>& GetAllDevices() const = 0;

		/** @name Internal
		 *  @{
		 */

		/** Called once per frame. Queues streaming audio requests. */
		virtual void Update();

		/** @} */
	protected:
		friend class AudioClip;
		friend class AudioListener;
		friend class AudioSource;

		/**
		 * Creates a new audio clip.
		 *
		 * @param	samples				Stream containing audio samples in format specified in @p createInformation.
		 * @param	streamSize			Size of the audio data in the provided stream, in bytes.
		 * @param	sampleCount			Number of samples in @p samples stream.
		 * @param	createInformation	Descriptor describing the type of the audio stream (format, sample rate, etc.).
		 * @return						Newly created AudioClip. Must be manually initialized.
		 */
		virtual SPtr<AudioClip> CreateClip(const SPtr<DataStream>& samples, u32 streamSize, u32 sampleCount, const AudioClipCreateInformation& createInformation) = 0;

		/** Creates a new AudioListener. */
		virtual SPtr<IAudioListenerImplementation> CreateListener() = 0;

		/** Creates a new AudioSource. */
		virtual SPtr<IAudioSourceImplementation> CreateSource() = 0;

		/** Stops playback of all sources started with Audio::Play calls. */
		void StopManualSources();

	private:
		Vector<SPtr<IAudioSourceImplementation>> mManualSources;
		Vector<SPtr<IAudioSourceImplementation>> mTempSources;
	};

	/** Provides easier access to Audio. */
	B3D_EXPORT Audio& GetAudio();

	/** @} */
} // namespace b3d

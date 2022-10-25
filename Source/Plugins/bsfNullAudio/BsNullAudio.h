//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsNullAudioPrerequisites.h"
#include "Audio/BsAudio.h"
#include "Audio/BsAudioClip.h"
#include "Audio/BsAudioListener.h"
#include "Audio/BsAudioSource.h"

namespace bs
{
	/** @addtogroup NullAudio
	 *  @{
	 */

	/** Global manager for the null audio implementation. */
	class NullAudio final : public Audio
	{
	public:
		NullAudio();

		/** @copydoc Audio::setVolume */
		void SetVolume(float volume) { mVolume = volume; }

		/** @copydoc Audio::getVolume */
		float GetVolume() const { return mVolume; }

		/** @copydoc Audio::setPaused */
		void SetPaused(bool paused) { mIsPaused = paused; }

		/** @copydoc Audio::isPaused */
		bool IsPaused() const { return mIsPaused; }

		/** @copydoc Audio::setActiveDevice */
		void SetActiveDevice(const AudioDevice& device) { mActiveDevice = device; }

		/** @copydoc Audio::getActiveDevice */
		AudioDevice GetActiveDevice() const { return mActiveDevice; }

		/** @copydoc Audio::getDefaultDevice */
		AudioDevice GetDefaultDevice() const { return mDefaultDevice; }

		/** @copydoc Audio::getAllDevices */
		const Vector<AudioDevice>& GetAllDevices() const { return mAllDevices; };

	private:
		friend class NullAudioSource;

		/** @copydoc Audio::createClip */
		SPtr<AudioClip> CreateClip(const SPtr<DataStream>& samples, u32 streamSize, u32 numSamples, const AUDIO_CLIP_DESC& desc);

		/** @copydoc Audio::createListener */
		SPtr<AudioListener> CreateListener() override;

		/** @copydoc Audio::createSource */
		SPtr<AudioSource> CreateSource();

		float mVolume = 1.0f;
		bool mIsPaused = false;

		Vector<AudioDevice> mAllDevices;
		AudioDevice mDefaultDevice;
		AudioDevice mActiveDevice;
	};

	/** Null implementation of an AudioClip. */
	class NullAudioClip final : public AudioClip
	{
	public:
		NullAudioClip(const SPtr<DataStream>& samples, u32 streamSize, u32 numSamples, const AUDIO_CLIP_DESC& desc);

	protected:
		/** @copydoc Resource::initialize */
		void Initialize() override;

		/** @copydoc AudioClip::getSourceStream */
		SPtr<DataStream> GetSourceStream(u32& size);

	private:
		// These streams exist to save original audio data in case it's needed later (usually for saving with the editor, or
		// manual data manipulation). In normal usage (in-game) these will be null so no memory is wasted.
		SPtr<DataStream> mSourceStreamData;
		u32 mSourceStreamSize;
	};

	/** Null implementation of an AudioListener. */
	class NullAudioListener final : public AudioListener
	{
	private:
		friend class NullAudio;
	};

	/** Null implementation of an AudioSource. */
	class NullAudioSource final : public AudioSource
	{
	public:
		/** @copydoc AudioSource::setTime */
		void SetTime(float time) override { mTime = time; }

		/** @copydoc AudioSource::getTime */
		float GetTime() const override { return mTime; }

		/** @copydoc AudioSource::play */
		void Play() override { mState = AudioSourceState::Playing; }

		/** @copydoc AudioSource::pause */
		void Pause() override { mState = AudioSourceState::Paused; }

		/** @copydoc AudioSource::stop */
		void Stop() override { mState = AudioSourceState::Stopped; }

		/** @copydoc AudioSource::getState */
		AudioSourceState GetState() const override { return mState; }

	private:
		friend class NullAudio;

		float mTime = 0.0f;
		AudioSourceState mState = AudioSourceState::Stopped;
		bool mGloballyPaused = false;
	};

	/** Provides easier access to the null audio manager. */
	NullAudio& gNullAudio();

	/** @} */
} // namespace bs

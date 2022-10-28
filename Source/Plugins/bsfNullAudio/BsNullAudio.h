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

		void SetVolume(float volume) override { mVolume = volume; }
		float GetVolume() const override { return mVolume; }
		void SetPaused(bool paused) override { mIsPaused = paused; }
		bool IsPaused() const override { return mIsPaused; }
		void SetActiveDevice(const AudioDevice& device) override { mActiveDevice = device; }
		AudioDevice GetActiveDevice() const override { return mActiveDevice; }
		AudioDevice GetDefaultDevice() const override { return mDefaultDevice; }
		const Vector<AudioDevice>& GetAllDevices() const override { return mAllDevices; };

	private:
		friend class NullAudioSource;

		SPtr<AudioClip> CreateClip(const SPtr<DataStream>& samples, u32 streamSize, u32 numSamples, const AUDIO_CLIP_DESC& desc) override;
		SPtr<AudioListener> CreateListener() override;
		SPtr<AudioSource> CreateSource() override;

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
		void Initialize() override;
		SPtr<DataStream> GetSourceStream(u32& size) override;

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
		void SetTime(float time) override { mTime = time; }
		float GetTime() const override { return mTime; }
		void Play() override { mState = AudioSourceState::Playing; }
		void Pause() override { mState = AudioSourceState::Paused; }
		void Stop() override { mState = AudioSourceState::Stopped; }
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

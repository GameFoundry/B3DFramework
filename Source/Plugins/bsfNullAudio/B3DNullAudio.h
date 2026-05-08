//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DNullAudioPrerequisites.h"
#include "Audio/B3DAudio.h"
#include "Audio/B3DAudioClip.h"
#include "Components/B3DAudioListener.h"
#include "Components/B3DAudioSource.h"

namespace b3d
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

		TShared<AudioClip> CreateClip(const TShared<DataStream>& samples, u32 streamSize, u32 sampleCount, const AudioClipCreateInformation& createInformation) override;
		TShared<IAudioListenerImplementation> CreateListener() override;
		TShared<IAudioSourceImplementation> CreateSource() override;

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
		NullAudioClip(const TShared<DataStream>& samples, u32 streamSize, u32 sampleCount, const AudioClipCreateInformation& createInformation);

	protected:
		void Initialize() override;
		TShared<DataStream> GetSourceStream(u32& outSize) override;

	private:
		// These streams exist to save original audio data in case it's needed later (usually for saving with the editor, or
		// manual data manipulation). In normal usage (in-game) these will be null so no memory is wasted.
		TShared<DataStream> mSourceStreamData;
		u32 mSourceStreamSize;
	};

	/** Null implementation of an AudioListener. */
	class NullAudioListener final : public IAudioListenerImplementation
	{
	public:
		void SetTransform(const Transform& transform) override {}
		void SetVelocity(const Vector3& velocity) override {}

	private:
		friend class NullAudio;
	};

	/** Null implementation of an AudioSource. */
	class NullAudioSource final : public IAudioSourceImplementation
	{
	public:
		void SetClip(const HAudioClip& clip) override {}
		void SetVelocity(const Vector3& velocity) override {}
		void SetTransform(const Transform& transform) override {}
		void SetVolume(float volume) override {}
		void SetPitch(float pitch) override {}
		void SetIsLooping(bool loop) override {}
		void SetPriority(i32 priority) override {}
		void SetMinDistance(float distance) override {}
		void SetAttenuation(float attenuation) override {}
		void Play() override { mState = AudioSourceState::Playing; }
		void Pause() override { mState = AudioSourceState::Paused; }
		void Stop() override { mState = AudioSourceState::Stopped; }
		void SetTime(float time) override { mTime = time; }
		float GetTime() const override { return mTime; }
		AudioSourceState GetState() const override { return mState; }

	private:
		friend class NullAudio;

		float mTime = 0.0f;
		AudioSourceState mState = AudioSourceState::Stopped;
	};

	/** Provides easier access to the null audio manager. */
	NullAudio& GetNullAudio();

	/** @} */
} // namespace b3d

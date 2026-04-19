//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DOAPrerequisites.h"
#include "Components/B3DAudioSource.h"

namespace b3d
{
	class OAAudioClip;
}

namespace b3d
{
	/** @addtogroup OpenAudio
	 *  @{
	 */

	/** OpenAL implementation of an AudioSource. */
	class OAAudioSource : public IAudioSourceImplementation
	{
	public:
		OAAudioSource();
		virtual ~OAAudioSource();

		void SetTransform(const Transform& transform) override;
		void SetClip(const HAudioClip& clip) override;
		void SetVelocity(const Vector3& velocity) override;
		void SetVolume(float volume) override;
		void SetPitch(float pitch) override;
		void SetIsLooping(bool loop) override;
		void SetPriority(i32 priority) override;
		void SetMinDistance(float distance) override;
		void SetAttenuation(float attenuation) override;
		void SetTime(float time) override;
		float GetTime() const override;
		void Play() override;
		void Pause() override;
		void Stop() override;
		AudioSourceState GetState() const override;

	private:
		friend class OAAudio;

		/** Destroys the internal representation of the audio source. */
		void Clear();

		/** Rebuilds the internal representation of an audio source. */
		void Rebuild();

		/** Streams new data into the source audio buffer, if needed. */
		void Stream();

		/** Same as stream(), but without a mutex lock (up to the caller to lock it). */
		void StreamUnlocked();

		/** Starts data streaming from the currently attached audio clip. */
		void StartStreaming();

		/** Stops streaming data from the currently attached audio clip. */
		void StopStreaming();

		/** Pauses or resumes audio playback due to the global pause setting. */
		void SetGlobalPause(bool pause);

		/**
		 * Returns true if the sound source is three dimensional (volume and pitch varies based on listener distance
		 * and velocity).
		 */
		bool Is3D() const;

		/**
		 * Returns true if the audio source is receiving audio data from a separate thread (as opposed to loading it all
		 * at once.
		 */
		bool RequiresStreaming() const;

		/** Fills the provided buffer with streaming data. */
		bool FillBuffer(u32 buffer, AudioDataInfo& info, u32 maxSampleCount);

		/** Makes the current audio clip active. Should be called whenever the audio clip changes. */
		void ApplyClip();

		TResourceHandle<OAAudioClip> mAudioClip;
		Vector3 mPosition = kZeroTag;
		Vector3 mVelocity = kZeroTag;
		float mVolume = 1.0f;
		float mPitch = 1.0f;
		bool mLoop = false;
		float mMinDistance = 1.0f;
		float mAttenuation = 1.0f;

		Vector<u32> mSourceIDs;
		float mSavedTime = 0.0f;
		AudioSourceState mSavedState = AudioSourceState::Stopped;
		bool mGloballyPaused = false;

		static const u32 kStreamBufferCount = 3; // Maximum 32
		u32 mStreamBuffers[kStreamBufferCount];
		u32 mBusyBuffers[kStreamBufferCount];
		u32 mStreamProcessedPosition = 0;
		u32 mStreamQueuedPosition = 0;
		bool mIsStreaming = false;
		mutable Mutex mMutex;
	};

	/** @} */
} // namespace b3d

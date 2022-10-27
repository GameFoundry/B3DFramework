//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsOAPrerequisites.h"
#include "Audio/BsAudioSource.h"

namespace bs
{
	/** @addtogroup OpenAudio
	 *  @{
	 */

	/** OpenAL implementation of an AudioSource. */
	class OAAudioSource : public AudioSource
	{
	public:
		OAAudioSource();
		virtual ~OAAudioSource();

		/** @copydoc SceneActor::setTransform */
		void SetTransform(const Transform& transform);

		/** @copydoc AudioSource::setClip */
		void SetClip(const HAudioClip& clip);

		/** @copydoc AudioSource::setVelocity */
		void SetVelocity(const Vector3& velocity);

		/** @copydoc AudioSource::setVolume */
		void SetVolume(float volume);

		/** @copydoc AudioSource::setPitch */
		void SetPitch(float pitch);

		/** @copydoc AudioSource::setIsLooping */
		void SetIsLooping(bool loop);

		/** @copydoc AudioSource::setPriority */
		void SetPriority(i32 priority);

		/** @copydoc AudioSource::setMinDistance */
		void SetMinDistance(float distance) override;

		/** @copydoc AudioSource::setAttenuation */
		void SetAttenuation(float attenuation) override;

		/** @copydoc AudioSource::setTime */
		void SetTime(float time) override;

		/** @copydoc AudioSource::getTime */
		float GetTime() const override;

		/** @copydoc AudioSource::play */
		void Play() override;

		/** @copydoc AudioSource::pause */
		void Pause() override;

		/** @copydoc AudioSource::stop */
		void Stop() override;

		/** @copydoc AudioSource::getState */
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
		bool FillBuffer(u32 buffer, AudioDataInfo& info, u32 maxNumSamples);

		/** Makes the current audio clip active. Should be called whenever the audio clip changes. */
		void ApplyClip();

		/** @copydoc AudioSource::onClipChanged */
		void OnClipChanged() override;

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
} // namespace bs

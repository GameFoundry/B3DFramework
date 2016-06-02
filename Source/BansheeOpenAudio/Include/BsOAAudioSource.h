//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsOAPrerequisites.h"
#include "BsAudioSource.h"

namespace BansheeEngine
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

		/** @copydoc AudioSource::setClip */
		void setClip(const HAudioClip& clip) override;

		/** @copydoc AudioSource::setPosition */
		void setPosition(const Vector3& position) override;

		/** @copydoc AudioSource::setVelocity */
		void setVelocity(const Vector3& velocity) override;

		/** @copydoc AudioSource::setVolume */
		void setVolume(float volume) override;

		/** @copydoc AudioSource::setPitch */
		void setPitch(float pitch) override;

		/** @copydoc AudioSource::setIsLooping */
		void setIsLooping(bool loop) override;

		/** @copydoc AudioSource::setPriority */
		void setPriority(INT32 priority) override;

		/** @copydoc AudioSource::setMinDistance */
		void setMinDistance(float distance) override;

		/** @copydoc AudioSource::setAttenuation */
		void setAttenuation(float attenuation) override;

		/** @copydoc AudioSource::setTime */
		void setTime(float setTime) override;

		/** @copydoc AudioSource::getTime */
		float getTime() const override;

		/** @copydoc AudioSource::play */
		void play() override;

		/** @copydoc AudioSource::pause */
		void pause() override;

		/** @copydoc AudioSource::stop */
		void stop() override;

		/** @copydoc AudioSource::getState */
		AudioSourceState getState() const override { return mState; }

	private:
		friend class OAAudio;

		/** Destroys the internal representation of the audio source. */
		void clear();

		/** Rebuilds the internal representation of an audio source. */
		void rebuild();

		/** Streams new data into the source audio buffer, if needed. */
		void stream();

		/** Starts data streaming from the currently attached audio clip. */
		void startStreaming();

		/** Stops streaming data from the currently attached audio clip. */
		void stopStreaming();

		/** Pauses or resumes audio playback due to the global pause setting. */
		void setGlobalPause(bool pause);

		/** 
		 * Returns true if the sound source is three dimensional (volume and pitch varies based on listener distance
		 * and velocity). 
		 */
		bool is3D() const;

		/** 
		 * Returns true if the audio source is receiving audio data from a separate thread (as opposed to loading it all
		 * at once. 
		 */
		bool requiresStreaming() const;

		/** Fills the provided buffer with streaming data. */
		bool fillBuffer(UINT32 buffer, AudioDataInfo& info, UINT32 maxNumSamples);

		/** Makes the current audio clip active. Should be called whenever the audio clip changes. */
		void applyClip();

		/** @copydoc IResourceListener::onClipChanged */
		void onClipChanged() override;

		Vector<UINT32> mSourceIDs;
		float mSavedTime;
		AudioSourceState mState;
		bool mGloballyPaused;

		static const UINT32 StreamBufferCount = 3;
		UINT32 mStreamBuffers[StreamBufferCount];
		bool mBusyBuffers[StreamBufferCount];
		UINT32 mStreamProcessedPosition;
		UINT32 mStreamQueuedPosition;
		bool mIsStreaming;
		mutable Mutex mMutex;
	};

	/** @} */
}
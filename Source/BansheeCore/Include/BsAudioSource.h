//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsIResourceListener.h"
#include "BsVector3.h"

namespace bs
{
	/** @addtogroup Audio
	 *  @{
	 */

	/** Valid states in which AudioSource can be in. */
	enum class AudioSourceState
	{
		Playing, /**< Source is currently playing. */
		Paused, /**< Source is currently paused (play will resume from paused point). */
		Stopped /**< Source is currently stopped (play will resume from start). */
	};

	/** 
	 * Represents a source for emitting audio. Audio can be played spatially (gun shot), or normally (music). Each audio
	 * source must have an AudioClip to play-back, and it can also have a position in the case of spatial (3D) audio. 
	 *
	 * Whether or not an audio source is spatial is controlled by the assigned AudioClip. The volume and the pitch of a
	 * spatial audio source is controlled by its position and the AudioListener's position/direction/velocity.
	 */
	class BS_CORE_EXPORT AudioSource : public IResourceListener
	{
	public:
		virtual ~AudioSource() { }

		/** Sets the audio clip to play. */
		virtual void setClip(const HAudioClip& clip);

		/** Returns the attached audio clip. */
		HAudioClip getClip() const { return mAudioClip; }

		/** Sets the world position of the source. Only relevant for spatial (3D) sources. */
		virtual void setPosition(const Vector3& position);

		/** Returns the world position of the source. Only relevant for spatial (3D) sources. */
		Vector3 getPosition() const { return mPosition; }

		/** Sets the velocity of the source. Only relevant for spatial (3D) sources. */
		virtual void setVelocity(const Vector3& velocity);

		/** Gets the velocity of the source. Only relevant for spatial (3D) sources. */
		Vector3 getVelocity() const { return mVelocity; }

		/** Sets the volume of the audio source, in [0, 1] range. */
		virtual void setVolume(float volume);

		/** Retrieves the volume of the audio source, in [0, 1] range. */
		float getVolume() const { return mVolume; }

		/** Sets the pitch of the audio source. */
		virtual void setPitch(float pitch);

		/** Gets the pitch of the audio source. */
		float getPitch() const { return mPitch; }

		/** Sets whether the audio clip should loop when it finishes playing. */
		virtual void setIsLooping(bool loop);

		/** Gets whether the audio clip should loop when it finishes playing. */
		bool getIsLooping() const { return mLoop; }

		/** 
		 * Sets the priority of the audio source. If more audio sources are playing than supported by the hardware, some
		 * might get disabled. By setting a higher priority the audio source is guaranteed to be disabled after sources
		 * with lower priority.
		 */
		virtual void setPriority(INT32 priority);

		/**
		 * Gets the priority of the audio source. 
		 *
		 * @see	setPriority
		 */
		UINT32 getPriority() const { return mPriority; }

		/** 
		 * Returns the minimum distance at which audio attenuation starts. When the listener is closer to the source
		 * than this value, audio is heard at full volume. Once farther away the audio starts attenuating.
		 */
		virtual void setMinDistance(float distance);

		/** 
		 * Gets the minimum distance at which audio attenuation starts. 
		 *
		 * @see	setMinDistance
		 */
		float getMinDistance() const { return mMinDistance; }

		/**
		 * Sets attenuation that controls how quickly does audio volume drop off as the listener moves further from the 
		 * source.
		 */
		virtual void setAttenuation(float attenuation);

		/** 
		 * Gets attenuation that controls how quickly does audio volume drop off as the listener moves further from the 
		 * source.
		 */
		float getAttenuation() const { return mAttenuation; }

		/** Starts playing the currently assigned audio clip. */
		virtual void play() = 0;

		/** Pauses the audio playback. */
		virtual void pause() = 0;

		/** Stops audio playback, rewinding it to the start. */
		virtual void stop() = 0;

		/**
		 * Sets the time at which playback will begin.
		 *
		 * @param[in]	time	Time in seconds, in range [0, clipLength].
		 */
		virtual void setTime(float time) = 0;

		/**
		 * Returns the current playback time.
		 *
		 * @return	time	Time in seconds, in range [0, clipLength].
		 */
		virtual float getTime() const = 0;

		/** Returns the current state of the audio playback (playing/paused/stopped). */
		virtual AudioSourceState getState() const = 0;

		/** Creates a new audio source. */
		static SPtr<AudioSource> create();

	protected:
		AudioSource();

		/** @copydoc IResourceListener::getListenerResources */
		void getListenerResources(Vector<HResource>& resources) override;

		/** @copydoc IResourceListener::notifyResourceChanged */
		void notifyResourceChanged(const HResource& resource) override;

		/** Triggered by the resources system whenever the attached audio clip changed (e.g. was reimported.) */
		virtual void onClipChanged() { }

		HAudioClip mAudioClip;
		Vector3 mPosition;
		Vector3 mVelocity;
		float mVolume;
		float mPitch;
		bool mLoop;
		INT32 mPriority;
		float mMinDistance;
		float mAttenuation;
	};

	/** @} */
}
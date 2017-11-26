//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "Resources/BsIResourceListener.h"
#include "Scene/BsSceneActor.h"
#include "Math/BsVector3.h"

namespace bs
{
	/** @addtogroup Audio
	 *  @{
	 */

	/** Valid states in which AudioSource can be in. */
	enum class BS_SCRIPT_EXPORT(m:Audio) AudioSourceState
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
	class BS_CORE_EXPORT AudioSource : public IReflectable, public SceneActor, public IResourceListener
	{
	public:
		virtual ~AudioSource() { }

		/** Aaudio clip to play. */
		virtual void setClip(const HAudioClip& clip);

		/** @copydoc setClip() */
		HAudioClip getClip() const { return mAudioClip; }

		/** 
		 * Velocity of the source. Determines pitch in relation to AudioListener's position. Only relevant for spatial 
		 * (3D) sources. 
		 */
		virtual void setVelocity(const Vector3& velocity);

		/** @copydoc setVelocity() */
		Vector3 getVelocity() const { return mVelocity; }

		/** Volume of the audio played from this source, in [0, 1] range. */
		virtual void setVolume(float volume);

		/** @copydoc setVolume() */
		float getVolume() const { return mVolume; }

		/** Determines the pitch of the played audio. 1 is the default. */
		virtual void setPitch(float pitch);

		/** @copydoc setPitch() */
		float getPitch() const { return mPitch; }

		/** Determines whether the audio clip should loop when it finishes playing. */
		virtual void setIsLooping(bool loop);

		/** @copydoc setIsLooping() */
		bool getIsLooping() const { return mLoop; }

		/** 
		 * Determines the priority of the audio source. If more audio sources are playing than supported by the hardware, 
		 * some might get disabled. By setting a higher priority the audio source is guaranteed to be disabled after sources
		 * with lower priority.
		 */
		virtual void setPriority(INT32 priority);

		/** @copydoc setPriority() */
		UINT32 getPriority() const { return mPriority; }

		/** 
		 * Minimum distance at which audio attenuation starts. When the listener is closer to the source
		 * than this value, audio is heard at full volume. Once farther away the audio starts attenuating.
		 */
		virtual void setMinDistance(float distance);

		/** @copydoc setMinDistance() */
		float getMinDistance() const { return mMinDistance; }

		/**
		 * Attenuation that controls how quickly does audio volume drop off as the listener moves further from the source.
		 */
		virtual void setAttenuation(float attenuation);

		/** @copydoc setAttenuation() */
		float getAttenuation() const { return mAttenuation; }

		/** Starts playing the currently assigned audio clip. */
		virtual void play() = 0;

		/** Pauses the audio playback. */
		virtual void pause() = 0;

		/** Stops audio playback, rewinding it to the start. */
		virtual void stop() = 0;

		/** 
		 * Determines the current time of playback. If playback hasn't yet started, it specifies the time at which playback
		 * will start at. The time is in seconds, in range [0, clipLength]. 
		 */
		virtual void setTime(float time) = 0;

		/** @copydoc setTime() */
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
		Vector3 mVelocity;
		float mVolume;
		float mPitch;
		bool mLoop;
		INT32 mPriority;
		float mMinDistance;
		float mAttenuation;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class AudioSourceRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** @} */
}
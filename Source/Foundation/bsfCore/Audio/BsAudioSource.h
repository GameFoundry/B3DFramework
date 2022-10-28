//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
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
	enum class BS_SCRIPT_EXPORT(DocumentationGroup(Audio)) AudioSourceState
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
		virtual ~AudioSource() = default;

		/** Audio clip to play. */
		virtual void SetClip(const HAudioClip& clip);

		/** @copydoc SetClip() */
		HAudioClip GetClip() const { return mAudioClip; }

		/**
		 * Velocity of the source. Determines pitch in relation to AudioListener's position. Only relevant for spatial
		 * (3D) sources.
		 */
		virtual void SetVelocity(const Vector3& velocity);

		/** @copydoc SetVelocity() */
		Vector3 GetVelocity() const { return mVelocity; }

		/** Volume of the audio played from this source, in [0, 1] range. */
		virtual void SetVolume(float volume);

		/** @copydoc SetVolume() */
		float GetVolume() const { return mVolume; }

		/** Determines the pitch of the played audio. 1 is the default. */
		virtual void SetPitch(float pitch);

		/** @copydoc SetPitch() */
		float GetPitch() const { return mPitch; }

		/** Determines whether the audio clip should loop when it finishes playing. */
		virtual void SetIsLooping(bool loop);

		/** @copydoc SetIsLooping() */
		bool GetIsLooping() const { return mLoop; }

		/**
		 * Determines the priority of the audio source. If more audio sources are playing than supported by the hardware,
		 * some might get disabled. By setting a higher priority the audio source is guaranteed to be disabled after sources
		 * with lower priority.
		 */
		virtual void SetPriority(i32 priority);

		/** @copydoc SetPriority() */
		u32 GetPriority() const { return mPriority; }

		/**
		 * Minimum distance at which audio attenuation starts. When the listener is closer to the source
		 * than this value, audio is heard at full volume. Once farther away the audio starts attenuating.
		 */
		virtual void SetMinDistance(float distance);

		/** @copydoc SetMinDistance() */
		float GetMinDistance() const { return mMinDistance; }

		/**
		 * Attenuation that controls how quickly does audio volume drop off as the listener moves further from the source.
		 */
		virtual void SetAttenuation(float attenuation);

		/** @copydoc SetAttenuation() */
		float GetAttenuation() const { return mAttenuation; }

		/** Starts playing the currently assigned audio clip. */
		virtual void Play() = 0;

		/** Pauses the audio playback. */
		virtual void Pause() = 0;

		/** Stops audio playback, rewinding it to the start. */
		virtual void Stop() = 0;

		/**
		 * Determines the current time of playback. If playback hasn't yet started, it specifies the time at which playback
		 * will start at. The time is in seconds, in range [0, clipLength].
		 */
		virtual void SetTime(float time) = 0;

		/** @copydoc SetTime() */
		virtual float GetTime() const = 0;

		/** Returns the current state of the audio playback (playing/paused/stopped). */
		virtual AudioSourceState GetState() const = 0;

		/** Creates a new audio source. */
		static SPtr<AudioSource> Create();

	protected:
		AudioSource() = default;

		void GetListenerResources(Vector<HResource>& resources) override;
		void NotifyResourceChanged(const HResource& resource) override;

		/** Triggered by the resources system whenever the attached audio clip changed (e.g. was reimported.) */
		virtual void OnClipChanged() {}

		HAudioClip mAudioClip;
		Vector3 mVelocity = BsZero;
		float mVolume = 1.0f;
		float mPitch = 1.0f;
		bool mLoop = false;
		i32 mPriority = 0;
		float mMinDistance = 1.0f;
		float mAttenuation = 1.0f;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class AudioSourceRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	/** @} */
} // namespace bs

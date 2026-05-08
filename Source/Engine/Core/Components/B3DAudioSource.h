//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Resources/B3DIResourceListener.h"
#include "Scene/B3DComponent.h"

namespace b3d
{
	class IAudioSourceImplementation;

	/** @addtogroup Audio
	 *  @{
	 */

	/** Valid states in which AudioSource can be in. */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Audio)) AudioSourceState
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
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Audio)) AudioSource : public Component, public IResourceListener
	{
	public:
		AudioSource(const HSceneObject& parent);
		virtual ~AudioSource() = default;

		/** Audio clip to play. */
		B3D_SCRIPT_EXPORT(ExportName(Clip), Property(Setter))
		void SetClip(const HAudioClip& clip);

		/** @copydoc SetClip */
		B3D_SCRIPT_EXPORT(ExportName(Clip), Property(Getter))
		HAudioClip GetClip() const { return mAudioClip; }

		/** Volume of the audio played from this source, in [0, 1] range. */
		B3D_SCRIPT_EXPORT(ExportName(Volume), Property(Setter), UIValueRange([ 0, 1 ]), UI(AsSlider))
		void SetVolume(float volume);

		/** @copydoc SetVolume */
		B3D_SCRIPT_EXPORT(ExportName(Volume), Property(Getter))
		float GetVolume() const { return mVolume; }

		/** Determines the pitch of the played audio. 1 is the default. */
		B3D_SCRIPT_EXPORT(ExportName(Pitch), Property(Setter))
		void SetPitch(float pitch);

		/** @copydoc SetPitch */
		B3D_SCRIPT_EXPORT(ExportName(Pitch), Property(Getter))
		float GetPitch() const { return mPitch; }

		/** Determines whether the audio clip should loop when it finishes playing. */
		B3D_SCRIPT_EXPORT(ExportName(Loop), Property(Setter))
		void SetIsLooping(bool loop);

		/** @copydoc SetIsLooping */
		B3D_SCRIPT_EXPORT(ExportName(Loop), Property(Getter))
		bool GetIsLooping() const { return mLoop; }

		/**
		 * Determines the priority of the audio source. If more audio sources are playing than supported by the hardware,
		 * some might get disabled. By setting a higher priority the audio source is guaranteed to be disabled after sources
		 * with lower priority.
		 */
		B3D_SCRIPT_EXPORT(ExportName(Priority), Property(Setter))
		void SetPriority(u32 priority);

		/** @copydoc SetPriority */
		B3D_SCRIPT_EXPORT(ExportName(Priority), Property(Getter))
		u32 GetPriority() const { return mPriority; }

		/**
		 * Minimum distance at which audio attenuation starts. When the listener is closer to the source
		 * than this value, audio is heard at full volume. Once farther away the audio starts attenuating.
		 */
		B3D_SCRIPT_EXPORT(ExportName(MinDistance), Property(Setter))
		void SetMinDistance(float distance);

		/** @copydoc SetMinDistance */
		B3D_SCRIPT_EXPORT(ExportName(MinDistance), Property(Getter))
		float GetMinDistance() const { return mMinDistance; }

		/** Attenuation that controls how quickly does audio volume drop off as the listener moves further from the source. */
		B3D_SCRIPT_EXPORT(ExportName(Attenuation), Property(Setter))
		void SetAttenuation(float attenuation);

		/** @copydoc SetAttenuation */
		B3D_SCRIPT_EXPORT(ExportName(Attenuation), Property(Getter))
		float GetAttenuation() const { return mAttenuation; }

		/**
		 * Determines the current time of playback. If playback hasn't yet started, it specifies the time at which playback
		 * will start at. The time is in seconds, in range [0, clipLength].
		 */
		B3D_SCRIPT_EXPORT(ExportName(Time), Property(Setter), UI(Hide))
		void SetTime(float time);

		/** @copydoc SetTime */
		B3D_SCRIPT_EXPORT(ExportName(Time), Property(Getter), UI(Hide))
		float GetTime() const;

		/** Sets whether playback should start as soon as the component is enabled. */
		B3D_SCRIPT_EXPORT(ExportName(PlayOnStart), Property(Setter))
		void SetPlayOnStart(bool enable) { mPlayOnStart = enable; }

		/** Determines should playback start as soon as the component is enabled. */
		B3D_SCRIPT_EXPORT(ExportName(PlayOnStart), Property(Getter))

		bool GetPlayOnStart() const { return mPlayOnStart; }

		/** Starts playing the currently assigned audio clip. */
		B3D_SCRIPT_EXPORT(ExportName(Play))
		void Play();

		/** Pauses the audio playback. */
		B3D_SCRIPT_EXPORT(ExportName(Pause))
		void Pause();

		/** Stops audio playback, rewinding it to the start. */
		B3D_SCRIPT_EXPORT(ExportName(Stop))
		void Stop();

		/** Returns the current state of the audio playback (playing/paused/stopped). */
		B3D_SCRIPT_EXPORT(ExportName(State), Property(Getter))
		AudioSourceState GetState() const;

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		void OnDestroyed() override;
		void OnDisabled() override;
		void OnEnabled() override;
		void OnTransformChanged(TransformChangedFlags flags) override;
		void Update() override;

	protected:
		void GetListenerResources(Vector<HResource>& resources) override;
		void NotifyResourceChanged(const HResource& resource) override;

		/** Creates the internal representation of the AudioSource and restores the values saved by the Component. */
		void RestoreInternal();

		/** Destroys the internal AudioSource representation. */
		void DestroyInternal();

		/**
		 * Updates the transform of the internal AudioSource representation from the transform of the component's scene
		 * object.
		 */
		void UpdateTransform();

		TShared<IAudioSourceImplementation> mImplementation;
		Vector3 mLastPosition = Vector3::kZero;
		Vector3 mVelocity = Vector3::kZero;

		HAudioClip mAudioClip;
		float mVolume = 1.0f;
		float mPitch = 1.0f;
		bool mLoop = false;
		u32 mPriority = 0;
		float mMinDistance = 1.0f;
		float mAttenuation = 1.0f;
		bool mPlayOnStart = true;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class AudioSourceRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const;

	protected:
		AudioSource(); // Serialization only
	};

	/** @} */

	/** @addtogroup Audio-Internal
	 *  @{
	 */

	/** Low-level interface for an audio source. Should be implemented by the audio plugin. */
	class B3D_EXPORT IAudioSourceImplementation
	{
	public:
		virtual ~IAudioSourceImplementation() = default;

		/** @copydoc AudioSource::SetClip */
		virtual void SetClip(const HAudioClip& clip) = 0;

		/**
		 * Velocity of the source. Determines pitch in relation to AudioListener's position. Only relevant for spatial
		 * (3D) sources.
		 */
		virtual void SetVelocity(const Vector3& velocity) = 0;

		/** Sets the position and orientation of the audio source. */
		virtual void SetTransform(const Transform& transform) = 0;

		/** @copydoc AudioSource::SetVolume */
		virtual void SetVolume(float volume) = 0;

		/** @copydoc AudioSource::SetPitch */
		virtual void SetPitch(float pitch) = 0;

		/** @copydoc AudioSource::SetIsLooping */
		virtual void SetIsLooping(bool loop) = 0;

		/** @copydoc AudioSource::SetPriority */
		virtual void SetPriority(i32 priority) = 0;

		/** @copydoc AudioSource::SetMinDistance */
		virtual void SetMinDistance(float distance) = 0;

		/** @copydoc AudioSource::SetAttenuation */
		virtual void SetAttenuation(float attenuation) = 0;

		/** @copydoc AudioSource::Play */
		virtual void Play() = 0;

		/** @copydoc AudioSource::Pause */
		virtual void Pause() = 0;

		/** @copydoc AudioSource::Stop */
		virtual void Stop() = 0;

		/** @copydoc AudioSource::SetTime */
		virtual void SetTime(float time) = 0;

		/** @copydoc SetTime */
		virtual float GetTime() const = 0;

		/** @copydoc AudioSource::GetState */
		virtual AudioSourceState GetState() const = 0;
	};

	/** @} */
} // namespace b3d

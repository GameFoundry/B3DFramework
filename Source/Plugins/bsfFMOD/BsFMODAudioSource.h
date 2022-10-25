//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsFMODPrerequisites.h"
#include "Audio/BsAudioSource.h"
#include "BsFMODAudio.h"

namespace bs
{
	/** @addtogroup FMOD
	 *  @{
	 */

	/** FMOD implementation of an AudioSource. */
	class FMODAudioSource : public AudioSource
	{
	public:
		FMODAudioSource();
		virtual ~FMODAudioSource();

		/** @copydoc SceneActor::setTransform */
		void SetTransform(const Transform& transform) override;

		/** @copydoc AudioSource::setClip */
		void SetClip(const HAudioClip& clip) override;

		/** @copydoc AudioSource::setVelocity */
		void SetVelocity(const Vector3& velocity) override;

		/** @copydoc AudioSource::setVolume */
		void SetVolume(float volume) override;

		/** @copydoc AudioSource::setPitch */
		void SetPitch(float pitch) override;

		/** @copydoc AudioSource::setIsLooping */
		void SetIsLooping(bool loop) override;

		/** @copydoc AudioSource::setPriority */
		void SetPriority(i32 priority) override;

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
		friend class FMODAudio;

		/** Pauses or resumes audio playback due to the global pause setting. */
		void SetGlobalPause(bool pause);

		/** @copydoc AudioSource::onClipChanged */
		void OnClipChanged() override;

		FMOD::Channel* mChannel = nullptr;
		FMOD::Sound* mStreamingSound = nullptr;

		float mTime = 0.0f;
		bool mGloballyPaused = false;
		AudioSourceState mGlobalUnpauseState = AudioSourceState::Stopped;
	};

	/** @} */
} // namespace bs

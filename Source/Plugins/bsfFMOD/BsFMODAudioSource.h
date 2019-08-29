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
		void setTransform(const Transform& transform) override;

		/** @copydoc AudioSource::setClip */
		void setClip(const HAudioClip& clip) override;

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

		/** @copydoc AudioSource::setTime */
		void setTime(float time) override;

		/** @copydoc AudioSource::getTime */
		float getTime() const override;

		/** @copydoc AudioSource::play */
		void play() override;

		/** @copydoc AudioSource::pause */
		void pause() override;

		/** @copydoc AudioSource::stop */
		void stop() override;

		/** @copydoc AudioSource::getState */
		AudioSourceState getState() const override;

	private:
		friend class FMODAudio;

		/** Pauses or resumes audio playback due to the global pause setting. */
		void setGlobalPause(bool pause);

		/** @copydoc AudioSource::onClipChanged */
		void onClipChanged() override;

		FMOD::Channel* mChannel = nullptr;
		FMOD::Sound* mStreamingSound = nullptr;

		float mTime = 0.0f;
		bool mGloballyPaused = false;
		AudioSourceState mGlobalUnpauseState = AudioSourceState::Stopped;
	};

	/** @} */
}

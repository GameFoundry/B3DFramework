//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsFMODPrerequisites.h"
#include "BsAudioSource.h"
#include "BsFMODAudio.h"

namespace BansheeEngine
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

		/** @copydoc IResourceListener::onClipChanged */
		void onClipChanged() override;

		FMOD::Channel* mChannel;
		FMOD::Sound* mStreamingSound;

		float mTime;
		bool mGloballyPaused;
		AudioSourceState mGlobalUnpauseState;
	};

	/** @} */
}
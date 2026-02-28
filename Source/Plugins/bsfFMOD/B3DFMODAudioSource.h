//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DFMODPrerequisites.h"
#include "B3DFMODAudio.h"
#include "Components/B3DAudioSource.h"

namespace b3d
{
	/** @addtogroup FMOD
	 *  @{
	 */

	/** FMOD implementation of an AudioSource. */
	class FMODAudioSource : public IAudioSourceImplementation
	{
	public:
		FMODAudioSource();
		~FMODAudioSource() override;

		void SetTransform(const Transform& transform) override;
		void SetClip(const HAudioClip& clip) override;
		void SetVelocity(const Vector3& velocity) override;
		void SetVolume(float volume) override;
		void SetPitch(float pitch) override;
		void SetIsLooping(bool loop) override;
		void SetPriority(i32 priority) override;
		void SetTime(float time) override;
		float GetTime() const override;
		void Play() override;
		void Pause() override;
		void Stop() override;
		AudioSourceState GetState() const override;

	private:
		friend class FMODAudio;

		/** Pauses or resumes audio playback due to the global pause setting. */
		void SetGlobalPause(bool pause);

		TResourceHandle<FMODAudioClip> mAudioClip;
		Vector3 mPosition = kZeroTag;
		Vector3 mVelocity = kZeroTag;
		float mVolume = 1.0f;
		float mPitch = 1.0f;
		bool mLoop = false;
		u32 mPriority = 0;

		FMOD::Channel* mChannel = nullptr;
		FMOD::Sound* mStreamingSound = nullptr;

		float mTime = 0.0f;
		bool mGloballyPaused = false;
		AudioSourceState mGlobalUnpauseState = AudioSourceState::Stopped;
	};

	/** @} */
} // namespace b3d

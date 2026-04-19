//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DOAPrerequisites.h"
#include "Audio/B3DAudio.h"
#include "AL/alc.h"
#include "Threading/B3DSignalEvent.h"

namespace b3d
{
	class IAudioSourceImplementation;
	class SignalEvent;

	/** @addtogroup OpenAudio
	 *  @{
	 */

	/** Global manager for the audio implementation using OpenAL as the backend. */
	class OAAudio : public Audio
	{
	public:
		OAAudio();
		virtual ~OAAudio();

		void SetVolume(float volume) override;
		float GetVolume() const override;
		void SetPaused(bool paused) override;
		bool IsPaused() const override { return mIsPaused; }
		void Update() override;
		void SetActiveDevice(const AudioDevice& device) override;
		AudioDevice GetActiveDevice() const override { return mActiveDevice; }
		AudioDevice GetDefaultDevice() const override { return mDefaultDevice; }
		const Vector<AudioDevice>& GetAllDevices() const override { return mAllDevices; };

		/** @name Internal
		 *  @{
		 */

		/** Checks is a specific OpenAL extension supported. */
		bool IsExtensionSupported(const String& extension) const;

		/** Registers a new AudioListener. Should be called on listener creation. */
		void RegisterListener(OAAudioListener* listener);

		/** Unregisters an existing AudioListener. Should be called before listener destruction. */
		void UnregisterListener(OAAudioListener* listener);

		/** Registers a new AudioSource. Should be called on source creation. */
		void RegisterSource(OAAudioSource* source);

		/** Unregisters an existing AudioSource. Should be called before source destruction. */
		void UnregisterSource(OAAudioSource* source);

		/** Returns a list of all OpenAL contexts. Each listener has its own context. */
		const Vector<ALCcontext*>& GetContexts() const { return mContexts; }

		/** Returns an OpenAL context assigned to the provided listener. */
		ALCcontext* GetContext(const OAAudioListener* listener) const;

		/**
		 * Returns optimal format for the provided number of channels and bit depth. It is assumed the user has checked if
		 * extensions providing these formats are actually available.
		 */
		i32 GetOpenALBufferFormat(u32 numChannels, u32 bitDepth);

		/**
		 * Writes provided samples into the OpenAL buffer with the provided ID. If the provided format is not supported the
		 * samples will first be converted into a valid format.
		 */
		void WriteToOpenALBuffer(u32 bufferId, u8* samples, const AudioDataInfo& info);

		/** @} */

	private:
		friend class OAAudioSource;

		/** Type of a command that can be queued for a streaming audio source. */
		enum class StreamingCommandType
		{
			Start,
			Stop
		};

		/** Command queued for a streaming audio source. */
		struct StreamingCommand
		{
			StreamingCommandType Type;
			OAAudioSource* Source;
		};

		SPtr<AudioClip> CreateClip(const SPtr<DataStream>& samples, u32 streamSize, u32 numSamples, const AudioClipCreateInformation& desc) override;
		SPtr<IAudioListenerImplementation> CreateListener() override;
		SPtr<IAudioSourceImplementation> CreateSource() override;

		/**
		 * Delete all existing contexts and rebuild them according to the listener list. All audio sources will be rebuilt
		 * as well.
		 *
		 * This should be called when listener count changes, or audio device is changed.
		 */
		void RebuildContexts();

		/** Delete all existing OpenAL contexts. */
		void ClearContexts();

		/** Streams new data to audio sources that require it. */
		void UpdateStreaming();

		/** Starts data streaming for the provided source. */
		void StartStreaming(OAAudioSource* source);

		/** Stops data streaming for the provided source. */
		void StopStreaming(OAAudioSource* source);

		float mVolume = 1.0f;
		bool mIsPaused = false;

		ALCdevice* mDevice = nullptr;
		Vector<AudioDevice> mAllDevices;
		AudioDevice mDefaultDevice;
		AudioDevice mActiveDevice;

		Vector<OAAudioListener*> mListeners;
		Vector<ALCcontext*> mContexts;
		UnorderedSet<OAAudioSource*> mSources;

		// Streaming thread
		Vector<StreamingCommand> mStreamingCommandQueue;
		UnorderedSet<OAAudioSource*> mStreamingSources;
		UnorderedSet<OAAudioSource*> mDestroyedSources;
		SignalEvent mStreamingTaskSignal;
		mutable Mutex mMutex;
	};

	/** Provides easier access to OAAudio. */
	OAAudio& GetOAAudio();

	/** @} */
} // namespace b3d

//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsOAPrerequisites.h"
#include "Audio/BsAudio.h"
#include "AL/alc.h"

namespace bs
{
	/** @addtogroup OpenAudio
	 *  @{
	 */

	/** Global manager for the audio implementation using OpenAL as the backend. */
	class OAAudio : public Audio
	{
	public:
		OAAudio();
		virtual ~OAAudio();

		/** @copydoc Audio::setVolume */
		void SetVolume(float volume);

		/** @copydoc Audio::getVolume */
		float GetVolume() const;

		/** @copydoc Audio::setPaused */
		void SetPaused(bool paused);

		/** @copydoc Audio::isPaused */
		bool IsPaused() const { return mIsPaused; }

		/** @copydoc Audio::_update */
		void UpdateInternal() override;

		/** @copydoc Audio::setActiveDevice */
		void SetActiveDevice(const AudioDevice& device);

		/** @copydoc Audio::getActiveDevice */
		AudioDevice GetActiveDevice() const { return mActiveDevice; }

		/** @copydoc Audio::getDefaultDevice */
		AudioDevice GetDefaultDevice() const { return mDefaultDevice; }

		/** @copydoc Audio::getAllDevices */
		const Vector<AudioDevice>& GetAllDevices() const { return mAllDevices; };

		/** @name Internal
		 *  @{
		 */

		/** Checks is a specific OpenAL extension supported. */
		bool IsExtensionSupportedInternal(const String& extension) const;

		/** Registers a new AudioListener. Should be called on listener creation. */
		void RegisterListenerInternal(OAAudioListener* listener);

		/** Unregisters an existing AudioListener. Should be called before listener destruction. */
		void UnregisterListenerInternal(OAAudioListener* listener);

		/** Registers a new AudioSource. Should be called on source creation. */
		void RegisterSourceInternal(OAAudioSource* source);

		/** Unregisters an existing AudioSource. Should be called before source destruction. */
		void UnregisterSourceInternal(OAAudioSource* source);

		/** Returns a list of all OpenAL contexts. Each listener has its own context. */
		const Vector<ALCcontext*>& GetContextsInternal() const { return mContexts; }

		/** Returns an OpenAL context assigned to the provided listener. */
		ALCcontext* GetContextInternal(const OAAudioListener* listener) const;

		/**
		 * Returns optimal format for the provided number of channels and bit depth. It is assumed the user has checked if
		 * extensions providing these formats are actually available.
		 */
		i32 GetOpenALBufferFormatInternal(u32 numChannels, u32 bitDepth);

		/**
		 * Writes provided samples into the OpenAL buffer with the provided ID. If the provided format is not supported the
		 * samples will first be converted into a valid format.
		 */
		void WriteToOpenALBufferInternal(u32 bufferId, u8* samples, const AudioDataInfo& info);

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

		/** @copydoc Audio::createClip */
		SPtr<AudioClip> CreateClip(const SPtr<DataStream>& samples, u32 streamSize, u32 numSamples, const AUDIO_CLIP_DESC& desc);

		/** @copydoc Audio::createListener */
		SPtr<AudioListener> CreateListener();

		/** @copydoc Audio::createSource */
		SPtr<AudioSource> CreateSource() override;

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
		SPtr<Task> mStreamingTask;
		mutable Mutex mMutex;
	};

	/** Provides easier access to OAAudio. */
	OAAudio& gOAAudio();

	/** @} */
} // namespace bs

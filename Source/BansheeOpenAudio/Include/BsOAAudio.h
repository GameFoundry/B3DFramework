//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsOAPrerequisites.h"
#include "BsAudio.h"
#include "AL/alc.h"

namespace BansheeEngine
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
		void setVolume(float volume) override;

		/** @copydoc Audio::getVolume */
		float getVolume() const override;

		/** @copydoc Audio::setPaused */
		void setPaused(bool paused) override;

		/** @copydoc Audio::isPaused */
		bool isPaused() const override { return mIsPaused; }

		/** @copydoc Audio::_update */
		void _update() override;

		/** @copydoc Audio::setActiveDevice */
		void setActiveDevice(const AudioDevice& device) override;

		/** @copydoc Audio::getActiveDevice */
		AudioDevice getActiveDevice() const override { return mActiveDevice; }

		/** @copydoc Audio::getDefaultDevice */
		AudioDevice getDefaultDevice() const override { return mDefaultDevice; }

		/** @copydoc Audio::getAllDevices */
		const Vector<AudioDevice>& getAllDevices() const override { return mAllDevices; };

		/** @name Internal 
		 *  @{
		 */

		/** Checks is a specific OpenAL extension supported. */
		bool _isExtensionSupported(const String& extension) const;

		/** Registers a new AudioListener. Should be called on listener creation. */
		void _registerListener(OAAudioListener* listener);

		/** Unregisters an existing AudioListener. Should be called before listener destruction. */
		void _unregisterListener(OAAudioListener* listener);

		/** Registers a new AudioSource. Should be called on source creation. */
		void _registerSource(OAAudioSource* source);

		/** Unregisters an existing AudioSource. Should be called before source destruction. */
		void _unregisterSource(OAAudioSource* source);

		/** Returns a list of all OpenAL contexts. Each listener has its own context. */
		const Vector<ALCcontext*>& _getContexts() const { return mContexts; }

		/** Returns an OpenAL context assigned to the provided listener. */
		ALCcontext* _getContext(const OAAudioListener* listener) const;

		/** 
		 * Returns optimal format for the provided number of channels and bit depth. It is assumed the user has checked if
		 * extensions providing these formats are actually available.
		 */
		INT32 _getOpenALBufferFormat(UINT32 numChannels, UINT32 bitDepth);

		/** 
		 * Writes provided samples into the OpenAL buffer with the provided ID. If the provided format is not supported the
		 * samples will first be converted into a valid format.
		 */
		void _writeToOpenALBuffer(UINT32 bufferId, UINT8* samples, const AudioDataInfo& info);

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
			StreamingCommandType type;
			OAAudioSource* source;
		};

		/** @copydoc Audio::createClip */
		SPtr<AudioClip> createClip(const SPtr<DataStream>& samples, UINT32 streamSize, UINT32 numSamples,
			const AUDIO_CLIP_DESC& desc) override;

		/** @copydoc Audio::createListener */
		SPtr<AudioListener> createListener() override;

		/** @copydoc Audio::createSource */
		SPtr<AudioSource> createSource() override;

		/** 
		 * Delete all existing contexts and rebuild them according to the listener list. All audio sources will be rebuilt
		 * as well.
		 *
		 * This should be called when listener count changes, or audio device is changed.
		 */
		void rebuildContexts();

		/** Delete all existing OpenAL contexts. */
		void clearContexts();

		/** Streams new data to audio sources that require it. */
		void updateStreaming();

		/** Starts data streaming for the provided source. */
		void startStreaming(OAAudioSource* source);

		/** Stops data streaming for the provided source. */
		void stopStreaming(OAAudioSource* source);

		float mVolume;
		bool mIsPaused;

		ALCdevice* mDevice;
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
}
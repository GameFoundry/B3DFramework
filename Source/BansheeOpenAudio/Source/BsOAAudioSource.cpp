//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsOAAudioSource.h"
#include "BsOAAudio.h"
#include "BsOAAudioClip.h"
#include "AL/al.h"

namespace BansheeEngine
{
	OAAudioSource::OAAudioSource()
		: mSavedTime(0.0f), mState(AudioSourceState::Stopped), mGloballyPaused(false), mStreamBuffers(), mBusyBuffers()
		, mStreamProcessedPosition(0), mStreamQueuedPosition(0), mIsStreaming(false)
	{
		gOAAudio()._registerSource(this);
		rebuild();
	}

	OAAudioSource::~OAAudioSource()
	{
		clear();
		gOAAudio()._unregisterSource(this);
	}

	void OAAudioSource::setClip(const HAudioClip& clip)
	{
		stop();

		Lock(mMutex);
		AudioSource::setClip(clip);

		auto& contexts = gOAAudio()._getContexts();
		UINT32 numContexts = (UINT32)contexts.size();
		for (UINT32 i = 0; i < numContexts; i++)
		{
			if (contexts.size() > 1) // If only one context is available it is guaranteed it is always active, so we can avoid setting it
				alcMakeContextCurrent(contexts[i]);

			alSourcei(mSourceIDs[i], AL_SOURCE_RELATIVE, !is3D());

			if (!requiresStreaming())
			{
				UINT32 oaBuffer = 0;
				if (clip.isLoaded())
				{
					OAAudioClip* oaClip = static_cast<OAAudioClip*>(clip.get());
					oaBuffer = oaClip->_getOpenALBuffer();
				}
				
				alSourcei(mSourceIDs[i], AL_BUFFER, oaBuffer);
			}
		}

		// Looping is influenced by streaming mode, so re-apply it in case it changed
		setIsLooping(mLoop);
	}

	void OAAudioSource::setPosition(const Vector3& position)
	{
		AudioSource::setPosition(position);

		auto& contexts = gOAAudio()._getContexts();
		UINT32 numContexts = (UINT32)contexts.size();
		for (UINT32 i = 0; i < numContexts; i++)
		{
			if (contexts.size() > 1) // If only one context is available it is guaranteed it is always active, so we can avoid setting it
				alcMakeContextCurrent(contexts[i]);

			if (is3D())
				alSource3f(mSourceIDs[i], AL_POSITION, position.x, position.y, position.z);
			else
				alSource3f(mSourceIDs[i], AL_POSITION, 0.0f, 0.0f, 0.0f);
		}
	}

	void OAAudioSource::setVelocity(const Vector3& velocity)
	{
		AudioSource::setVelocity(velocity);

		auto& contexts = gOAAudio()._getContexts();
		UINT32 numContexts = (UINT32)contexts.size();
		for (UINT32 i = 0; i < numContexts; i++)
		{
			if (contexts.size() > 1) // If only one context is available it is guaranteed it is always active, so we can avoid setting it
				alcMakeContextCurrent(contexts[i]);

			if (is3D())
				alSource3f(mSourceIDs[i], AL_VELOCITY, velocity.x, velocity.y, velocity.z);
			else
				alSource3f(mSourceIDs[i], AL_VELOCITY, 0.0f, 0.0f, 0.0f);
		}
	}

	void OAAudioSource::setVolume(float volume)
	{
		AudioSource::setVolume(volume);

		auto& contexts = gOAAudio()._getContexts();
		UINT32 numContexts = (UINT32)contexts.size();
		for (UINT32 i = 0; i < numContexts; i++)
		{
			if (contexts.size() > 1) // If only one context is available it is guaranteed it is always active, so we can avoid setting it
				alcMakeContextCurrent(contexts[i]);

			alSourcef(mSourceIDs[i], AL_GAIN, mVolume);
		}
	}

	void OAAudioSource::setPitch(float pitch)
	{
		AudioSource::setPitch(pitch);

		auto& contexts = gOAAudio()._getContexts();
		UINT32 numContexts = (UINT32)contexts.size();
		for (UINT32 i = 0; i < numContexts; i++)
		{
			if (contexts.size() > 1) // If only one context is available it is guaranteed it is always active, so we can avoid setting it
				alcMakeContextCurrent(contexts[i]);

			alSourcef(mSourceIDs[i], AL_PITCH, pitch);
		}		
	}

	void OAAudioSource::setIsLooping(bool loop)
	{
		AudioSource::setIsLooping(loop);

		// When streaming we handle looping manually
		if (requiresStreaming())
			loop = false;

		auto& contexts = gOAAudio()._getContexts();
		UINT32 numContexts = (UINT32)contexts.size();
		for (UINT32 i = 0; i < numContexts; i++)
		{
			if (contexts.size() > 1) // If only one context is available it is guaranteed it is always active, so we can avoid setting it
				alcMakeContextCurrent(contexts[i]);

			alSourcef(mSourceIDs[i], AL_LOOPING, loop);
		}
	}

	void OAAudioSource::setPriority(INT32 priority)
	{
		AudioSource::setPriority(priority);

		// Do nothing, OpenAL doesn't support priorities (perhaps emulate the behaviour by manually disabling sources?)
	}

	void OAAudioSource::setMinDistance(float distance)
	{
		AudioSource::setMinDistance(distance);

		auto& contexts = gOAAudio()._getContexts();
		UINT32 numContexts = (UINT32)contexts.size();
		for (UINT32 i = 0; i < numContexts; i++)
		{
			if (contexts.size() > 1) // If only one context is available it is guaranteed it is always active, so we can avoid setting it
				alcMakeContextCurrent(contexts[i]);

			alSourcef(mSourceIDs[i], AL_REFERENCE_DISTANCE, distance);
		}
	}

	void OAAudioSource::setAttenuation(float attenuation)
	{
		AudioSource::setAttenuation(attenuation);

		auto& contexts = gOAAudio()._getContexts();
		UINT32 numContexts = (UINT32)contexts.size();
		for (UINT32 i = 0; i < numContexts; i++)
		{
			if (contexts.size() > 1) // If only one context is available it is guaranteed it is always active, so we can avoid setting it
				alcMakeContextCurrent(contexts[i]);

			alSourcef(mSourceIDs[i], AL_ROLLOFF_FACTOR, attenuation);
		}
	}

	void OAAudioSource::play()
	{
		mState = AudioSourceState::Playing;

		if (mGloballyPaused)
			return;

		if(requiresStreaming())
		{
			Lock(mMutex);
			
			if (!mIsStreaming)
			{
				startStreaming();
				stream(); // Stream first block on this thread to ensure something can play right away
			}
		}
		
		auto& contexts = gOAAudio()._getContexts();
		UINT32 numContexts = (UINT32)contexts.size();
		for (UINT32 i = 0; i < numContexts; i++)
		{
			if (contexts.size() > 1) // If only one context is available it is guaranteed it is always active, so we can avoid setting it
				alcMakeContextCurrent(contexts[i]);

			alSourcePlay(mSourceIDs[i]);
		}
	}

	void OAAudioSource::pause()
	{
		mState = AudioSourceState::Paused;

		auto& contexts = gOAAudio()._getContexts();
		UINT32 numContexts = (UINT32)contexts.size();
		for (UINT32 i = 0; i < numContexts; i++)
		{
			if (contexts.size() > 1) // If only one context is available it is guaranteed it is always active, so we can avoid setting it
				alcMakeContextCurrent(contexts[i]);

			alSourcePause(mSourceIDs[i]);
		}
	}

	void OAAudioSource::stop()
	{
		mState = AudioSourceState::Stopped;

		auto& contexts = gOAAudio()._getContexts();
		UINT32 numContexts = (UINT32)contexts.size();
		for (UINT32 i = 0; i < numContexts; i++)
		{
			if (contexts.size() > 1) // If only one context is available it is guaranteed it is always active, so we can avoid setting it
				alcMakeContextCurrent(contexts[i]);

			alSourceStop(mSourceIDs[i]);
			alSourcef(mSourceIDs[i], AL_SEC_OFFSET, 0.0f);
		}

		{
			Lock(mMutex);

			mStreamProcessedPosition = 0;
			mStreamQueuedPosition = 0;

			if (mIsStreaming)
				stopStreaming();
		}
	}

	void OAAudioSource::setGlobalPause(bool pause)
	{
		if (mGloballyPaused == pause)
			return;

		mGloballyPaused = pause;

		if (getState() == AudioSourceState::Playing)
		{
			if (pause)
			{
				auto& contexts = gOAAudio()._getContexts();
				UINT32 numContexts = (UINT32)contexts.size();
				for (UINT32 i = 0; i < numContexts; i++)
				{
					if (contexts.size() > 1) // If only one context is available it is guaranteed it is always active, so we can avoid setting it
						alcMakeContextCurrent(contexts[i]);

					alSourcePause(mSourceIDs[i]);
				}
			}
			else
			{
				play();
			}
		}
	}

	void OAAudioSource::setTime(float time)
	{
		if (!mAudioClip.isLoaded())
			return;

		AudioSourceState state = getState();
		stop();

		bool needsStreaming = requiresStreaming();
		float clipTime;
		{
			Lock(mMutex);

			if (!needsStreaming)
				clipTime = time;
			else
			{
				if (mAudioClip.isLoaded())
					mStreamProcessedPosition = (UINT32)(time * mAudioClip->getFrequency() * mAudioClip->getNumChannels());
				else
					mStreamProcessedPosition = 0;

				mStreamQueuedPosition = mStreamProcessedPosition;
				clipTime = 0.0f;
			}
		}

		auto& contexts = gOAAudio()._getContexts();
		UINT32 numContexts = (UINT32)contexts.size();
		for (UINT32 i = 0; i < numContexts; i++)
		{
			if (contexts.size() > 1) // If only one context is available it is guaranteed it is always active, so we can avoid setting it
				alcMakeContextCurrent(contexts[i]);

			alSourcef(mSourceIDs[i], AL_SEC_OFFSET, clipTime);
		}

		if (state != AudioSourceState::Stopped)
			play();
		
		if (state == AudioSourceState::Paused)
			pause();
	}

	float OAAudioSource::getTime() const
	{
		Lock(mMutex);

		auto& contexts = gOAAudio()._getContexts();

		if (contexts.size() > 1) // If only one context is available it is guaranteed it is always active, so we can avoid setting it
			alcMakeContextCurrent(contexts[0]);

		bool needsStreaming = requiresStreaming();
		float time;
		if (!needsStreaming)
		{
			alGetSourcef(mSourceIDs[0], AL_SEC_OFFSET, &time);
			return time;
		}
		else
		{
			float timeOffset = 0.0f;
			if (mAudioClip.isLoaded())
				timeOffset = (float)mStreamProcessedPosition / mAudioClip->getFrequency() / mAudioClip->getNumChannels();

			// When streaming, the returned offset is relative to the last queued buffer
			alGetSourcef(mSourceIDs[0], AL_SEC_OFFSET, &time);
			return timeOffset + time;
		}
	}

	void OAAudioSource::clear()
	{
		mSavedTime = getTime();
		stop();
		
		for (auto& source : mSourceIDs)
			alSourcei(source, AL_BUFFER, 0);

		{
			Lock(mMutex);

			alDeleteSources((UINT32)mSourceIDs.size(), mSourceIDs.data());
			mSourceIDs.clear();
		}
	}

	void OAAudioSource::rebuild()
	{
		AudioSourceState state = getState();
		auto& contexts = gOAAudio()._getContexts();

		{
			Lock(mMutex);

			mSourceIDs.resize(contexts.size());
			alGenSources((UINT32)mSourceIDs.size(), mSourceIDs.data());
		}

		UINT32 numContexts = (UINT32)contexts.size();
		for (UINT32 i = 0; i < numContexts; i++)
		{
			if (contexts.size() > 1) // If only one context is available it is guaranteed it is always active, so we can avoid setting it
				alcMakeContextCurrent(contexts[i]);

			alSourcef(mSourceIDs[i], AL_PITCH, mPitch);
			alSourcef(mSourceIDs[i], AL_REFERENCE_DISTANCE, mMinDistance);
			alSourcef(mSourceIDs[i], AL_ROLLOFF_FACTOR, mAttenuation);

			if(requiresStreaming())
				alSourcef(mSourceIDs[i], AL_LOOPING, false);
			else
				alSourcef(mSourceIDs[i], AL_LOOPING, mLoop);

			if (is3D())
			{
				alSourcei(mSourceIDs[i], AL_SOURCE_RELATIVE, false);
				alSource3f(mSourceIDs[i], AL_POSITION, mPosition.x, mPosition.y, mPosition.z);
				alSource3f(mSourceIDs[i], AL_VELOCITY, mVelocity.x, mVelocity.y, mVelocity.z);
			}
			else
			{
				alSourcei(mSourceIDs[i], AL_SOURCE_RELATIVE, true);
				alSource3f(mSourceIDs[i], AL_POSITION, 0.0f, 0.0f, 0.0f);
				alSource3f(mSourceIDs[i], AL_VELOCITY, 0.0f, 0.0f, 0.0f);
			}

			{
				Lock(mMutex);

				if (!mIsStreaming)
				{
					UINT32 oaBuffer = 0;
					if (mAudioClip.isLoaded())
					{
						OAAudioClip* oaClip = static_cast<OAAudioClip*>(mAudioClip.get());
						oaBuffer = oaClip->_getOpenALBuffer();
					}

					alSourcei(mSourceIDs[i], AL_BUFFER, oaBuffer);
				}
			}
		}

		setTime(mSavedTime);

		if (state != AudioSourceState::Stopped)
			play();

		if (state == AudioSourceState::Paused)
			pause();
	}

	void OAAudioSource::startStreaming()
	{
		assert(!mIsStreaming);

		alGenBuffers(StreamBufferCount, mStreamBuffers);
		gOAAudio().startStreaming(this);

		memset(&mBusyBuffers, 0, sizeof(mBusyBuffers));
		mIsStreaming = true;
	}

	void OAAudioSource::stopStreaming()
	{
		assert(mIsStreaming);

		mIsStreaming = false;
		gOAAudio().stopStreaming(this);

		auto& contexts = gOAAudio()._getContexts();
		UINT32 numContexts = (UINT32)contexts.size();
		for (UINT32 i = 0; i < numContexts; i++)
		{
			if (contexts.size() > 1) // If only one context is available it is guaranteed it is always active, so we can avoid setting it
				alcMakeContextCurrent(contexts[i]);

			INT32 numQueuedBuffers;
			alGetSourcei(mSourceIDs[i], AL_BUFFERS_QUEUED, &numQueuedBuffers);

			UINT32 buffer;
			for (INT32 j = 0; j < numQueuedBuffers; j++)
				alSourceUnqueueBuffers(mSourceIDs[i], 1, &buffer);
		}

		alDeleteBuffers(StreamBufferCount, mStreamBuffers);
	}

	void OAAudioSource::stream()
	{
		Lock(mMutex);

		// Note: Not setting context here. This might be an issue when multiple contexts are used. If context setting
		// ends up to be needed, then I'll need to lock every audio source operation to avoid other thread changing
		// the context.

		AudioDataInfo info;
		info.bitDepth = mAudioClip->getBitDepth();
		info.numChannels = mAudioClip->getNumChannels();
		info.sampleRate = mAudioClip->getFrequency();
		info.numSamples = 0;

		UINT32 totalNumSamples = mAudioClip->getNumSamples();

		// Note: This code only uses the first source to determine the number of processed buffers. This will be an issue
		// if other sources haven't yet processed those same buffers. No easy way to fix that situation other than to
		// use different buffers for each source.

		INT32 numProcessedBuffers = 0;
		alGetSourcei(mSourceIDs[0], AL_BUFFERS_PROCESSED, &numProcessedBuffers);

		for (INT32 i = numProcessedBuffers; i > 0; i--)
		{
			UINT32 buffer;
			alSourceUnqueueBuffers(mSourceIDs[0], 1, &buffer);

			INT32 bufferIdx = -1;
			for (UINT32 j = 0; j < StreamBufferCount; j++)
			{
				if (buffer == mStreamBuffers[j])
				{
					bufferIdx = j;
					break;
				}
			}
			
			// Possibly some buffer from previous playback remained unqueued, in which case ignore it
			if (bufferIdx == -1)
				continue;

			mBusyBuffers[bufferIdx] = false;

			INT32 bufferSize;
			INT32 bufferBits;

			alGetBufferi(buffer, AL_SIZE, &bufferSize);
			alGetBufferi(buffer, AL_BITS, &bufferBits);

			if (bufferBits == 0)
			{
				LOGERR("Error decoding stream.");
				return;
			}
			else
			{
				UINT32 bytesPerSample = bufferBits / 8;
				mStreamProcessedPosition += bufferSize / bytesPerSample;
			}

			if (mStreamProcessedPosition == totalNumSamples) // Reached the end
			{
				mStreamProcessedPosition = 0;

				if (!mLoop) // Variable used on both threads and not thread safe, but it doesn't matter
				{
					stopStreaming();
					return;
				}
			}
		}

		for(UINT32 i = 0; i < StreamBufferCount; i++)
		{
			if (mBusyBuffers[i])
				continue;

			if (fillBuffer(mStreamBuffers[i], info, totalNumSamples))
			{
				for (auto& source : mSourceIDs)
					alSourceQueueBuffers(source, 1, &mStreamBuffers[i]);

				mBusyBuffers[i] = true;
			}
			else
				break;
		}
	}

	bool OAAudioSource::fillBuffer(UINT32 buffer, AudioDataInfo& info, UINT32 maxNumSamples)
	{
		UINT32 numRemainingSamples = maxNumSamples - mStreamQueuedPosition;
		if (numRemainingSamples == 0) // Reached the end
		{
			if (mLoop)
			{
				mStreamQueuedPosition = 0;
				numRemainingSamples = maxNumSamples;
			}
			else // If not looping, don't queue any more buffers, we're done
				return false;
		}

		// Read audio data
		UINT32 numSamples = std::min(numRemainingSamples, info.sampleRate * info.numChannels); // 1 second of data
		UINT32 sampleBufferSize = numSamples * (info.bitDepth / 8);

		UINT8* samples = (UINT8*)bs_stack_alloc(sampleBufferSize);

		OAAudioClip* audioClip = static_cast<OAAudioClip*>(mAudioClip.get());

		audioClip->getSamples(samples, mStreamQueuedPosition, numSamples);
		mStreamQueuedPosition += numSamples;

		info.numSamples = numSamples;
		gOAAudio()._writeToOpenALBuffer(buffer, samples, info);

		bs_stack_free(samples);

		return true;
	}

	bool OAAudioSource::is3D() const
	{
		if (!mAudioClip.isLoaded())
			return true;

		return mAudioClip->is3D();
	}

	bool OAAudioSource::requiresStreaming() const
	{
		if (!mAudioClip.isLoaded())
			return false;

		AudioReadMode readMode = mAudioClip->getReadMode();
		bool isCompressed = readMode == AudioReadMode::LoadCompressed && mAudioClip->getFormat() != AudioFormat::PCM;

		return (readMode == AudioReadMode::Stream) || isCompressed;
	}
}
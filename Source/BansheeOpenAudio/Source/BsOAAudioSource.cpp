//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsOAAudioSource.h"
#include "BsOAAudio.h"
#include "BsOAAudioClip.h"
#include "AL/al.h"

namespace BansheeEngine
{
	OAAudioSource::OAAudioSource()
		:mSeekPosition(0), mRequiresStreaming(false)
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
		AudioSource::setClip(clip);

		stop(); // TODO: I should wait until all buffers are unqueued before proceeding
		mRequiresStreaming = isStreaming();

		auto& contexts = gOAAudio()._getContexts();
		UINT32 numContexts = (UINT32)contexts.size();
		for (UINT32 i = 0; i < numContexts; i++)
		{
			if (contexts.size() > 1) // If only one context is available it is guaranteed it is always active, so we can avoid setting it
				alcMakeContextCurrent(contexts[i]);

			alSourcei(mSourceIDs[i], AL_SOURCE_RELATIVE, !is3D());

			if (!mRequiresStreaming)
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

			alSourcef(mSourceIDs[i], AL_GAIN, volume);
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

		auto& contexts = gOAAudio()._getContexts();
		UINT32 numContexts = (UINT32)contexts.size();
		for (UINT32 i = 0; i < numContexts; i++)
		{
			if (contexts.size() > 1) // If only one context is available it is guaranteed it is always active, so we can avoid setting it
				alcMakeContextCurrent(contexts[i]);

			alSourcef(mSourceIDs[i], AL_LOOPING, loop);
		}
	}

	void OAAudioSource::setPriority(UINT32 priority)
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
		AudioSourceState state = getState();
		AudioSource::play();

		if (state == AudioSourceState::Playing)
			return;
		
		if(!mRequiresStreaming || state == AudioSourceState::Paused)
		{
			auto& contexts = gOAAudio()._getContexts();
			UINT32 numContexts = (UINT32)contexts.size();
			for (UINT32 i = 0; i < numContexts; i++)
			{
				if (contexts.size() > 1) // If only one context is available it is guaranteed it is always active, so we can avoid setting it
					alcMakeContextCurrent(contexts[i]);

				alSourcePlay(mSourceIDs[i]);
			}
		} 
		else // Streaming and currently stopped
		{
			gOAAudio().startStreaming(this, false);
		}
	}

	void OAAudioSource::pause()
	{
		AudioSourceState state = getState();
		AudioSource::pause();

		if (state == AudioSourceState::Paused)
			return;

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
		AudioSourceState state = getState();
		AudioSource::stop();

		if (state == AudioSourceState::Stopped)
			return;

		auto& contexts = gOAAudio()._getContexts();
		UINT32 numContexts = (UINT32)contexts.size();
		for (UINT32 i = 0; i < numContexts; i++)
		{
			if (contexts.size() > 1) // If only one context is available it is guaranteed it is always active, so we can avoid setting it
				alcMakeContextCurrent(contexts[i]);

			alSourceStop(mSourceIDs[i]);
		}

		if (mRequiresStreaming)
			gOAAudio().stopStreaming(this);
	}

	void OAAudioSource::seek(float position)
	{
		if (mAudioClip.isLoaded())
			mSeekPosition = position * mAudioClip->getFrequency() * mAudioClip->getNumChannels();
		else
			mSeekPosition = 0;

		if (!mRequiresStreaming)
		{
			auto& contexts = gOAAudio()._getContexts();
			UINT32 numContexts = (UINT32)contexts.size();
			for (UINT32 i = 0; i < numContexts; i++)
			{
				if (contexts.size() > 1) // If only one context is available it is guaranteed it is always active, so we can avoid setting it
					alcMakeContextCurrent(contexts[i]);

				alSourcef(mSourceIDs[i], AL_SEC_OFFSET, position);
			}
		}
		else
		{
			AudioSourceState state = getState();
			if(state != AudioSourceState::Stopped)
			{
				gOAAudio().stopStreaming(this);
				gOAAudio().startStreaming(this, state == AudioSourceState::Paused);
			}
		}
	}

	void OAAudioSource::clear()
	{
		if (mAudioClip.isLoaded())
			mSeekPosition = tell() * mAudioClip->getFrequency() * mAudioClip->getNumChannels();
		else
			mSeekPosition = 0;

		auto destroySources = [this]()
		{
			for (auto& source : mSourceIDs)
				alSourcei(source, AL_BUFFER, 0);

			alDeleteSources((UINT32)mSourceIDs.size(), mSourceIDs.data());
			mSourceIDs.clear();
		};

		if(mRequiresStreaming)
		{
			AudioSourceState state = getState();
			if (state != AudioSourceState::Stopped)
			{
				gOAAudio().stopStreaming(this);
				gOAAudio().queueCommand(destroySources);
			}
		}
		else
		{
			// Still used by the streaming thread
			if (gOAAudio().isStreaming(this)) // Assuming stopStreaming was already called
				gOAAudio().queueCommand(destroySources);
			else
				destroySources();
		}
	}

	void OAAudioSource::rebuild()
	{
		AudioSourceState state = getState();
		auto& contexts = gOAAudio()._getContexts();

		mSourceIDs.resize(contexts.size());
		alGenSources((UINT32)mSourceIDs.size(), mSourceIDs.data());

		UINT32 numContexts = (UINT32)contexts.size();
		for (UINT32 i = 0; i < numContexts; i++)
		{
			if (contexts.size() > 1) // If only one context is available it is guaranteed it is always active, so we can avoid setting it
				alcMakeContextCurrent(contexts[i]);

			alSourcef(mSourceIDs[i], AL_PITCH, mPitch);
			alSourcef(mSourceIDs[i], AL_LOOPING, mLoop);
			alSourcef(mSourceIDs[i], AL_REFERENCE_DISTANCE, mMinDistance);
			alSourcef(mSourceIDs[i], AL_ROLLOFF_FACTOR, mAttenuation);
			alSourcef(mSourceIDs[i], AL_SEC_OFFSET, mSeekPosition);

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

			if (!mRequiresStreaming)
			{
				UINT32 oaBuffer = 0;
				if (mAudioClip.isLoaded())
				{
					OAAudioClip* oaClip = static_cast<OAAudioClip*>(mAudioClip.get());
					oaBuffer = oaClip->_getOpenALBuffer();
				}

				alSourcei(mSourceIDs[i], AL_BUFFER, oaBuffer);

				float offset = 0.0f;
				if (mAudioClip.isLoaded())
					offset = (float)mSeekPosition / mAudioClip->getFrequency() / mAudioClip->getNumChannels();

				alSourcef(mSourceIDs[i], AL_SEC_OFFSET, offset);
			}
			else
				alSourcef(mSourceIDs[i], AL_SEC_OFFSET, 0.0f); // Offset handled by the streaming system

			if (state == AudioSourceState::Playing || state == AudioSourceState::Paused)
				alSourcePlay(mSourceIDs[i]);
			
			if (state == AudioSourceState::Paused)
				alSourcePause(mSourceIDs[i]);
		}

		if(mRequiresStreaming)
		{
			if (state != AudioSourceState::Stopped)
				gOAAudio().startStreaming(this, state == AudioSourceState::Paused);
		}
	}

	bool OAAudioSource::is3D() const
	{
		if (!mAudioClip.isLoaded())
			return true;

		return mAudioClip->is3D();
	}

	bool OAAudioSource::isStreaming() const
	{
		if (!mAudioClip.isLoaded())
			return false;

		AudioReadMode readMode = mAudioClip->getReadMode();
		bool isCompressed = readMode == AudioReadMode::LoadCompressed && mAudioClip->getFormat() != AudioFormat::PCM;

		return (readMode == AudioReadMode::Stream) || isCompressed;
	}
}
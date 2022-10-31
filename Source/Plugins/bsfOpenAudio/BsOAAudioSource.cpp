//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsOAAudioSource.h"
#include "BsOAAudio.h"
#include "BsOAAudioClip.h"
#include "AL/al.h"

using namespace bs;

OAAudioSource::OAAudioSource()
	: mStreamBuffers(), mBusyBuffers()
{
	GetOAAudio().RegisterSourceInternal(this);
	Rebuild();
}

OAAudioSource::~OAAudioSource()
{
	Clear();
	GetOAAudio().UnregisterSourceInternal(this);
}

void OAAudioSource::SetClip(const HAudioClip& clip)
{
	Stop();

	Lock lock(mMutex);
	AudioSource::SetClip(clip);

	ApplyClip();
}

void OAAudioSource::SetTransform(const Transform& transform)
{
	SceneActor::SetTransform(transform);

	auto& contexts = GetOAAudio().GetContextsInternal();
	u32 numContexts = (u32)contexts.size();
	for(u32 i = 0; i < numContexts; i++)
	{
		if(contexts.size() > 1)
			alcMakeContextCurrent(contexts[i]);

		if(Is3D())
		{
			Vector3 position = transform.GetPosition();
			alSource3f(mSourceIDs[i], AL_POSITION, position.X, position.Y, position.Z);
		}
		else
			alSource3f(mSourceIDs[i], AL_POSITION, 0.0f, 0.0f, 0.0f);
	}
}

void OAAudioSource::SetVelocity(const Vector3& velocity)
{
	AudioSource::SetVelocity(velocity);

	auto& contexts = GetOAAudio().GetContextsInternal();
	u32 numContexts = (u32)contexts.size();
	for(u32 i = 0; i < numContexts; i++)
	{
		if(contexts.size() > 1)
			alcMakeContextCurrent(contexts[i]);

		if(Is3D())
			alSource3f(mSourceIDs[i], AL_VELOCITY, velocity.X, velocity.Y, velocity.Z);
		else
			alSource3f(mSourceIDs[i], AL_VELOCITY, 0.0f, 0.0f, 0.0f);
	}
}

void OAAudioSource::SetVolume(float volume)
{
	AudioSource::SetVolume(volume);

	auto& contexts = GetOAAudio().GetContextsInternal();
	u32 numContexts = (u32)contexts.size();
	for(u32 i = 0; i < numContexts; i++)
	{
		if(contexts.size() > 1)
			alcMakeContextCurrent(contexts[i]);

		alSourcef(mSourceIDs[i], AL_GAIN, mVolume);
	}
}

void OAAudioSource::SetPitch(float pitch)
{
	AudioSource::SetPitch(pitch);

	auto& contexts = GetOAAudio().GetContextsInternal();
	u32 numContexts = (u32)contexts.size();
	for(u32 i = 0; i < numContexts; i++)
	{
		if(contexts.size() > 1)
			alcMakeContextCurrent(contexts[i]);

		alSourcef(mSourceIDs[i], AL_PITCH, pitch);
	}
}

void OAAudioSource::SetIsLooping(bool loop)
{
	AudioSource::SetIsLooping(loop);

	// When streaming we handle looping manually
	if(RequiresStreaming())
		loop = false;

	auto& contexts = GetOAAudio().GetContextsInternal();
	u32 numContexts = (u32)contexts.size();
	for(u32 i = 0; i < numContexts; i++)
	{
		if(contexts.size() > 1)
			alcMakeContextCurrent(contexts[i]);

		alSourcei(mSourceIDs[i], AL_LOOPING, loop);
	}
}

void OAAudioSource::SetPriority(i32 priority)
{
	AudioSource::SetPriority(priority);

	// Do nothing, OpenAL doesn't support priorities (perhaps emulate the behaviour by manually disabling sources?)
}

void OAAudioSource::SetMinDistance(float distance)
{
	AudioSource::SetMinDistance(distance);

	auto& contexts = GetOAAudio().GetContextsInternal();
	u32 numContexts = (u32)contexts.size();
	for(u32 i = 0; i < numContexts; i++)
	{
		if(contexts.size() > 1)
			alcMakeContextCurrent(contexts[i]);

		alSourcef(mSourceIDs[i], AL_REFERENCE_DISTANCE, distance);
	}
}

void OAAudioSource::SetAttenuation(float attenuation)
{
	AudioSource::SetAttenuation(attenuation);

	auto& contexts = GetOAAudio().GetContextsInternal();
	u32 numContexts = (u32)contexts.size();
	for(u32 i = 0; i < numContexts; i++)
	{
		if(contexts.size() > 1)
			alcMakeContextCurrent(contexts[i]);

		alSourcef(mSourceIDs[i], AL_ROLLOFF_FACTOR, attenuation);
	}
}

void OAAudioSource::Play()
{
	if(mGloballyPaused)
		return;

	if(RequiresStreaming())
	{
		Lock lock(mMutex);

		if(!mIsStreaming)
		{
			StartStreaming();
			StreamUnlocked(); // Stream first block on this thread to ensure something can play right away
		}
	}

	auto& contexts = GetOAAudio().GetContextsInternal();
	u32 numContexts = (u32)contexts.size();
	for(u32 i = 0; i < numContexts; i++)
	{
		if(contexts.size() > 1)
			alcMakeContextCurrent(contexts[i]);

		alSourcePlay(mSourceIDs[i]);

		// Non-3D clips need to play only on a single source
		// Note: I'm still creating sourcs objects (and possibly queuing streaming buffers) for these non-playing
		// sources. It would be possible to optimize them out at cost of more complexity. At this time it doesn't feel
		// worth it.
		if(!Is3D())
			break;
	}
}

void OAAudioSource::Pause()
{
	auto& contexts = GetOAAudio().GetContextsInternal();
	u32 numContexts = (u32)contexts.size();
	for(u32 i = 0; i < numContexts; i++)
	{
		if(contexts.size() > 1)
			alcMakeContextCurrent(contexts[i]);

		alSourcePause(mSourceIDs[i]);
	}
}

void OAAudioSource::Stop()
{
	auto& contexts = GetOAAudio().GetContextsInternal();
	u32 numContexts = (u32)contexts.size();
	for(u32 i = 0; i < numContexts; i++)
	{
		if(contexts.size() > 1)
			alcMakeContextCurrent(contexts[i]);

		alSourceStop(mSourceIDs[i]);
		alSourcef(mSourceIDs[i], AL_SEC_OFFSET, 0.0f);
	}

	{
		Lock lock(mMutex);

		mStreamProcessedPosition = 0;
		mStreamQueuedPosition = 0;

		if(mIsStreaming)
			StopStreaming();
	}
}

void OAAudioSource::SetGlobalPause(bool pause)
{
	if(mGloballyPaused == pause)
		return;

	mGloballyPaused = pause;

	if(GetState() == AudioSourceState::Playing)
	{
		if(pause)
		{
			auto& contexts = GetOAAudio().GetContextsInternal();
			u32 numContexts = (u32)contexts.size();
			for(u32 i = 0; i < numContexts; i++)
			{
				if(contexts.size() > 1)
					alcMakeContextCurrent(contexts[i]);

				alSourcePause(mSourceIDs[i]);
			}
		}
		else
		{
			Play();
		}
	}
}

void OAAudioSource::SetTime(float time)
{
	if(!mAudioClip.IsLoaded())
		return;

	AudioSourceState state = GetState();
	Stop();

	bool needsStreaming = RequiresStreaming();
	float clipTime;
	{
		Lock lock(mMutex);

		if(!needsStreaming)
			clipTime = time;
		else
		{
			if(mAudioClip.IsLoaded())
				mStreamProcessedPosition = (u32)(time * mAudioClip->GetFrequency() * mAudioClip->GetNumChannels());
			else
				mStreamProcessedPosition = 0;

			mStreamQueuedPosition = mStreamProcessedPosition;
			clipTime = 0.0f;
		}
	}

	auto& contexts = GetOAAudio().GetContextsInternal();
	u32 numContexts = (u32)contexts.size();
	for(u32 i = 0; i < numContexts; i++)
	{
		if(contexts.size() > 1)
			alcMakeContextCurrent(contexts[i]);

		alSourcef(mSourceIDs[i], AL_SEC_OFFSET, clipTime);
	}

	if(state != AudioSourceState::Stopped)
		Play();

	if(state == AudioSourceState::Paused)
		Pause();
}

float OAAudioSource::GetTime() const
{
	Lock lock(mMutex);

	auto& contexts = GetOAAudio().GetContextsInternal();

	if(contexts.size() > 1)
		alcMakeContextCurrent(contexts[0]);

	bool needsStreaming = RequiresStreaming();
	float time;
	if(!needsStreaming)
	{
		alGetSourcef(mSourceIDs[0], AL_SEC_OFFSET, &time);
		return time;
	}
	else
	{
		float timeOffset = 0.0f;
		if(mAudioClip.IsLoaded())
			timeOffset = (float)mStreamProcessedPosition / mAudioClip->GetFrequency() / mAudioClip->GetNumChannels();

		// When streaming, the returned offset is relative to the last queued buffer
		alGetSourcef(mSourceIDs[0], AL_SEC_OFFSET, &time);
		return timeOffset + time;
	}
}

AudioSourceState OAAudioSource::GetState() const
{
	ALint state;
	alGetSourcei(mSourceIDs[0], AL_SOURCE_STATE, &state);

	switch(state)
	{
	case AL_PLAYING:
		return AudioSourceState::Playing;
	case AL_PAUSED:
		return AudioSourceState::Paused;
	case AL_INITIAL:
	case AL_STOPPED:
	default:
		return AudioSourceState::Stopped;
	}
}

void OAAudioSource::Clear()
{
	mSavedState = GetState();
	mSavedTime = GetTime();
	Stop();

	auto& contexts = GetOAAudio().GetContextsInternal();
	u32 numContexts = (u32)contexts.size();

	Lock lock(mMutex);
	for(u32 i = 0; i < numContexts; i++)
	{
		if(contexts.size() > 1)
			alcMakeContextCurrent(contexts[i]);

		alSourcei(mSourceIDs[i], AL_BUFFER, 0);
		alDeleteSources(1, &mSourceIDs[i]);
	}

	mSourceIDs.clear();
}

void OAAudioSource::Rebuild()
{
	auto& contexts = GetOAAudio().GetContextsInternal();
	u32 numContexts = (u32)contexts.size();

	{
		Lock lock(mMutex);

		for(u32 i = 0; i < numContexts; i++)
		{
			if(contexts.size() > 1)
				alcMakeContextCurrent(contexts[i]);

			u32 source = 0;
			alGenSources(1, &source);

			mSourceIDs.push_back(source);
		}
	}

	for(u32 i = 0; i < numContexts; i++)
	{
		if(contexts.size() > 1)
			alcMakeContextCurrent(contexts[i]);

		alSourcef(mSourceIDs[i], AL_PITCH, mPitch);
		alSourcef(mSourceIDs[i], AL_REFERENCE_DISTANCE, mMinDistance);
		alSourcef(mSourceIDs[i], AL_ROLLOFF_FACTOR, mAttenuation);

		if(RequiresStreaming())
			alSourcei(mSourceIDs[i], AL_LOOPING, false);
		else
			alSourcei(mSourceIDs[i], AL_LOOPING, mLoop);

		if(Is3D())
		{
			Vector3 position = mTransform.GetPosition();

			alSourcei(mSourceIDs[i], AL_SOURCE_RELATIVE, false);
			alSource3f(mSourceIDs[i], AL_POSITION, position.X, position.Y, position.Z);
			alSource3f(mSourceIDs[i], AL_VELOCITY, mVelocity.X, mVelocity.Y, mVelocity.Z);
		}
		else
		{
			alSourcei(mSourceIDs[i], AL_SOURCE_RELATIVE, true);
			alSource3f(mSourceIDs[i], AL_POSITION, 0.0f, 0.0f, 0.0f);
			alSource3f(mSourceIDs[i], AL_VELOCITY, 0.0f, 0.0f, 0.0f);
		}

		{
			Lock lock(mMutex);

			if(!mIsStreaming)
			{
				u32 oaBuffer = 0;
				if(mAudioClip.IsLoaded())
				{
					OAAudioClip* oaClip = static_cast<OAAudioClip*>(mAudioClip.Get());
					oaBuffer = oaClip->GetOpenALBufferInternal();
				}

				alSourcei(mSourceIDs[i], AL_BUFFER, oaBuffer);
			}
		}
	}

	SetTime(mSavedTime);

	if(mSavedState != AudioSourceState::Stopped)
		Play();

	if(mSavedState == AudioSourceState::Paused)
		Pause();
}

void OAAudioSource::StartStreaming()
{
	assert(!mIsStreaming);

	alGenBuffers(kStreamBufferCount, mStreamBuffers);
	GetOAAudio().StartStreaming(this);

	memset(&mBusyBuffers, 0, sizeof(mBusyBuffers));
	mIsStreaming = true;
}

void OAAudioSource::StopStreaming()
{
	assert(mIsStreaming);

	mIsStreaming = false;
	GetOAAudio().StopStreaming(this);

	auto& contexts = GetOAAudio().GetContextsInternal();
	u32 numContexts = (u32)contexts.size();
	for(u32 i = 0; i < numContexts; i++)
	{
		if(contexts.size() > 1)
			alcMakeContextCurrent(contexts[i]);

		i32 numQueuedBuffers;
		alGetSourcei(mSourceIDs[i], AL_BUFFERS_QUEUED, &numQueuedBuffers);

		u32 buffer;
		for(i32 j = 0; j < numQueuedBuffers; j++)
			alSourceUnqueueBuffers(mSourceIDs[i], 1, &buffer);
	}

	alDeleteBuffers(kStreamBufferCount, mStreamBuffers);
}

void OAAudioSource::Stream()
{
	Lock lock(mMutex);

	StreamUnlocked();
}

void OAAudioSource::StreamUnlocked()
{
	AudioDataInfo info;
	info.BitDepth = mAudioClip->GetBitDepth();
	info.NumChannels = mAudioClip->GetNumChannels();
	info.SampleRate = mAudioClip->GetFrequency();
	info.NumSamples = 0;

	u32 totalNumSamples = mAudioClip->GetNumSamples();

	// Note: It is safe to access contexts here only because it is guaranteed by the OAAudio manager that it will always
	// stop all streaming before changing contexts. Otherwise a mutex lock would be needed for every context access.
	auto& contexts = GetOAAudio().GetContextsInternal();
	u32 numContexts = (u32)contexts.size();
	for(u32 i = 0; i < numContexts; i++)
	{
		if(contexts.size() > 1)
			alcMakeContextCurrent(contexts[i]);

		i32 numProcessedBuffers = 0;
		alGetSourcei(mSourceIDs[i], AL_BUFFERS_PROCESSED, &numProcessedBuffers);

		for(i32 j = numProcessedBuffers; j > 0; j--)
		{
			u32 buffer;
			alSourceUnqueueBuffers(mSourceIDs[i], 1, &buffer);

			i32 bufferIdx = -1;
			for(u32 k = 0; k < kStreamBufferCount; k++)
			{
				if(buffer == mStreamBuffers[k])
				{
					bufferIdx = k;
					break;
				}
			}

			// Possibly some buffer from previous playback remained unqueued, in which case ignore it
			if(bufferIdx == -1)
				continue;

			mBusyBuffers[bufferIdx] &= ~(1 << bufferIdx);

			// Check if all sources are done with this buffer
			if(mBusyBuffers[bufferIdx] != 0)
				break;

			i32 bufferSize;
			i32 bufferBits;

			alGetBufferi(buffer, AL_SIZE, &bufferSize);
			alGetBufferi(buffer, AL_BITS, &bufferBits);

			if(bufferBits == 0)
			{
				BS_LOG(Error, Audio, "Error decoding stream.");
				return;
			}
			else
			{
				u32 bytesPerSample = bufferBits / 8;
				mStreamProcessedPosition += bufferSize / bytesPerSample;
			}

			if(mStreamProcessedPosition == totalNumSamples) // Reached the end
			{
				mStreamProcessedPosition = 0;

				if(!mLoop) // Variable used on both threads and not thread safe, but it doesn't matter
				{
					StopStreaming();
					return;
				}
			}
		}
	}

	for(u32 i = 0; i < kStreamBufferCount; i++)
	{
		if(mBusyBuffers[i] != 0)
			continue;

		if(FillBuffer(mStreamBuffers[i], info, totalNumSamples))
		{
			for(auto& source : mSourceIDs)
				alSourceQueueBuffers(source, 1, &mStreamBuffers[i]);

			mBusyBuffers[i] |= 1 << i;
		}
		else
			break;
	}
}

bool OAAudioSource::FillBuffer(u32 buffer, AudioDataInfo& info, u32 maxNumSamples)
{
	u32 numRemainingSamples = maxNumSamples - mStreamQueuedPosition;
	if(numRemainingSamples == 0) // Reached the end
	{
		if(mLoop)
		{
			mStreamQueuedPosition = 0;
			numRemainingSamples = maxNumSamples;
		}
		else // If not looping, don't queue any more buffers, we're done
			return false;
	}

	// Read audio data
	u32 numSamples = std::min(numRemainingSamples, info.SampleRate * info.NumChannels); // 1 second of data
	u32 sampleBufferSize = numSamples * (info.BitDepth / 8);

	u8* samples = (u8*)bs_stack_alloc(sampleBufferSize);

	OAAudioClip* audioClip = static_cast<OAAudioClip*>(mAudioClip.Get());

	audioClip->GetSamples(samples, mStreamQueuedPosition, numSamples);
	mStreamQueuedPosition += numSamples;

	info.NumSamples = numSamples;
	GetOAAudio().WriteToOpenALBufferInternal(buffer, samples, info);

	bs_stack_free(samples);

	return true;
}

void OAAudioSource::ApplyClip()
{
	auto& contexts = GetOAAudio().GetContextsInternal();
	u32 numContexts = (u32)contexts.size();
	for(u32 i = 0; i < numContexts; i++)
	{
		if(contexts.size() > 1)
			alcMakeContextCurrent(contexts[i]);

		alSourcei(mSourceIDs[i], AL_SOURCE_RELATIVE, !Is3D());

		if(!RequiresStreaming())
		{
			u32 oaBuffer = 0;
			if(mAudioClip.IsLoaded())
			{
				OAAudioClip* oaClip = static_cast<OAAudioClip*>(mAudioClip.Get());
				oaBuffer = oaClip->GetOpenALBufferInternal();
			}

			alSourcei(mSourceIDs[i], AL_BUFFER, oaBuffer);
		}
	}

	// Looping is influenced by streaming mode, so re-apply it in case it changed
	SetIsLooping(mLoop);
}

void OAAudioSource::OnClipChanged()
{
	AudioSourceState state = GetState();
	float savedTime = GetTime();

	Stop();

	{
		Lock lock(mMutex);
		ApplyClip();
	}

	SetTime(savedTime);

	if(state != AudioSourceState::Stopped)
		Play();

	if(state == AudioSourceState::Paused)
		Pause();
}

bool OAAudioSource::Is3D() const
{
	if(!mAudioClip.IsLoaded())
		return true;

	return mAudioClip->Is3D();
}

bool OAAudioSource::RequiresStreaming() const
{
	if(!mAudioClip.IsLoaded())
		return false;

	AudioReadMode readMode = mAudioClip->GetReadMode();
	bool isCompressed = readMode == AudioReadMode::LoadCompressed && mAudioClip->GetFormat() != AudioFormat::PCM;

	return (readMode == AudioReadMode::Stream) || isCompressed;
}

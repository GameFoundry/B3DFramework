//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DOAAudioSource.h"
#include "B3DOAAudio.h"
#include "B3DOAAudioClip.h"
#include "AL/al.h"
#include "Math/B3DTransform.h"

using namespace b3d;

OAAudioSource::OAAudioSource()
	: mStreamBuffers(), mBusyBuffers()
{
	GetOAAudio().RegisterSource(this);
	Rebuild();
}

OAAudioSource::~OAAudioSource()
{
	Clear();
	GetOAAudio().UnregisterSource(this);
}

void OAAudioSource::SetClip(const HAudioClip& clip)
{
	mAudioClip = B3DStaticResourceCast<OAAudioClip>(clip);

	Stop();

	Lock lock(mMutex);
	ApplyClip();
}

void OAAudioSource::SetTransform(const Transform& transform)
{
	mPosition = transform.GetPosition();

	auto& contexts = GetOAAudio().GetContexts();
	const u32 contextCount = (u32)contexts.size();
	for(u32 contextIndex = 0; contextIndex < contextCount; contextIndex++)
	{
		if(contexts.size() > 1)
			alcMakeContextCurrent(contexts[contextIndex]);

		if(Is3D())
		{
			Vector3 position = transform.GetPosition();
			alSource3f(mSourceIDs[contextIndex], AL_POSITION, position.X, position.Y, position.Z);
		}
		else
			alSource3f(mSourceIDs[contextIndex], AL_POSITION, 0.0f, 0.0f, 0.0f);
	}
}

void OAAudioSource::SetVelocity(const Vector3& velocity)
{
	mVelocity = velocity;

	auto& contexts = GetOAAudio().GetContexts();
	const u32 contextCount = (u32)contexts.size();
	for(u32 contextIndex = 0; contextIndex < contextCount; contextIndex++)
	{
		if(contexts.size() > 1)
			alcMakeContextCurrent(contexts[contextIndex]);

		if(Is3D())
			alSource3f(mSourceIDs[contextIndex], AL_VELOCITY, velocity.X, velocity.Y, velocity.Z);
		else
			alSource3f(mSourceIDs[contextIndex], AL_VELOCITY, 0.0f, 0.0f, 0.0f);
	}
}

void OAAudioSource::SetVolume(float volume)
{
	mVolume = volume;

	auto& contexts = GetOAAudio().GetContexts();
	const u32 contextCount = (u32)contexts.size();
	for(u32 contextIndex = 0; contextIndex < contextCount; contextIndex++)
	{
		if(contexts.size() > 1)
			alcMakeContextCurrent(contexts[contextIndex]);

		alSourcef(mSourceIDs[contextIndex], AL_GAIN, volume);
	}
}

void OAAudioSource::SetPitch(float pitch)
{
	mPitch = pitch;

	auto& contexts = GetOAAudio().GetContexts();
	const u32 contextCount = (u32)contexts.size();
	for(u32 contextIndex = 0; contextIndex < contextCount; contextIndex++)
	{
		if(contexts.size() > 1)
			alcMakeContextCurrent(contexts[contextIndex]);

		alSourcef(mSourceIDs[contextIndex], AL_PITCH, pitch);
	}
}

void OAAudioSource::SetIsLooping(bool loop)
{
	mLoop = loop;

	// When streaming we handle looping manually
	if(RequiresStreaming())
		loop = false;

	auto& contexts = GetOAAudio().GetContexts();
	const u32 contextCount = (u32)contexts.size();
	for(u32 contextIndex = 0; contextIndex < contextCount; contextIndex++)
	{
		if(contexts.size() > 1)
			alcMakeContextCurrent(contexts[contextIndex]);

		alSourcei(mSourceIDs[contextIndex], AL_LOOPING, loop);
	}
}

void OAAudioSource::SetPriority(i32 priority)
{
	// Do nothing, OpenAL doesn't support priorities (perhaps emulate the behaviour by manually disabling sources?)
}

void OAAudioSource::SetMinDistance(float distance)
{
	mMinDistance = distance;

	auto& contexts = GetOAAudio().GetContexts();
	const u32 contextCount = (u32)contexts.size();
	for(u32 contextIndex = 0; contextIndex < contextCount; contextIndex++)
	{
		if(contexts.size() > 1)
			alcMakeContextCurrent(contexts[contextIndex]);

		alSourcef(mSourceIDs[contextIndex], AL_REFERENCE_DISTANCE, distance);
	}
}

void OAAudioSource::SetAttenuation(float attenuation)
{
	mAttenuation = attenuation;

	auto& contexts = GetOAAudio().GetContexts();
	const u32 contextCount = (u32)contexts.size();
	for(u32 contextIndex = 0; contextIndex < contextCount; contextIndex++)
	{
		if(contexts.size() > 1)
			alcMakeContextCurrent(contexts[contextIndex]);

		alSourcef(mSourceIDs[contextIndex], AL_ROLLOFF_FACTOR, attenuation);
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

	auto& contexts = GetOAAudio().GetContexts();
	const u32 contextCount = (u32)contexts.size();
	for(u32 contextIndex = 0; contextIndex < contextCount; contextIndex++)
	{
		if(contexts.size() > 1)
			alcMakeContextCurrent(contexts[contextIndex]);

		alSourcePlay(mSourceIDs[contextIndex]);

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
	auto& contexts = GetOAAudio().GetContexts();
	const u32 contextCount = (u32)contexts.size();
	for(u32 contextIndex = 0; contextIndex < contextCount; contextIndex++)
	{
		if(contexts.size() > 1)
			alcMakeContextCurrent(contexts[contextIndex]);

		alSourcePause(mSourceIDs[contextIndex]);
	}
}

void OAAudioSource::Stop()
{
	auto& contexts = GetOAAudio().GetContexts();
	const u32 contextCount = (u32)contexts.size();
	for(u32 contextIndex = 0; contextIndex < contextCount; contextIndex++)
	{
		if(contexts.size() > 1)
			alcMakeContextCurrent(contexts[contextIndex]);

		alSourceStop(mSourceIDs[contextIndex]);
		alSourcef(mSourceIDs[contextIndex], AL_SEC_OFFSET, 0.0f);
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
			auto& contexts = GetOAAudio().GetContexts();
			const u32 contextCount = (u32)contexts.size();
			for(u32 contextIndex = 0; contextIndex < contextCount; contextIndex++)
			{
				if(contexts.size() > 1)
					alcMakeContextCurrent(contexts[contextIndex]);

				alSourcePause(mSourceIDs[contextIndex]);
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
				mStreamProcessedPosition = (u32)(time * mAudioClip->GetFrequency() * mAudioClip->GetChannelCount());
			else
				mStreamProcessedPosition = 0;

			mStreamQueuedPosition = mStreamProcessedPosition;
			clipTime = 0.0f;
		}
	}

	auto& contexts = GetOAAudio().GetContexts();
	const u32 contextCount = (u32)contexts.size();
	for(u32 contextIndex = 0; contextIndex < contextCount; contextIndex++)
	{
		if(contexts.size() > 1)
			alcMakeContextCurrent(contexts[contextIndex]);

		alSourcef(mSourceIDs[contextIndex], AL_SEC_OFFSET, clipTime);
	}

	if(state != AudioSourceState::Stopped)
		Play();

	if(state == AudioSourceState::Paused)
		Pause();
}

float OAAudioSource::GetTime() const
{
	Lock lock(mMutex);

	auto& contexts = GetOAAudio().GetContexts();

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
			timeOffset = (float)mStreamProcessedPosition / mAudioClip->GetFrequency() / mAudioClip->GetChannelCount();

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

	auto& contexts = GetOAAudio().GetContexts();
	const u32 contextCount = (u32)contexts.size();

	Lock lock(mMutex);
	for(u32 contextIndex = 0; contextIndex < contextCount; contextIndex++)
	{
		if(contexts.size() > 1)
			alcMakeContextCurrent(contexts[contextIndex]);

		alSourcei(mSourceIDs[contextIndex], AL_BUFFER, 0);
		alDeleteSources(1, &mSourceIDs[contextIndex]);
	}

	mSourceIDs.clear();
}

void OAAudioSource::Rebuild()
{
	auto& contexts = GetOAAudio().GetContexts();
	u32 contextCount = (u32)contexts.size();

	{
		Lock lock(mMutex);

		for(u32 i = 0; i < contextCount; i++)
		{
			if(contexts.size() > 1)
				alcMakeContextCurrent(contexts[i]);

			u32 source = 0;
			alGenSources(1, &source);

			mSourceIDs.push_back(source);
		}
	}

	for(u32 contextIndex = 0; contextIndex < contextCount; contextIndex++)
	{
		if(contexts.size() > 1)
			alcMakeContextCurrent(contexts[contextIndex]);

		alSourcef(mSourceIDs[contextIndex], AL_PITCH, mPitch);
		alSourcef(mSourceIDs[contextIndex], AL_REFERENCE_DISTANCE, mMinDistance);
		alSourcef(mSourceIDs[contextIndex], AL_ROLLOFF_FACTOR, mAttenuation);

		if(RequiresStreaming())
			alSourcei(mSourceIDs[contextIndex], AL_LOOPING, false);
		else
			alSourcei(mSourceIDs[contextIndex], AL_LOOPING, mLoop);

		if(Is3D())
		{
			alSourcei(mSourceIDs[contextIndex], AL_SOURCE_RELATIVE, false);
			alSource3f(mSourceIDs[contextIndex], AL_POSITION, mPosition.X, mPosition.Y, mPosition.Z);
			alSource3f(mSourceIDs[contextIndex], AL_VELOCITY, mVelocity.X, mVelocity.Y, mVelocity.Z);
		}
		else
		{
			alSourcei(mSourceIDs[contextIndex], AL_SOURCE_RELATIVE, true);
			alSource3f(mSourceIDs[contextIndex], AL_POSITION, 0.0f, 0.0f, 0.0f);
			alSource3f(mSourceIDs[contextIndex], AL_VELOCITY, 0.0f, 0.0f, 0.0f);
		}

		{
			Lock lock(mMutex);

			if(!mIsStreaming)
			{
				u32 oaBuffer = 0;
				if(mAudioClip.IsLoaded())
				{
					OAAudioClip* oaClip = static_cast<OAAudioClip*>(mAudioClip.Get());
					oaBuffer = oaClip->GetOpenALBuffer();
				}

				alSourcei(mSourceIDs[contextIndex], AL_BUFFER, oaBuffer);
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
	B3D_ASSERT(!mIsStreaming);

	alGenBuffers(kStreamBufferCount, mStreamBuffers);
	GetOAAudio().StartStreaming(this);

	memset(&mBusyBuffers, 0, sizeof(mBusyBuffers));
	mIsStreaming = true;
}

void OAAudioSource::StopStreaming()
{
	B3D_ASSERT(mIsStreaming);

	mIsStreaming = false;
	GetOAAudio().StopStreaming(this);

	auto& contexts = GetOAAudio().GetContexts();
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
	info.ChannelCount = mAudioClip->GetChannelCount();
	info.SampleRate = mAudioClip->GetFrequency();
	info.SampleCount = 0;

	const u32 totalSampleCount = mAudioClip->GetSampleCount();

	// Note: It is safe to access contexts here only because it is guaranteed by the OAAudio manager that it will always
	// stop all streaming before changing contexts. Otherwise a mutex lock would be needed for every context access.
	auto& contexts = GetOAAudio().GetContexts();
	const u32 contextCount = (u32)contexts.size();
	for(u32 contextIndex = 0; contextIndex < contextCount; contextIndex++)
	{
		if(contexts.size() > 1)
			alcMakeContextCurrent(contexts[contextIndex]);

		i32 processedBufferCount = 0;
		alGetSourcei(mSourceIDs[contextIndex], AL_BUFFERS_PROCESSED, &processedBufferCount);

		for(i32 j = processedBufferCount; j > 0; j--)
		{
			u32 buffer;
			alSourceUnqueueBuffers(mSourceIDs[contextIndex], 1, &buffer);

			i32 bufferIndex = -1;
			for(u32 k = 0; k < kStreamBufferCount; k++)
			{
				if(buffer == mStreamBuffers[k])
				{
					bufferIndex = (i32)k;
					break;
				}
			}

			// Possibly some buffer from previous playback remained unqueued, in which case ignore it
			if(bufferIndex == -1)
				continue;

			mBusyBuffers[bufferIndex] &= ~(1 << bufferIndex);

			// Check if all sources are done with this buffer
			if(mBusyBuffers[bufferIndex] != 0)
				break;

			i32 bufferSize;
			i32 bufferBits;

			alGetBufferi(buffer, AL_SIZE, &bufferSize);
			alGetBufferi(buffer, AL_BITS, &bufferBits);

			if(bufferBits == 0)
			{
				B3D_LOG(Error, LogAudio, "Error decoding stream.");
				return;
			}
			else
			{
				u32 bytesPerSample = bufferBits / 8;
				mStreamProcessedPosition += bufferSize / bytesPerSample;
			}

			if(mStreamProcessedPosition == totalSampleCount) // Reached the end
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

		if(FillBuffer(mStreamBuffers[i], info, totalSampleCount))
		{
			for(auto& source : mSourceIDs)
				alSourceQueueBuffers(source, 1, &mStreamBuffers[i]);

			mBusyBuffers[i] |= 1 << i;
		}
		else
			break;
	}
}

bool OAAudioSource::FillBuffer(u32 buffer, AudioDataInfo& info, u32 maxSampleCount)
{
	u32 remainingSampleCount = maxSampleCount - mStreamQueuedPosition;
	if(remainingSampleCount == 0) // Reached the end
	{
		if(mLoop)
		{
			mStreamQueuedPosition = 0;
			remainingSampleCount = maxSampleCount;
		}
		else // If not looping, don't queue any more buffers, we're done
			return false;
	}

	// Read audio data
	const u32 sampleCount = std::min(remainingSampleCount, info.SampleRate * info.ChannelCount); // 1 second of data
	const u32 sampleBufferSize = sampleCount * (info.BitDepth / 8);

	u8* const samples = (u8*)B3DStackAllocate(sampleBufferSize);

	mAudioClip->GetSamples(samples, mStreamQueuedPosition, sampleCount);
	mStreamQueuedPosition += sampleCount;

	info.SampleCount = sampleCount;
	GetOAAudio().WriteToOpenALBuffer(buffer, samples, info);

	B3DStackFree(samples);

	return true;
}

void OAAudioSource::ApplyClip()
{
	auto& contexts = GetOAAudio().GetContexts();
	const u32 contextCount = (u32)contexts.size();
	for(u32 contextIndex = 0; contextIndex < contextCount; contextIndex++)
	{
		if(contexts.size() > 1)
			alcMakeContextCurrent(contexts[contextIndex]);

		alSourcei(mSourceIDs[contextIndex], AL_SOURCE_RELATIVE, !Is3D());

		if(!RequiresStreaming())
		{
			u32 oaBuffer = 0;
			if(mAudioClip.IsLoaded())
			{
				OAAudioClip* oaClip = static_cast<OAAudioClip*>(mAudioClip.Get());
				oaBuffer = oaClip->GetOpenALBuffer();
			}

			alSourcei(mSourceIDs[contextIndex], AL_BUFFER, oaBuffer);
		}
	}

	// Looping is influenced by streaming mode, so re-apply it in case it changed
	SetIsLooping(mLoop);
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

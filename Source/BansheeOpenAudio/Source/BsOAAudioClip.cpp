//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsOAAudioClip.h"
#include "BsOAOggVorbisWriter.h"
#include "BsOAOggVorbisReader.h"
#include "BsDataStream.h"
#include "BsAudioUtility.h"
#include "BsOAAudio.h"
#include "AL/al.h"

namespace BansheeEngine
{
	/** 
	 * Returns optimal format for the provided number of channels and bit depth. It is assumed the user has checked if
	 * extensions providing these formats are actually available.
	 */
	ALenum getSoundBufferFormat(UINT32 numChannels, UINT32 bitDepth)
	{
		switch (bitDepth)
		{
		case 8:
		{
			switch (numChannels)
			{
			case 1:  return AL_FORMAT_MONO8;
			case 2:  return AL_FORMAT_STEREO8;
			case 4:  return alGetEnumValue("AL_FORMAT_QUAD8");
			case 6:  return alGetEnumValue("AL_FORMAT_51CHN8");
			case 7:  return alGetEnumValue("AL_FORMAT_61CHN8");
			case 8:  return alGetEnumValue("AL_FORMAT_71CHN8");
			default:
				assert(false);
				return 0;
			}
		}
		case 16:
		{
			switch (numChannels)
			{
			case 1:  return AL_FORMAT_MONO16;
			case 2:  return AL_FORMAT_STEREO16;
			case 4:  return alGetEnumValue("AL_FORMAT_QUAD16");
			case 6:  return alGetEnumValue("AL_FORMAT_51CHN16");
			case 7:  return alGetEnumValue("AL_FORMAT_61CHN16");
			case 8:  return alGetEnumValue("AL_FORMAT_71CHN16");
			default:
				assert(false);
				return 0;
			}
		}
		case 32:
		{
			switch (numChannels)
			{
			case 1:  return alGetEnumValue("AL_FORMAT_MONO_FLOAT32");
			case 2:  return alGetEnumValue("AL_FORMAT_STEREO_FLOAT32");
			case 4:  return alGetEnumValue("AL_FORMAT_QUAD32");
			case 6:  return alGetEnumValue("AL_FORMAT_51CHN32");
			case 7:  return alGetEnumValue("AL_FORMAT_61CHN32");
			case 8:  return alGetEnumValue("AL_FORMAT_71CHN32");
			default:
				assert(false);
				return 0;
			}
		}
		default:
			assert(false);
			return 0;
		}
	}

	/** 
	 * Writes provided samples into the OpenAL buffer with the provided ID. If the supported format is not supported the
	 * samples will first be converted into a valid format.
	 */
	void writeToSoundBuffer(UINT32 bufferId, UINT8* samples, const AudioFileInfo& info)
	{
		if (info.numChannels <= 2) // Mono or stereo
		{
			if (info.bitDepth > 16)
			{
				if (gOAAudio().isExtensionSupported("AL_EXT_float32"))
				{
					UINT32 bufferSize = info.numSamples * sizeof(float);
					float* sampleBufferFloat = (float*)bs_stack_alloc(bufferSize);

					AudioUtility::convertToFloat(samples, info.bitDepth, sampleBufferFloat, info.numSamples);

					ALenum format = getSoundBufferFormat(info.numChannels, info.bitDepth);
					alBufferData(bufferId, format, sampleBufferFloat, bufferSize, info.sampleRate);

					bs_stack_free(sampleBufferFloat);
				}
				else
				{
					LOGWRN("OpenAL doesn't support bit depth larger than 16. Your audio data will be truncated.");

					UINT32 bufferSize = info.numSamples * 2;
					UINT8* sampleBuffer16 = (UINT8*)bs_stack_alloc(bufferSize);

					AudioUtility::convertBitDepth(samples, info.bitDepth, sampleBuffer16, 16, info.numSamples);

					ALenum format = getSoundBufferFormat(info.numChannels, 16);
					alBufferData(bufferId, format, sampleBuffer16, bufferSize, info.sampleRate);

					bs_stack_free(sampleBuffer16);
				}
			}
			else
			{
				ALenum format = getSoundBufferFormat(info.numChannels, 16);
				alBufferData(bufferId, format, samples, info.numSamples * (info.bitDepth / 8), info.sampleRate);
			}
		}
		else // Multichannel
		{
			// Note: Assuming AL_EXT_MCFORMATS is supported. If it's not, channels should be reduced to mono or stereo.

			if (info.bitDepth == 24) // 24-bit not supported, convert to 32-bit
			{
				UINT32 bufferSize = info.numSamples * sizeof(INT32);
				UINT8* sampleBuffer32 = (UINT8*)bs_stack_alloc(bufferSize);

				AudioUtility::convertBitDepth(samples, info.bitDepth, sampleBuffer32, 32, info.numSamples);

				ALenum format = getSoundBufferFormat(info.numChannels, 32);
				alBufferData(bufferId, format, sampleBuffer32, bufferSize, info.sampleRate);

				bs_stack_free(sampleBuffer32);
			}
			else
			{
				ALenum format = getSoundBufferFormat(info.numChannels, info.bitDepth);
				alBufferData(bufferId, format, samples, info.numSamples * (info.bitDepth / 8), info.sampleRate);
			}
		}
	}

	OAAudioClip::OAAudioClip(const SPtr<DataStream>& samples, UINT32 streamSize, UINT32 numSamples, const AUDIO_CLIP_DESC& desc)
		:AudioClip(samples, streamSize, numSamples, desc), mNeedsDecompression(false), mBufferId((UINT32)-1), mSourceStreamSize(0)
	{ }

	OAAudioClip::~OAAudioClip()
	{
		if (mBufferId != (UINT32)-1)
			alDeleteBuffers(1, &mBufferId);
	}

	void OAAudioClip::initialize()
	{
		{
			Lock lock(mMutex); // Needs to be called even if stream data is null, to ensure memory fence is added so the
							   // other thread sees properly initialized AudioClip members

			AudioFileInfo info;
			info.bitDepth = mDesc.bitDepth;
			info.numChannels = mDesc.numChannels;
			info.numSamples = mNumSamples;
			info.sampleRate = mDesc.frequency;

			// If we need to keep source data, read everything into memory and keep a copy
			if (mKeepSourceData)
			{
				mStreamData->seek(mStreamOffset);

				UINT8* sampleBuffer = (UINT8*)bs_alloc(mStreamSize);
				mStreamData->read(sampleBuffer, mStreamSize);

				mSourceStreamData = bs_shared_ptr_new<MemoryDataStream>(sampleBuffer, mStreamSize);
				mSourceStreamSize = mStreamSize;
			}

			if(mDesc.readMode == AudioReadMode::LoadDecompressed && mDesc.format == AudioFormat::VORBIS)
			{
				// Read all uncompressed data into memory
				SPtr<DataStream> stream;
				UINT32 offset = 0;
				if (mSourceStreamData != nullptr) // If it's already loaded in memory, use it directly
					stream = mSourceStreamData;
				else
				{
					stream = mStreamData;
					offset = mStreamOffset;
				}

				stream->seek(offset);

				// Decompress from Ogg
				OAOggVorbisReader reader;
				if (reader.open(stream, info))
				{
					bs_frame_mark();

					UINT32 bufferSize = info.numSamples * info.bitDepth;
					UINT8* sampleBuffer = (UINT8*)bs_frame_alloc(bufferSize);

					reader.read(sampleBuffer, info.numSamples);

					alGenBuffers(1, &mBufferId);



					if(info.bitDepth > 16)
					{
						LOGWRN("OpenAL doesn't support bit depth larger than 16. Your audio data will be truncated. "
							"Consider re-importing the audio clip as 16-bit. Audio clip: " + toString(getName()) + ".");

						bufferSize = info.numSamples * 16;
						UINT8* sampleBuffer16 = (UINT8*)bs_frame_alloc(bufferSize);

						AudioUtility::convertBitDepth(sampleBuffer, info.bitDepth, sampleBuffer16, 16, info.numSamples);

						info.bitDepth = 16;

						bs_frame_free(sampleBuffer);
						sampleBuffer = sampleBuffer16;
					}

					writeToSoundBuffer(mBufferId, sampleBuffer, info);

					mStreamData = nullptr;
					mStreamOffset = 0;
					mStreamSize = 0;

					bs_frame_free(sampleBuffer);
					bs_frame_clear();
				}
				else
					LOGERR("Failed decompressing AudioClip stream.");
			}
			else if(mDesc.readMode == AudioReadMode::LoadCompressed)
			{
				// If reading from file, make a copy of data in memory, otherwise just take ownership of the existing buffer
				if (mStreamData->isFile()) 
				{
					if (mSourceStreamData != nullptr) // If it's already loaded in memory, use it directly
						mStreamData = mSourceStreamData;
					else
					{
						UINT8* data = (UINT8*)bs_alloc(mStreamSize);

						mStreamData->seek(mStreamOffset);
						mStreamData->read(data, mStreamSize);

						mStreamData = bs_shared_ptr_new<MemoryDataStream>(data, mStreamSize);
					}

					mStreamOffset = 0;
				}
			}

			if (mDesc.format == AudioFormat::VORBIS && mDesc.readMode != AudioReadMode::LoadDecompressed)
			{
				mNeedsDecompression = true;

				if (mStreamData != nullptr)
				{
					if (!mVorbisReader.open(mStreamData, info))
						LOGERR("Failed decompressing AudioClip stream.");
				}
			}

			if (mStreamData != nullptr)
				mStreamData->seek(mStreamOffset);
		}

		AudioClip::initialize();
	}

	void OAAudioClip::getSamples(UINT8* samples, UINT32 count) const
	{
		Lock lock(mMutex);

		// Try to read from normal stream, and if that fails read from in-memory stream if it exists
		if (mStreamData == nullptr)
		{
			if (mNeedsDecompression)
				mVorbisReader.read(samples, count);
			else
			{
				UINT32 bytesPerSample = mDesc.bitDepth / 8;
				UINT32 size = count * bytesPerSample;

				mStreamData->read(samples, size);
			}

			return;
		}

		if (mSourceStreamData != nullptr)
		{
			assert(!mNeedsDecompression); // Normal stream must exist if decompressing

			UINT32 bytesPerSample = mDesc.bitDepth / 8;
			UINT32 size = count * bytesPerSample;

			mSourceStreamData->read(samples, size);
			return;
		}

		LOGWRN("Attempting to read samples while sample data is not available.");
	}

	void OAAudioClip::seekSamples(UINT32 offset)
	{
		Lock lock(mMutex);

		// Try to seek normal stream, and if that fails seek in-memory stream if it exists
		if (mStreamData != nullptr)
		{
			if (mNeedsDecompression)
				mVorbisReader.seek(offset);
			else
			{
				UINT32 bytesPerSample = mDesc.bitDepth / 8;
				UINT32 streamOffset = mStreamOffset + offset * bytesPerSample;

				mStreamData->seek(streamOffset);
			}

			return;
		}

		if(mSourceStreamData != nullptr)
		{
			assert(!mNeedsDecompression); // Normal stream must exist if decompressing

			UINT32 bytesPerSample = mDesc.bitDepth / 8;
			UINT32 streamOffset = offset * bytesPerSample;

			mSourceStreamData->seek(streamOffset);
			return;
		}

		LOGWRN("Seeking samples while sample data is not available.");
	}

	SPtr<DataStream> OAAudioClip::getSourceStream(UINT32& size)
	{
		Lock lock(mMutex);

		size = mSourceStreamSize;
		mSourceStreamData->seek(0);

		return mSourceStreamData;
	}
}